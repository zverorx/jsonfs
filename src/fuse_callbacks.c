/**
 * @file fuse_callbacks.c
 */

#define FUSE_USE_VERSION 35

/* Includes */
#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

/* TODO: Implement callbacks logic */
int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi)
{
	return 0;
}

int jsonfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
				   off_t offset, struct fuse_file_info *fi,
				   enum fuse_readdir_flags flags)
{
	return 0;
}

int jsonfs_read(const char *path, char *buffer, size_t size,
				off_t offset, struct fuse_file_info *fi)
{
	return 0;
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

struct fuse_operations jsonfs_get_fuse_op(void)
{
	struct fuse_operations op = {
		.getattr = jsonfs_getattr,
		.readdir = jsonfs_readdir,
		.read	 = jsonfs_read,
		.destroy = jsonfs_destroy
	};

	return op;
}
