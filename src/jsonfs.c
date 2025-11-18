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
#include <stdio.h>

#include "common.h"
#include "jsonfs.h"

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
	int count_byte;
	char cwd[MID_SIZE];
	char full_path[BIG_SIZE];
	time_t now = time(NULL);

	CHECK_POINTER(json_root, NULL);
	CHECK_POINTER(path, NULL);

	struct jsonfs_private_data *pd = calloc(1, sizeof(struct jsonfs_private_data));
	CHECK_POINTER(pd, NULL);

	pd->root = json_root;

	if (!getcwd(cwd, sizeof(cwd))) { goto handle_error; }

	count_byte = snprintf(full_path, sizeof(full_path), "%s/%s", cwd, path);
	if (count_byte >= sizeof(full_path)) { goto handle_error; }

	pd->path_to_json_file = strdup(full_path);
	if (!pd->path_to_json_file) { goto handle_error; }

	pd->ft = add_node_to_list_ft("/", NULL, SET_ATIME | SET_MTIME | SET_CTIME);
	if (!pd->ft) { goto handle_error; }

	pd->mount_time = now;
	pd->uid = getuid();
	pd->gid = getgid();
	pd->is_saved = 1;

	return pd;
	
	handle_error:
		json_decref(pd->root);
		free(pd->path_to_json_file);
		free(pd);
		return NULL;
}

void destroy_private_data(struct jsonfs_private_data *pd)
{
	struct file_time *next = NULL;
	struct file_time *curr = NULL;

	if (!pd) { return; }

	if (pd->root) {
		json_decref(pd->root);
	}

	free(pd->path_to_json_file);

	curr = pd->ft;
	while(curr) {
		next = curr->next_node;
		free(curr->path);
		free(curr);
		curr = next;
	}

	free(pd);
}

struct file_time *add_node_to_list_ft(const char* path, struct file_time *root, 
									   enum set_time flags)
{
	time_t now;
	char *path_dup = NULL;
	struct file_time *new_node = NULL;
	struct file_time *last = NULL;

	CHECK_POINTER(path, NULL);

	last = root;
	while(last) {
		if (strcmp(path, last->path) == 0) {
			return NULL;
		}
		last = last->next_node;
	}

	path_dup = strdup(path);
	CHECK_POINTER(path_dup, NULL);

	new_node = calloc(1, sizeof(struct file_time));
	CHECK_POINTER(new_node, NULL);
	
	now = time(NULL);
	new_node->path = path_dup;

	if (flags & SET_ATIME || !root) { new_node->atime = now; }
	else { new_node->atime = root->atime; }

	if (flags & SET_MTIME || !root) { new_node->mtime = now; }
	else { new_node->mtime = root->mtime; }

	if (flags & SET_CTIME || !root) { new_node->ctime = now; }
	else { new_node->ctime = root->ctime; }

	new_node->next_node = NULL;

	if(!root) { return new_node; }

	last = root;
	while(last->next_node) {
		last = last->next_node;
	}

	last->next_node = new_node;

	return new_node;
}
struct file_time *find_node_file_time(const char *path, struct file_time *root)
{
	struct file_time *res = NULL;
	struct file_time *curr = NULL;

	CHECK_POINTER(path, NULL);
	CHECK_POINTER(root, NULL);

	curr = root;
	while(curr) {
		if (strcmp(path, curr->path) == 0) {
			res = curr;
			break;
		}
		curr = curr->next_node;
	}

	return res;
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
	if (!key) goto handle_error;

	json_t *curr_obj = root;

	while(key) {
		if (!json_is_object(curr_obj)) goto handle_error;
		curr_obj = json_object_get(curr_obj, key);
		if (!curr_obj) goto handle_error;
		key = strtok_r(NULL, "/", &saveptr);
	}

	free(dup);
	return curr_obj;

	handle_error:
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
            char key[SHRT_SIZE];
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

int replace_nodes(json_t *old_node, json_t *new_node, json_t *root)
{
	json_t *parent = NULL;
	const char *key = NULL;
	int res_find;
	int res_set;

	CHECK_POINTER(old_node, -EFAULT);
	CHECK_POINTER(new_node, -EFAULT);
	CHECK_POINTER(root, -EFAULT);

	res_find = find_parent_key(root, old_node, &parent, &key);
	if (res_find) { return res_find; }

	if (json_is_object(parent)) {
		res_set = json_object_set(parent, key, new_node);	
	}

	if (res_set) { return -EINVAL; }
	return 0;
}

int find_parent_key(json_t *root, json_t *node, json_t **parent, 
					const char **key)
{
	json_t *v = NULL;
	const char *k = NULL;
	int res_find;

	CHECK_POINTER(root, -EFAULT);
    CHECK_POINTER(node, -EFAULT);
    CHECK_POINTER(parent, -EFAULT);
    CHECK_POINTER(key, -EFAULT);

	if (json_equal(root, node)) { return -EINVAL; }

	if (json_is_object(root)) {
		json_object_foreach(root, k, v) {
			if (json_equal(node, v)) {
				*parent = root;
				*key = k;
				return 0;
			}

			res_find = find_parent_key(v, node, parent, key);
			if (!res_find) { return 0; }
		}
	}

	return -ENOENT;
}
