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
 * @brief Contains handlers for callbacks.
 *
 * Function declarations and specifications can be found in handlers.h.
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

int getattr_special_file(const char *path, struct stat *st,
						 struct jsonfs_private_data *pd)
{
	time_t now;
	int is_saved;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(st, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);
    
    if (!is_special_file(path)) {
        return -EINVAL;
    }
	
	is_saved = pd->is_saved;

	now = time(NULL);
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = now; 
	st->st_mtime = now;
	st->st_ctime = now;

	if (strcmp("/.status", path) == 0) {
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		st->st_size = is_saved ? strlen("SAVED") : strlen("UNSAVED");
	}
	else if (strcmp("/.save", path) == 0) {
		st->st_mode = S_IFREG | 0666;
		st->st_nlink = 1;
		st->st_size = 1;
	}
	return 0;
}

int getattr_json_file(const char *path, struct stat *st,
					  struct jsonfs_private_data *pd)
{
	time_t now;
	json_t *node = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(st, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);
    
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
		st->st_nlink = 2 + count_subdirs(node);
	}
	else {
		st->st_mode = S_IFREG | 0666;
		st->st_nlink = 1;
		char *str = json_dumps(node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
		CHECK_POINTER(str, 1);
		st->st_size = str ? strlen(str) : 0;
		free(str);
	}
	return 0;
}

int read_special_file(const char *path, char *buffer, size_t size,
					  off_t offset, struct jsonfs_private_data *pd)
{
	char *text = NULL;
	int is_saved;
	size_t text_len;
	size_t final_size = 0;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

    if (!is_special_file(path)) {
        return -EINVAL;
    }

	is_saved = pd->is_saved;

	if (strcmp("/.status", path) == 0) {
		text = is_saved ? "SAVED" : "UNSAVED";

	}
	else if (strcmp("/.save", path) == 0) {
		text = is_saved ? "0" : "1";
	}
	
	text_len = strlen(text);
	if (offset < text_len) {
		final_size = text_len - offset;
		if (final_size > size) {
			final_size = size;
		}
		memcpy(buffer, text + offset, final_size);
	}

	return (int)final_size;
}

int read_json_file(const char *path, char *buffer, size_t size,
				   off_t offset, struct jsonfs_private_data *pd)
{
	json_t *node = NULL;
	char *text = NULL;
	size_t text_len;
	size_t final_size = 0;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	node = find_node_by_path(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	
	text = json_dumps(node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(text, -ENOMEM);

	text_len = strlen(text);
	if (offset < text_len) {
		final_size = text_len - offset;
		if (final_size > size) {
			final_size = size;
		}
		memcpy(buffer, text + offset, final_size);
	}
	free(text);

	return (int)final_size;
}

int write_special_file(const char *path, const char *buffer, size_t size,
					   off_t offset, struct jsonfs_private_data *pd)
{
	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(buffer, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	if (!is_special_file(path)) { 
		return -EINVAL; 
	}

	if (strcmp("/.save", path) == 0) {
		pd->is_saved = 1;
		return (int) size;
	}

	return -EACCES;
}

int write_json_file(const char *path, const char *buffer, size_t size,
					off_t offset, struct jsonfs_private_data *pd)
{
	json_t *old_node = NULL;
	json_t *new_node = NULL;
	json_t *root = NULL;
	char *content = NULL;
	void *res_realloc = NULL;
	size_t content_len;
	int res_replace;
	int ret = (int) size;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(buffer, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	root = pd->root;
	CHECK_POINTER(root, -EFAULT);

	old_node = find_node_by_path(path, root);
	CHECK_POINTER(old_node, -ENOENT);

	content = json_dumps(old_node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(content, -ENOMEM);

	content_len = strlen(content);

	if (size + offset > content_len) {
		res_realloc = realloc(content, size + offset + 1);
		if (!res_realloc) { ret = -ENOMEM; goto handle_error; }
		content = res_realloc;
	}

	content_len = size + offset;
	content[content_len] = '\0';

	for(int i = offset, j = 0; i < content_len && j < size; i++, j++) {
		content[i] = buffer[j];
	}

#	if 0
	fprintf(stderr, "content: %s\n", content);
	fprintf(stderr, "content_len: %d\n", content_len);
	fprintf(stderr, "size: %d\n", size);
	fprintf(stderr, "offset: %d\n", offset);
	for(int i = 0; i < size; i++) {
		fprintf(stderr, "el[%d]: %c\n", i, buffer[i]);
	}
#	endif

	new_node = json_loads(content, JSON_DECODE_ANY, NULL);
	if (!new_node) { ret = -EINVAL; goto handle_error; }

	res_replace = replace_nodes(old_node, new_node, root);
	if (res_replace) { ret = -ENOENT; goto handle_error; }

	pd->is_saved = 0;

	json_decref(old_node);
	free(content);
	return ret;

	handle_error:
	json_decref(new_node);
	free(content);
	return ret;
}