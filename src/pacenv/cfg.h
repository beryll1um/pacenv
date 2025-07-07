// Copyright (c) 2025 The Pacenv developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or https://opensource.org/license/mit/.

// Forward declaration of the JSON object from the json-c library.
struct json_object;

// I am not happy with the default json-c library from file parsing
// as it isn't verbose enough.
struct json_object* parse_cfg(const char* path);

// vim: set ts=4 sw=4 noexpandtab:
