// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pacenv/glob.h>
#include <json-c/json.h>

// You may disabandon this little shortcut. It makes me feel better.
#define FAILED_TO_ACTION_CONFIG_FILE(ACTION) "failed to " ACTION " config file"

struct json_object* parse_cfg(const char* path)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "%s: " FAILED_TO_ACTION_CONFIG_FILE("open")
			": %s: %s\n", g_filename, path, strerror(errno));
		return NULL;
	}
	struct json_object* jso = json_object_from_fd(fd);
	if (close(fd) == -1)
	{
		fprintf(stderr, "%s: " FAILED_TO_ACTION_CONFIG_FILE("close")
			": %s: %s\n", g_filename, path, strerror(errno));
		// This is a problem, but there is no need to interrupt execution.
	}
	if (!jso)
	{
		// Thanks JSON-C developers for the newline they are adding
		// to the end of each error.
		fprintf(stderr, "%s: " FAILED_TO_ACTION_CONFIG_FILE("parse")
			": %s: %s", g_filename, path, json_util_get_last_err());
		return NULL;
	}
	if (json_object_get_type(jso) != json_type_object)
	{
		fprintf(stderr, "%s: type mismatch of config: %s: must be object\n",
			g_filename, path);
		json_object_put(jso);
		return NULL;
	}
	return jso;
}

// vim: set ts=4 sw=4 noexpandtab:
