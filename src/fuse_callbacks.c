/**
 * @file fuse_callbacks.c
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

/* TODO: Implement callbacks logic */
int jsonfs_getattr(const char *path, struct stat *st,
				   struct fuse_file_info *fi)
{
	(void) fi;
	time_t now;
	json_t *node;

	struct fuse_context *ctx = fuse_get_context();
	struct json_private_data *pd = ctx->private_data;

	memset(st, 0, sizeof(struct stat));
	now = time(NULL);

	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = now; 
	st->st_mtime = now;
	st->st_ctime = now;

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	if (json_is_object(node)) {
		st->st_mode = S_IFDIR | 0555;
		st->st_nlink = 2;
	}
	else {
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		char *str = json_dumps(node, JSON_ENSURE_ASCII | JSON_ENCODE_ANY);
		st->st_size = str ? strlen(str) : 0;
		free(str);
	}

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
