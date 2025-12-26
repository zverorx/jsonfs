/* 
 * This file is part of jsonfs.
 * jsonfs - File system for working with JSON.
 *
 * Copyright (C) 2025 Egorov Konstantin
 *
 * jsonfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * jsonfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jsonfs. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Entry point of the JSONFS.
 *
 * Processes command line parameters and calls fuse_main().
 */

#define FUSE_USE_VERSION 	35

#include <jansson.h>
#include <fuse.h>
#include <stdlib.h>

#include "common.h"
#include "jsonfs.h"
#include "json_operations.h"

int main(int argc, char **argv)
{
	json_t *root = NULL;
	json_t *norm_root = NULL;
	json_error_t json_error;
	const char *json_file = NULL;
	int ret;

	if (argc < 3) { return EXIT_FAILURE; }

	json_file = argv[1];

	root = json_load_file(json_file, JSON_DECODE_ANY, &json_error);
	if (!root) { goto handle_error; }

	norm_root = normalize_json(root, 1);
	if (!norm_root) { goto handle_error; }
	json_decref(root);

	struct jsonfs_private_data *pd = init_private_data(norm_root, json_file);
	if (!pd) { goto handle_error; }

	struct fuse_operations op = get_fuse_op();
	struct private_args args = get_fuse_args(argc, argv);

	ret = fuse_main(args.fuse_argc, args.fuse_argv, &op, pd);
	free(args.fuse_argv);
	return ret;

	handle_error:
		json_decref(root);
		json_decref(norm_root); 
		fputs("jsonfs: failed to initialize filesystem\n", stderr);
		return EXIT_FAILURE;
}
