/**
 * @file jsonfs.c
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
