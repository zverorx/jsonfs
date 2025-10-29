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
#include <errno.h>

#include "common.h"
#include "jsonfs.h"

int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi)
{
	json_t *node = NULL;
	int res;

	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	memset(st, 0, sizeof(struct stat));

	if (is_special_file(path)) {
		res = getattr_special_file(path, st, pd);
		if (res) { return res; }
	}
	else {
		res = getattr_json_file(path, st, pd);
		if (res) { return res; }
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
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	FILL_OR_RETURN(buffer, ".");
	FILL_OR_RETURN(buffer, "..");
	if (strcmp("/", path) == 0) {
		FILL_OR_RETURN(buffer, ".status");
		FILL_OR_RETURN(buffer, ".save");
	}

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	if (!json_is_object(node)) {
		return -ENOTDIR;
	}

	json_object_foreach(node, key, value) {
		FILL_OR_RETURN(buffer, key);
	}

	return 0;
}

int jsonfs_read(const char *path, char *buffer, size_t size,
				off_t offset, struct fuse_file_info *fi)
{
	int len;
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	if (is_special_file(path)) {
		len = read_special_file(path, buffer, size, offset, pd);
	}
	else {
		len = read_json_file(path, buffer, size, offset, pd);
	}

	return len;
}

void jsonfs_destroy(void *userdata)
{
	if (!userdata) {
		return;
	}

	struct jsonfs_private_data *pd = (struct jsonfs_private_data *)userdata;

	if (pd->root) {
		json_decref(pd->root);
	}

	free(pd->path_to_json_file);
	free(pd);
}
