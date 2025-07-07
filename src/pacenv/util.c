// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pacenv/glob.h>

static int mkdir_recurs(const char* path, char* stage, mode_t mode)
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
		return mkdir_recurs(path, slash + 1, mode);
	}
	return 0;
}

int mkdir_p(const char* path, mode_t mode)
{
	char buf[PATH_MAX];
	size_t len = strnlen(path, sizeof(buf) - 1);
	memcpy(buf, path, len);
	buf[len] = '\0';
	return mkdir_recurs((const char*)buf, buf, mode);
}

// vim: set ts=4 sw=4 noexpandtab:
