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
 * including destroy, getattr, readdir, read, write, unlink, 
 * rmdir, mknode, mkdir, utimens.
 */

#define FUSE_USE_VERSION 35

#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common.h"
#include "handlers.h"
#include "file_time.h"
#include "json_operations.h"

int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi)
{
	int res_getattr;

	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	memset(st, 0, sizeof(struct stat));

	if (is_special_file(path)) {
		res_getattr = getattr_special_file(path, st, pd);
		if (res_getattr) { return res_getattr; }
	}
	else {
		res_getattr = getattr_json_file(path, st, pd);
		if (res_getattr) { return res_getattr; }
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

	node = find_json_node(path, pd->root);
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
	int res_read;
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	if (is_special_file(path)) {
		res_read = read_special_file(path, buffer, size, offset, pd);
	}
	else {
		res_read = read_json_file(path, buffer, size, offset, pd);
	}

	return res_read;
}

int jsonfs_write(const char *path, const char *buffer, size_t size,
				 off_t offset, struct fuse_file_info *fi)
{
	int res_write; 
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	if (is_special_file(path)) {
		res_write = write_special_file(path, buffer, size, offset, pd);
	}
	else {
		res_write = write_json_file(path, buffer, size, offset, pd);
	}
	return res_write;
}

int jsonfs_unlink(const char *path)
{
	int res_rm;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	res_rm = rm_file(path, S_IFREG, pd);
	
	return res_rm;
}

int jsonfs_rmdir(const char *path)
{
	int res_rm;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	res_rm = rm_file(path, S_IFDIR, pd);
	
	return res_rm;
}

int jsonfs_mknod(const char *path, mode_t mode, dev_t dev)
{
	int res_mk;

	if (strstr(path, ".sw")) { return -EPERM; }

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);
	
	res_mk = make_file(path, mode, pd);

	return res_mk;
}

int jsonfs_mkdir(const char *path, mode_t mode)
{
	int res_mk;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);
	
	res_mk = make_file(path, mode, pd);

	return res_mk;
}

int jsonfs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi)
{
	struct file_time *ft = NULL;
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct jsonfs_private_data *pd = ctx->private_data;
	CHECK_POINTER(pd, -ENOMEM);

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->atime = tv[0].tv_sec;
		ft->mtime = tv[1].tv_sec;
		ft->ctime =	tv[1].tv_sec; 
	}
	else {
		add_node_to_list_ft(path, pd->ft, 0);
		ft = find_node_file_time(path, pd->ft);
		ft->atime = tv[0].tv_sec;
		ft->mtime = tv[1].tv_sec;
		ft->ctime =	tv[1].tv_sec; 
	}

    return 0;
}

void jsonfs_destroy(void *userdata)
{
	if (!userdata) {
		return;
	}

	struct jsonfs_private_data *pd = (struct jsonfs_private_data *)userdata;
	destroy_private_data(pd);
}
