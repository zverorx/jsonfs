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
 * @brief FUSE filesystem operation handlers.
 *
 * Implements callback functions for FUSE filesystem operations,
 * including destroy, getattr, readdir, and read.
 */

#define FUSE_USE_VERSION 35

/* Includes */
#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "jsonfs.h"

int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi)
{
	time_t now;
	json_t *node;

	(void) fi;

	memset(st, 0, sizeof(struct stat));
	now = time(NULL);

	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = now; 
	st->st_mtime = now;
	st->st_ctime = now;

	if (is_special_file(path)) {
		handle_special_file(path, st);
	}
	else {
		struct fuse_context *ctx = fuse_get_context();
		struct json_private_data *pd = ctx->private_data;

		node = find_node_by_path(path, pd->root);
		CHECK_POINTER(node, -ENOENT);

		handle_json_file(node, st);
	}

	return 0;
}

int jsonfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
				   off_t offset, struct fuse_file_info *fi,
				   enum fuse_readdir_flags flags)
{
	json_t *node;
	const char *key;
    json_t *value;

	(void) offset;
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct json_private_data *pd = ctx->private_data;

	FILL_OR_RETURN(buffer, ".");
	FILL_OR_RETURN(buffer, "..");

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	if (!json_is_object(node)) {
		return -ENOTDIR;
	}

	json_object_foreach(node, key, value) {
		FILL_OR_RETURN(buffer, key);
	}

	if (strcmp("/", path) == 0) {
		FILL_OR_RETURN(buffer, ".status");
		FILL_OR_RETURN(buffer, ".save");
	}

	return 0;
}

int jsonfs_read(const char *path, char *buffer, size_t size,
				off_t offset, struct fuse_file_info *fi)
{
	char *text;
	size_t text_len;
	size_t copy_size = 0;
	json_t *node;

	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct json_private_data *pd = ctx->private_data;

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	text = json_dumps(node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(text, -ENOMEM);

	text_len = strlen(text);
	if (offset < text_len) {
		copy_size = text_len - offset;
		if (copy_size > size) {
			copy_size = size;
		}
		memcpy(buffer, text + offset, copy_size);
	}

	free(text);
	return (int)copy_size;
}

void jsonfs_destroy(void *userdata)
{
	if (!userdata) {
		return;
	}

	struct json_private_data *pd = (struct json_private_data *)userdata;

	if (pd->root) {
		json_decref(pd->root);
	}

	free(pd->path_to_json_file);
	free(pd);
}
