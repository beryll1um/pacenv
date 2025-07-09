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
int pacenv_makedir(const char* path, mode_t mode);

// vim: set ts=4 sw=4 noexpandtab:
