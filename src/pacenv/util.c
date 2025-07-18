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
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pacenv/glob.h>
#include <json-c/json.h>

// You may disapprove this little shortcut. It makes me feel better.
#define FAILED_TO_ACTION_FILE_STR(ACTION) "failed to " ACTION " file"

// I don't like these solutions, but it really simplifies things and I don't
// see a case where I would need more arguments.
#define MAKEDIRL_MAXARGS 7

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
	// The reason the first condition exists is that the first N characters
	// of the path may be '/' characters, so the argument to 'mkdir'
	// will be the empty string.
	if (*path && mkdir(path, mode) == -1 && errno != EEXIST)
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

int pacenv_makedir(mode_t mode, const char* path)
{
	char buf[PATH_MAX];
	// I don't want to use BSD strlcpy, and using strcpy would be unsafe since
	// the valid path argument might be longer than PATH_MAX.
	// Maybe it make sense to write our own function for such cases.
	char* cursor = mempcpy(buf, path, strnlen(path, sizeof(buf) - 1));
	*cursor = '\0';
	return makedir(buf, buf, mode);
}

int pacenv_makedirl(mode_t mode, const char* args, ...)
{
	const char* arr[MAKEDIRL_MAXARGS + 1];
	// We need one more byte for null terminating character.
	size_t pathsz = 1;

	va_list ap;
	size_t argno = 0;
	va_start(ap, args);
	while (args != NULL && argno < MAKEDIRL_MAXARGS)
	{
		// We don't need do another loop to calculate total length
		// as we can do it here, right?
		arr[argno++] = args;
		pathsz += strlen(args);
		args = va_arg(ap, const char*);
	}
	arr[argno] = NULL;
	va_end(ap);

	char* path = malloc(pathsz);
	if (!path)
	{
		return -1;
	}

	// The last step is to concatenate all arguments together.
	char* writer = path;
	for (const char** arg = arr; *arg; arg++)
	{
		writer = mempcpy(writer, *arg, strlen(*arg));
	}
	*writer = '\0';

	int ret = makedir(path, path, mode);
	free(path);
	return ret;
}

size_t pacenv_memocc(const char* start, const char* end, const char* occur,
	size_t occurlen)
{
	size_t cnt = 0;
	while ((start = memmem(start, end - start, occur, occurlen)))
	{
		cnt++;
		// Offset start of the memory region on the size of found occurrence.
		start += occurlen;
	}
	return cnt;
}

// This may not be the most efficient option, but it works reliably
// with any number of substitutions without dynamic reallocations.
char* pacenv_memrep(char* start, char* end, const char* old, size_t oldlen,
	const char* new, size_t newlen)
{
	while ((start = memmem(start, end - start, old, oldlen)))
	{
		// If new isn't the same size we need to shift buffer.
		if (oldlen != newlen)
		{
			memmove(start + newlen, start + oldlen, end - start - oldlen);
			end += newlen - oldlen;
		}
		// Insert new start into the startfer and move start pointer to
		// the end of it.
		start = mempcpy(start, new, newlen);
	}
	return end;
}

// vim: set ts=4 sw=4 noexpandtab:
