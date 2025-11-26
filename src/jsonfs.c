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
 * @brief Functions for getting arguments to fuse_main(). 
 */

#define FUSE_USE_VERSION 35

#include <jansson.h>
#include <fuse.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "file_time.h"
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
extern int jsonfs_unlink(const char *path);
extern int jsonfs_rmdir (const char *path);
extern int jsonfs_mknod(const char *path, mode_t mode, dev_t dev);
extern int jsonfs_mkdir(const char *path, mode_t mode);
extern int jsonfs_utimens(const char *path, const struct timespec tv[2], 
                          struct fuse_file_info *fi);
extern void jsonfs_destroy(void *userdata);

struct fuse_operations get_fuse_op(void)
{
	struct fuse_operations op = {
		.getattr = jsonfs_getattr,
		.readdir = jsonfs_readdir,
		.read	 = jsonfs_read,
		.write	 = jsonfs_write,
		.unlink	 = jsonfs_unlink,
		.rmdir	 = jsonfs_rmdir,
		.mknod	 = jsonfs_mknod,
 		.mkdir	 = jsonfs_mkdir,
		.utimens = jsonfs_utimens,
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
		free_file_time(curr);
		curr = next;
	}

	free(pd);
}
