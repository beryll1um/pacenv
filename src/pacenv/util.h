// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

#include <sys/types.h>

// Forward declaration of the JSON object from the json-c library.
struct json_object;

// I am not happy with the default json-c library from file parsing
// as it isn't verbose enough.
struct json_object* pacenv_jso_parse(const char* filepath);

// I would like to unify this and avoid spelling problems in the future.
#define PACENV_TYPE_MISMATCH_STR(WHAT, WHERE, EXPECT) \
	"type mismatch for " WHAT " in " WHERE ": must be " EXPECT
#define PACENV_MISSING_PROP_STR(WHAT, WHERE) \
	"missing property " WHAT " in " WHERE

// This is similar to 'mkdir -p', but looks much better!
int pacenv_makedir(mode_t mode, const char* path);

// This version of the function accept arbitrary number of arguments:
// pacenv_makedirl(mode, arg1, arg2, ..., NULL);
int pacenv_makedirl(mode_t mode, const char* args, ...);

// Helps to calculate number of occurrences of the needle in a haystack.
size_t pacenv_memocc(const char* start, const char* end, const char* occur,
	size_t occurlen);

// It implements replacement in a buffer that is already long enough
// to hold original string and replacement result.
// Returns pointer to the new end of the buffer.
char* pacenv_memrep(char* start, char* end, const char* old, size_t oldlen,
	const char* new, size_t newlen);

// Should I explain why we need this?
#define MAX(A, B) (A > B ? A : B)

// vim: set ts=4 sw=4 noexpandtab:
