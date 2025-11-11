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
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "jsonfs.h"
#include <stdio.h>

extern int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi);
extern int jsonfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
				   off_t offset, struct fuse_file_info *fi,
				   enum fuse_readdir_flags flags);
extern int jsonfs_read(const char *path, char *buffer, size_t size,
				off_t offset, struct fuse_file_info *fi);
extern int jsonfs_write(const char *path, const char *buffer, size_t size,
				 off_t offset, struct fuse_file_info *fi);
extern void jsonfs_destroy(void *userdata);



struct jsonfs_private_data *init_private_data(json_t *json_root, const char *path)
{
	CHECK_POINTER(json_root, NULL);
	CHECK_POINTER(path, NULL);

	struct jsonfs_private_data *pd = calloc(1, sizeof(struct jsonfs_private_data));
	CHECK_POINTER(pd, NULL);

	pd->root = json_root;
	pd->path_to_json_file = strdup(path);
	pd->is_saved = 1;
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
		.write	 = jsonfs_write,
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

int replace_json_value(const char *path, const char *buffer, size_t size,
					   off_t offset, struct jsonfs_private_data *pd)
{
	json_t *old_node = NULL;
	json_t *new_node = NULL;
	char *content = NULL;
	char *new_content = NULL;
	size_t content_len;
	int res_replace;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(buffer, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	old_node = find_node_by_path(path, pd->root);
	CHECK_POINTER(old_node, -ENOENT);

	content = json_dumps(old_node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(content, -ENOMEM);

	content_len = strlen(content);

	if (size + offset > content_len) {
		new_content = realloc(content, size + offset + 1);
		if (!new_content) { goto handle_error; }
		content = new_content;
		content_len = size + offset;
	}

	for(int i = offset, j = 0; i < content_len && j < size; i++, j++) {
		content[i] = buffer[j];
	}

	//if (content_len) { content[content_len - 1] = '\0'; }

#if 1
	fprintf(stderr, "content_len: %d\n", content_len);
	fprintf(stderr, "content: %s\n", content);
#endif

	new_node = json_loads(content, JSON_DECODE_ANY, NULL);
	if (!new_node) { goto handle_error; }

	res_replace = replace_nodes(old_node, new_node, pd);
	if (res_replace) { 
		json_decref(new_node);
		free(content);
		return res_replace;
	}

	json_decref(old_node); /* XXX */
	free(content);
	return 0;

	handle_error:
	json_decref(new_node);
	free(content);
	return -ENOMEM;
}

int replace_nodes(json_t *old_node, json_t *new_node, 
				  struct jsonfs_private_data *pd)
{
	json_t *parent = NULL;
	const char *key = NULL;
	int res_find;
	int res_set;

	CHECK_POINTER(old_node, -EFAULT);
	CHECK_POINTER(new_node, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	res_find = find_parent_key_index(pd->root, old_node, &parent, &key);
	if (res_find) { return -ENOENT; }

	if (json_is_object(parent)) {
		res_set = json_object_set(parent, key, new_node);	
	}

	if (res_set) { return -EINVAL; }
	return 0;
}

int find_parent_key_index(json_t *root, json_t *node, json_t **parent, 
						  const char **key)
{
	json_t *v = NULL;
	const char *k = NULL;
	int res_find;

	CHECK_POINTER(root, -EFAULT);
    CHECK_POINTER(node, -EFAULT);
    CHECK_POINTER(parent, -EFAULT);
    CHECK_POINTER(key, -EFAULT);

	if (json_equal(root, node)) { return 1; }

	if (json_is_object(root)) {
		json_object_foreach(root, k, v) {
			if (json_equal(node, v)) {
				*parent = root;
				*key = k;
				return 0;
			}

			res_find = find_parent_key_index(v, node, parent, key);
			if (!res_find) { return 0; }
		}
	}

	return 1;
}
