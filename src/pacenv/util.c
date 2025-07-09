// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pacenv/glob.h>
#include <json-c/json.h>

// You may disapprove this little shortcut. It makes me feel better.
#define FAILED_TO_ACTION_FILE_STR(ACTION) "failed to " ACTION " file"

struct json_object* pacenv_jso_parse(const char* filepath)
{
	int fd = open(filepath, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "%s: " FAILED_TO_ACTION_FILE_STR("open") ": %s: %s\n",
			g_filename, filepath, strerror(errno));
		return NULL;
	}
	struct json_object* jso = json_object_from_fd(fd);
	if (close(fd) == -1)
	{
		fprintf(stderr, "%s: " FAILED_TO_ACTION_FILE_STR("close") ": %s: %s\n",
			g_filename, filepath, strerror(errno));
		// This is a problem, but there is no need to interrupt execution.
	}
	if (!jso)
	{
		// Thanks JSON-C developers for the newline they are adding
		// to the end of each error.
		fprintf(stderr, "%s: " FAILED_TO_ACTION_FILE_STR("parse") ": %s: %s",
			g_filename, filepath, json_util_get_last_err());
		return NULL;
	}
	if (json_object_get_type(jso) != json_type_object)
	{
		fprintf(stderr, "%s: type mismatch of file content: %s: "
			"must be object\n", g_filename, filepath);
		json_object_put(jso);
		return NULL;
	}
	return jso;
}

static int makedir(const char* path, char* stage, mode_t mode)
{
	char* slash = strchr(stage, '/');
	if (slash)
	{
		*slash = '\0';
	}
	if (mkdir(path, mode) == -1 && errno != EEXIST)
	{
		fprintf(stderr, "%s: failed to make directory: %s: %s\n", g_filename,
			path, strerror(errno));
		return -1;
	}
	if (slash)
	{
		*slash = '/';
		return makedir(path, slash + 1, mode);
	}
	return 0;
}

int pacenv_makedir(const char* path, mode_t mode)
{
	char buf[PATH_MAX];
	size_t len = strnlen(path, sizeof(buf) - 1);
	memcpy(buf, path, len);
	buf[len] = '\0';
	return makedir((const char*)buf, buf, mode);
}

// vim: set ts=4 sw=4 noexpandtab:
