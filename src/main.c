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

/* Includes */
#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "jsonfs.h"

int main(int argc, char **argv)
{
	json_t *root = NULL;
	json_t *correct_root = NULL;
	json_error_t json_error;
	const char *json_file = NULL;
	int ret;

	if (argc < 3) { 
		return EXIT_FAILURE; 
	}

	json_file = argv[1];

	root = json_load_file(json_file, JSON_DECODE_ANY, &json_error);
	CHECK_POINTER(root, EXIT_FAILURE);

	correct_root = convert_to_obj(root, 1);
	CHECK_POINTER(correct_root, EXIT_FAILURE);
	json_decref(root);

	struct json_private_data *pd = init_private_data(correct_root, json_file);
	if (!pd) {
		json_decref(correct_root);
		return EXIT_FAILURE;
	}

	struct fuse_operations op = get_fuse_op();
	struct private_args args = get_fuse_args(argc, argv);

	ret = fuse_main(args.fuse_argc, args.fuse_argv, &op, pd);
	free(args.fuse_argv);
	return ret;
}
