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
		st->st_nlink = 2; /* TODO: + number of child directories */
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
	int ret_fill;
	json_t *node;

	const char *key;
    json_t *value;

	(void) offset;
	(void) fi;

	struct fuse_context *ctx = fuse_get_context();
	struct json_private_data *pd = ctx->private_data;

	ret_fill = filler(buffer, ".", NULL, 0,  FUSE_FILL_DIR_PLUS);
	if (ret_fill) {
		return -ENOMEM;
	}

	ret_fill = filler(buffer, "..", NULL, 0, FUSE_FILL_DIR_PLUS);
	if (ret_fill) {
		return -ENOMEM;
	}

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	json_object_foreach(node, key, value) {
		ret_fill = filler(buffer, key, NULL, 0, FUSE_FILL_DIR_PLUS);
		if (ret_fill) {
			return -ENOMEM;
		}
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

	text = json_dumps(node, JSON_ENSURE_ASCII | JSON_ENCODE_ANY);
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
