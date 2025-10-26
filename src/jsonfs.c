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
 * @brief Contains common functions for JSONFS.
 *
 * Function declarations and specifications can be found in jsonfs.h.
 */

#define FUSE_USE_VERSION 35

#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

extern int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi);
extern int jsonfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
				   off_t offset, struct fuse_file_info *fi,
				   enum fuse_readdir_flags flags);
extern int jsonfs_read(const char *path, char *buffer, size_t size,
				off_t offset, struct fuse_file_info *fi);
extern void jsonfs_destroy(void *userdata);



struct json_private_data *init_private_data(json_t *json_root, const char *path)
{
	CHECK_POINTER(json_root, NULL);
	CHECK_POINTER(path, NULL);

	struct json_private_data *pd = calloc(1, sizeof(struct json_private_data));
	CHECK_POINTER(pd, NULL);

	pd->root = json_root;
	pd->path_to_json_file = strdup(path);
	if (!pd->path_to_json_file) {
		free(pd);
		return NULL;
	}

	return pd;
}

json_t *find_node_by_path(const char *path, json_t *root)
{
	char *dup = NULL;
	char *saveptr = NULL;

	CHECK_POINTER(path, NULL);
	CHECK_POINTER(root, NULL);

	if (strcmp(path, "/") == 0) {
		return root;
	}

	dup = strdup(path);
	CHECK_POINTER(dup, NULL);

	char *key = strtok_r(dup, "/", &saveptr);
	if (!key) goto exit_fail;

	json_t *curr_obj = root;

	while(key) {
		if (!json_is_object(curr_obj)) goto exit_fail;
		curr_obj = json_object_get(curr_obj, key);
		if (!curr_obj) goto exit_fail;
		key = strtok_r(NULL, "/", &saveptr);
	}

	free(dup);
	return curr_obj;

exit_fail:
	free(dup);
	return NULL;
}

struct fuse_operations get_fuse_op(void)
{
	struct fuse_operations op = {
		.getattr = jsonfs_getattr,
		.readdir = jsonfs_readdir,
		.read	 = jsonfs_read,
		.destroy = jsonfs_destroy
	};

	return op;
}

struct private_args get_fuse_args(int argc, char **argv)
{
	struct private_args args;
	args.fuse_argc = argc - 1;
	args.fuse_argv = calloc(args.fuse_argc, sizeof(char *));
	if (!args.fuse_argv) {
		exit(EXIT_FAILURE);
	}
	args.fuse_argv[0] = argv[0];
	for (int i = 2; i < argc; i++) {
		args.fuse_argv[i - 1] = argv[i];
	}

	return args;
}

int count_subdirs(json_t *obj)
{
	int count = 0;
	const char *key = NULL;
	json_t *value = NULL;

	if (!obj || !json_is_object(obj)) {
		return 0;
	}

	json_object_foreach(obj, key, value) {
		if (json_is_object(value)) {
			count++;
		}
	}

	return count;
}

json_t *convert_to_obj(json_t *root, int is_root)
{
	json_t *obj = NULL;
	json_t *value = NULL;
	const char *key = NULL;
	json_t *json_copy_ret = NULL;
	json_t *converted_val = NULL;
	size_t i;
	
	CHECK_POINTER(root, NULL);
	obj = json_object();
	CHECK_POINTER(obj, NULL);

	if (json_is_object(root)) {
        json_object_foreach(root, key, value) {
            converted_val = convert_to_obj(value, 0);
            CHECK_POINTER(converted_val, NULL);
            json_object_set_new(obj, key, converted_val);
        }
	}
	else if (json_is_array(root)) {
		value = NULL;
		converted_val = NULL;
        json_array_foreach(root, i, value) {
            char key[32];
            snprintf(key, sizeof(key), "%s%zu", SPECIAL_PREFIX, i);

            converted_val = convert_to_obj(value, 0);
            CHECK_POINTER(converted_val, NULL);
            json_object_set_new(obj, key, converted_val);
        }
	} 
	else {
		if (is_root) {
			json_copy_ret = json_copy(root);
			CHECK_POINTER(json_copy_ret, NULL);
			json_object_set_new(obj, SPECIAL_PREFIX"scalar", json_copy_ret);
		}
		else {
			obj = json_copy(root);
			CHECK_POINTER(obj, NULL);
		}
    }

	return obj;
}

int is_special_file(const char *path)
{
	if (strcmp("/.status", path) == 0 ||
		strcmp("/.save", path) == 0) {
			return 1;
	}
	return 0;
}

void handle_special_file(const char *path, struct stat *st)
{
	if (strcmp("/.status", path) == 0) {
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		st->st_size = 1; /* FIXME */
	}
	else if (strcmp("/.save", path) == 0) {
		st->st_mode = S_IFREG | 0666;
		st->st_nlink = 1;
		st->st_size = 1;
	}
}

void handle_json_file(json_t *node, struct stat *st)
{
	if (json_is_object(node)) {
		st->st_mode = S_IFDIR | 0555;
		st->st_nlink = 2 + count_subdirs(node);
	}
	else {
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		char *str = json_dumps(node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
		st->st_size = str ? strlen(str) : 0;
		free(str);
	}
}