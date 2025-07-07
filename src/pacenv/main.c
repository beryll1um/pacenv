// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <pacenv/cfg.h>
#include <pacenv/glob.h>
#include <pacenv/util.h>
#include <json-c/json.h>
#include <alpm.h>

static void release_alpm_cb(void* data)
{
	alpm_handle_t* handle = (alpm_handle_t*)data;

	// I suppose that only successfully initialized handles will be released.
	// It means that root is always contains meaningfull pointer.
	char* root = strdup(alpm_option_get_root(handle));
	if (!root)
	{
		return;
	}

	if (alpm_release(handle) == -1)
	{
		fprintf(stderr, "%s: failed to release the ALPM library: %s\n",
			g_filename, root);
	}
	else if (g_verbose > 0)
	{
		printf("%s: successful release of ALPM library -- '%s'\n", g_filename,
			root);
	}

	free(root);
}

static void log_alpm_cb(void* ctx, alpm_loglevel_t level, const char* fmt,
	va_list args)
{
	// There is currently no multithreading, but if it is implemented,
	// this approach should be abandoned since it does not protect
	// against stdout buffer flush races.
	printf("%s: libalpm: ", g_filename);
	vprintf(fmt, args);
}

static struct option long_opts[] =
{
	{ "help", no_argument, NULL, 'h' },
	{ "config", required_argument, NULL, 'c' },
	{ NULL, 0, NULL, 0 }
};

static char help_txt[] = {
// Thanks to the C23 standard we can use this beautiful macro.
#embed "assets/help.txt" suffix(, 0)
};

// This must be a constant since the database is unique for each environment.
#define PACENV_DBPATH "/var/lib/pacenv"

static alpm_handle_t* initialize_alpm(const char* root, alpm_errno_t* err)
{
	alpm_handle_t* handle = NULL;

	// Note that the sizeof(PACENV_DBPATH) includes needed
	// null terminating character.
	char* dbpath = malloc(strlen(root) + sizeof(PACENV_DBPATH));
	if (!dbpath)
	{
		return handle;
	}
	strcpy(dbpath, root);
	strcat(dbpath, PACENV_DBPATH);

	if (mkdir_p(dbpath, 0755) == -1)
	{
		goto out;
	}

	handle = alpm_initialize(root, dbpath, err);
out:
	free(dbpath);
	return handle;
}

int main(int argc, char** argv, char** envp)
{
	int ret;
	g_filename = argv[0];
	struct json_object* cfg = NULL;

	while ((ret = getopt_long(argc, argv, "hvc:", long_opts, NULL)) != -1)
	{
		switch (ret)
		{
			case '?':
				fprintf(stderr, "Try '%s --help' for more information.\n",
					g_filename);
				return EXIT_FAILURE;
			case 'h':
				printf("Usage: %s [OPTION]... DIRECTORY...\n%s", g_filename,
					help_txt);
				return EXIT_SUCCESS;
			case 'c':
				if (!(cfg = parse_cfg(optarg)))
				{
					return EXIT_FAILURE;
				}
				break;
			case 'v': g_verbose++;
		}
	}

	// I expect this file to be like 'package.json' or something like that.
	if (!cfg && !(cfg = parse_cfg("pacenv.json")))
	{
		return EXIT_FAILURE;
	}

	ret = EXIT_FAILURE;
	alpm_list_t* handles = NULL;
	for (int ind = optind; ind < argc; ind++)
	{
		alpm_errno_t err;
		alpm_handle_t* handle = initialize_alpm(argv[ind], &err);
		if (!handle)
		{
			fprintf(stderr, "%s: failed to initialize the ALPM library: %s\n",
				g_filename, alpm_strerror(err));
			goto out;
		}

		if (g_verbose > 1)
		{
			alpm_option_set_logcb(handle, log_alpm_cb, NULL);
		}

		handles = alpm_list_add(handles, handle);
		if (!handles)
		{
			goto out;
		}

		if (g_verbose > 0)
		{
			printf("%s: successful initialization of ALPM library -- '%s'\n",
				g_filename, argv[ind]);
		}
	}

	struct json_object* syncdbs = json_object_object_get(cfg, "syncdbs");
	if (!syncdbs)
	{
		fprintf(stderr, "%s: missing required property 'syncdbs' "
			"in config\n", g_filename);
		goto out;
	}
	if (json_object_get_type(syncdbs) != json_type_object)
	{
		fprintf(stderr, "%s: type mismatch for 'syncdbs' in config: "
			"must be object\n", g_filename);
		goto out;
	}

	struct json_object* deps = json_object_object_get(cfg, "deps");
	if (!deps)
	{
		fprintf(stderr, "%s: missing required property 'deps' "
			"in config\n", g_filename);
		goto out;
	}
	if (json_object_get_type(deps) != json_type_array)
	{
		fprintf(stderr, "%s: type mismatch for 'deps' in config: "
			"must be array\n", g_filename);
		goto out;
	}

	for (alpm_list_t* ent = handles; ent; ent = alpm_list_next(ent))
	{
		alpm_handle_t* handle = (alpm_handle_t*)ent->data;
		json_object_object_foreach(syncdbs, treename, syncdb)
		{
			if (json_object_get_type(syncdb) != json_type_array)
			{
				fprintf(stderr, "%s: type mismatch for '%s' in 'syncdbs': "
					"must be array\n", g_filename, treename);
				goto out;
			}

			// We do not currently verify signatures as this requires managing
			// a GNUPG keyring, which has to be a separate task.
			alpm_db_t* db = alpm_register_syncdb(handle, treename,
				ALPM_SIG_PACKAGE_OPTIONAL | ALPM_SIG_DATABASE_OPTIONAL);
			if (!db)
			{
				fprintf(stderr, "%s: failed to register synchronization "
					"database: %s: %s\n", g_filename, treename,
					alpm_strerror(alpm_errno(handle)));
				goto out;
			}

			for (size_t i = 0; i < json_object_array_length(syncdb); i++)
			{
				struct json_object* server;
				server = json_object_array_get_idx(syncdb, i);
				if (json_object_get_type(server) != json_type_string)
				{
					fprintf(stderr, "%s: type mismatch for '%s' in 'syncdbs': "
						"must be string array\n", g_filename, treename);
					goto out;
				}

				const char* url = json_object_get_string(server);
				if (alpm_db_add_server(db, url) == -1)
				{
					fprintf(stderr, "%s: failed to add server to database: "
						"%s\n", g_filename, alpm_strerror(alpm_errno(handle)));
					goto out;
				}
			}
		}
		if (alpm_db_update(handle, alpm_get_syncdbs(handle), 0) == -1)
		{
			fprintf(stderr, "%s: failed to update package databases: "
				"%s\n", g_filename, alpm_strerror(alpm_errno(handle)));
			goto out;
		}

		if (alpm_trans_init(handle, ALPM_TRANS_FLAG_NODEPS) == -1)
		{
			fprintf(stderr, "%s: failed to initialize transaction: %s\n",
				g_filename, alpm_strerror(alpm_errno(handle)));
			goto out;
		}

		// This one should be defined here because of
		// 'trans_out' label restriction.
		alpm_list_t* data = NULL;

		for (size_t i = 0; i < json_object_array_length(deps); i++)
		{
			struct json_object* dep = json_object_array_get_idx(deps, i);
			if (json_object_get_type(dep) != json_type_string)
			{
				fprintf(stderr, "%s: type mismatch for 'deps' in config: "
					"must be string array\n", g_filename);
				goto trans_out;
			}
			const char* depstr = json_object_get_string(dep);

			alpm_pkg_t* pkg = alpm_find_dbs_satisfier(handle,
				alpm_get_syncdbs(handle), depstr);
			if (!pkg)
			{
				fprintf(stderr, "%s: failed to resolve dependency "
					"satisfaction: %s: %s\n", g_filename, depstr,
					alpm_strerror(alpm_errno(handle)));
				goto trans_out;
			}

			if (g_verbose > 0)
			{
				printf("%s: successful resolve of dependency satisfaction -- "
					"'%s'\n", g_filename, depstr);
			}

			if (alpm_add_pkg(handle, pkg) == -1)
			{
				fprintf(stderr, "%s: failed to add package: %s: %s\n",
					g_filename, depstr, alpm_strerror(alpm_errno(handle)));
				goto trans_out;
			}
		}

		if (alpm_trans_prepare(handle, &data) == -1)
		{
			fprintf(stderr, "%s: failed to prepare transaction: %s\n",
				g_filename, alpm_strerror(alpm_errno(handle)));
			goto trans_out;
		}

		// In the future, it will be crucial to integrate a progress bar.
		if (alpm_trans_commit(handle, &data) == -1)
		{
			fprintf(stderr, "%s: failed to commit transaction: %s\n",
				g_filename, alpm_strerror(alpm_errno(handle)));
			goto trans_out;
		}

// In case we fail to achieve transaction commit, we will still continue
// processing other handles.
trans_out:
		if (alpm_trans_release(handle) == -1)
		{
			fprintf(stderr, "%s: failed to release transaction: %s\n",
				g_filename, alpm_strerror(alpm_errno(handle)));
		}
		alpm_list_free(data);
	}
	ret = EXIT_SUCCESS;
out:
	alpm_list_free_inner(handles, release_alpm_cb);
	alpm_list_free(handles);
	json_object_put(cfg);
	return ret;
}

// vim: set ts=4 sw=4 noexpandtab:
