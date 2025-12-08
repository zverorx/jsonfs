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
#include "json_operations.h"
#include "file_time.h"

int getattr_special_file(const char *path, struct stat *st,
						 struct jsonfs_private_data *pd)
{
	int is_saved;
	struct file_time *ft = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(st, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);
    
    if (!is_special_file(path)) {
        return -EINVAL;
    }
	
	is_saved = pd->is_saved;

	st->st_uid = pd->uid;
	st->st_gid = pd->gid;

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		st->st_atime = ft->atime;
		st->st_mtime = ft->mtime;
		st->st_ctime = ft->ctime;
	}
	else {
		st->st_atime = pd->ft->atime;
		st->st_mtime = pd->ft->mtime;
		st->st_ctime = pd->ft->ctime;
	}

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
	json_t *node = NULL;
	struct file_time *ft = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(st, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);
    
	st->st_uid = pd->uid;
	st->st_gid = pd->gid;

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		st->st_atime = ft->atime;
		st->st_mtime = ft->mtime;
		st->st_ctime = ft->ctime;
	}
	else {
		st->st_atime = pd->ft->atime;
		st->st_mtime = pd->ft->mtime;
		st->st_ctime = pd->ft->ctime;
	}

	node = find_json_node(path, pd->root);
	CHECK_POINTER(node, -ENOENT);

	if (json_is_object(node)) {
		st->st_mode = S_IFDIR | 0775;
		st->st_nlink = 2 + count_subdirs(node);
	}
	else {
		st->st_mode = S_IFREG | 0666;
		st->st_nlink = 1;
		char *str = json_dumps(node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
		CHECK_POINTER(str, -ENOMEM);
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
	struct file_time *ft = NULL;
	time_t now = time(NULL);

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

    if (!is_special_file(path)) {
        return -EINVAL;
    }

	is_saved = pd->is_saved;

	if (strcmp("/.status", path) == 0) {
		text = is_saved ? "SAVED\n" : "UNSAVED\n";
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

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->atime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_ATIME | SET_CTIME);
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
	time_t now = time(NULL);
	struct file_time *ft = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	node = find_json_node(path, pd->root);
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

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->atime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_ATIME | SET_CTIME);
	}

	return (int)final_size;
}

int write_special_file(const char *path, const char *buffer, size_t size,
					   off_t offset, struct jsonfs_private_data *pd)
{
	int res_save;
	time_t now = time(NULL);
	struct file_time *ft = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(buffer, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	if (!is_special_file(path)) { 
		return -EINVAL; 
	}

	if (strcmp("/.save", path) != 0) {
		return -EACCES;
	}

	res_save = json_dump_file(pd->root, pd->path_to_json_file, 
							  JSON_INDENT(2) | JSON_ENCODE_ANY);
	if (res_save < 0) { return -EINVAL; }

	pd->is_saved = 1;

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->mtime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_MTIME | SET_CTIME);
	}

	return (int) size;
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
	time_t now = time(NULL);
	struct file_time *ft = NULL;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(buffer, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);

	root = pd->root;
	CHECK_POINTER(root, -EFAULT);

	old_node = find_json_node(path, root);
	CHECK_POINTER(old_node, -ENOENT);

	content = json_dumps(old_node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(content, -ENOMEM);

	content_len = strlen(content);

	if (size + offset > content_len) {
		res_realloc = realloc(content, size + offset + 1);
		if (!res_realloc) { ret = -ENOMEM; goto handle_error; }
		content = res_realloc;

		content_len = size + offset;
		content[content_len] = '\0';
	}

	for(int i = offset, j = 0; i < content_len && j < size; i++, j++) {
		content[i] = buffer[j];
	}

#	if 0
	fputs("=== start write ===\n", stderr);
	fprintf(stderr, "content: %s\n", content);
	fprintf(stderr, "content_len: %ld\n", content_len);
	fprintf(stderr, "size: %ld\n", size);
	fprintf(stderr, "offset: %ld\n", offset);
	fprintf(stderr, "buffer:\n");
	for(int i = 0; i < size; i++) {
		fprintf(stderr, "el[%d]: %c\n", i, buffer[i]);
	}
	fputs("=== end write ===\n", stderr);
#	endif

	new_node = json_loads(content, JSON_DECODE_ANY, NULL);
	if (!new_node) { ret = -EINVAL; goto handle_error; }

	res_replace = replace_json_nodes(old_node, new_node, root);
	if (res_replace) { ret = -ENOENT; goto handle_error; }

	pd->is_saved = 0;

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->mtime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_MTIME | SET_CTIME);
	}

	free(content);
	return ret;

	handle_error:
		json_decref(new_node);
		free(content);
		return ret;
}

int rm_file(const char *path, int file_type, struct jsonfs_private_data *pd)
{
	json_t *node = NULL;
	json_t *parent = NULL;
	const char *node_key = NULL;
	int res_find;
	size_t size;

	CHECK_POINTER(path, -EFAULT);
	CHECK_POINTER(pd, -EFAULT);
	if (file_type != S_IFREG && file_type != S_IFDIR) { return -EINVAL; }

	node = find_json_node(path, pd->root);
	if (!node && is_special_file(path)) { return -EPERM; }
	else if (!node) { return -ENOENT; }

	switch (file_type) {
		case S_IFREG:
			if (json_is_object(node)) { return -EISDIR; }
			break;
		case S_IFDIR:
			if (!json_is_object(node)) { return -ENOTDIR; }
			if (strcmp(path, "/") == 0) { return -EBUSY; }

			size = json_object_size(node);
			if (size) { return -ENOTEMPTY; }	
			break;
	}

	res_find = find_parent_and_key(pd->root, node, &parent, &node_key);
	if (res_find < 0) { return res_find; }

	json_object_del(parent, node_key);
	remove_node_to_list_ft(path, pd->ft);

	return 0;
}

int make_file(const char *path, mode_t mode, struct jsonfs_private_data *pd)
{
	int res_set;
	size_t path_size;
	size_t key_size;
	char key[MID_SIZE];
	char *parent_path = NULL;
	json_t *new_node = NULL;
	json_t *parent = NULL;
	struct file_time *ft = NULL;
	time_t now = time(NULL);
	int type;

	if ((mode & S_IFMT) == S_IFREG) {
		type = S_IFREG;
	}
	else if ((mode & S_IFMT) == 0) {
		type = S_IFDIR;
	}
	else {
		return -EINVAL; 
	}

	parent_path = strdup(path);
	CHECK_POINTER(parent_path, -ENOMEM);

	path_size = strlen(parent_path);
	key_size = 0;

	for (int i = path_size - 1; i >= 0; i--) {
		if (parent_path[i] == '/') { 
			key_size = path_size - i - 1;
			if (key_size >= MID_SIZE) {
				free(parent_path);
				return -ENAMETOOLONG;
			}
			strncpy(key, &parent_path[i + 1], key_size);
			key[key_size] = '\0';
			parent_path[i + 1] = '\0';
			break; 
		}
	}

	if (key_size == 0) { 
		free(parent_path); 
		return -EINVAL; 
	}

	parent = find_json_node(parent_path, pd->root);
	if (!parent) { 
		free(parent_path); 
		return -ENOENT; 
	}

	if (json_object_get(parent, key)) {
	    free(parent_path);
	    return -EEXIST;
	}

	if (type == S_IFREG) {
		new_node = json_integer(0);
	}
	else if (type == S_IFDIR) {
		new_node = json_object();
	}

	if (!new_node) { 
		free(parent_path); 
		return -ENOMEM; 
	}

	res_set = json_object_set_new(parent, key, new_node);
	if (res_set < 0) { 
		free(parent_path);	
		return -EIO; 
	}

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->mtime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_MTIME | SET_CTIME);
	}

	free(parent_path);
	return 0;
}

int rename_file(const char *old_path, const char *new_path, 
				struct jsonfs_private_data *pd)
{
	json_t *node = NULL;
	json_t *old_parent = NULL;
	json_t *new_parent = NULL;
	char *old_parent_path = NULL;
	char *old_name = NULL;
	char *new_parent_path = NULL;
	char *new_name = NULL;
	int res_sep;
	int res_set;
	int res_rename = 0;

	CHECK_POINTER(old_path, -EINVAL);
	CHECK_POINTER(new_path, -EINVAL);
	CHECK_POINTER(pd, -EINVAL);

 	node = find_json_node(old_path, pd->root);
 	CHECK_POINTER(node, -ENOENT);

	res_sep = separate_filepath(old_path, &old_parent_path, &old_name);
	if (res_sep < 0) {
		res_rename = -EINVAL;
		goto handle_error;
	}

	res_sep = separate_filepath(new_path, &new_parent_path, &new_name);
	if (res_sep < 0) {
		res_rename = -EINVAL;
		goto handle_error;
	}

	if (strcmp(old_parent_path, ".") == 0) {
		old_parent = pd->root;
	}
	else {
		old_parent = find_json_node(old_parent_path, pd->root);
		if (!old_parent) {
			res_rename = -ENOENT;
			goto handle_error;
		}
	}

	if (strcmp(new_parent_path, ".") == 0) {
		new_parent = pd->root;
	}
	else {
		new_parent = find_json_node(new_parent_path, pd->root);
		if (!new_parent) {
			res_rename = -ENOENT;
			goto handle_error;
		}
	}

	if (!json_is_object(new_parent)) {
		res_rename = -ENOTDIR;
		goto handle_error;
	}

	if (strncmp(old_path, new_path, strlen(old_path)) == 0 &&
		(strlen(new_path) > strlen(old_path)) &&
		new_path[strlen(old_path)] == '/') {
		res_rename = -EINVAL;
		goto handle_error;
	}

	json_incref(node);

	res_set = json_object_set_new(new_parent, new_name, node);
	if (res_set < 0) {
		json_decref(node);
		res_rename = -EIO;
		goto handle_error;
	}

	json_object_del(old_parent, old_name);

	remove_node_to_list_ft(old_path, pd->ft);

	handle_error:
		free(old_parent_path);
    	free(old_name);
    	free(new_parent_path);
   		free(new_name);
		return res_rename;
}

int trunc_json_file(const char *path, off_t offset, 
					struct jsonfs_private_data *pd)
{
#	if 0
	fputs("=== trunc start ===\n", stderr);
	fprintf(stderr, "path: %s\n", path);
	fprintf(stderr, "offset: %ld\n", offset);
#	endif

	size_t content_len;
	json_t *old_node = NULL;
	json_t *new_node = NULL;
	struct file_time *ft = NULL;
	time_t now = time(NULL);
	char *res_realloc = NULL;
	char *content = NULL;
	int res_replace;
	int ret = 0;

	CHECK_POINTER(path, -EINVAL);
	CHECK_POINTER(pd, -EINVAL);
	if (offset < 0) { return -EINVAL; }

	old_node = find_json_node(path, pd->root);
	CHECK_POINTER(old_node, -ENOENT);

	content = json_dumps(old_node, JSON_ENCODE_ANY | JSON_REAL_PRECISION(10));
	CHECK_POINTER(content, -ENOMEM);

	content_len = strlen(content);

	if (offset == 0) {
		new_node = json_integer(0); 
		res_replace = replace_json_nodes(old_node, new_node, pd->root);
		if (res_replace) { ret = -ENOENT; goto handle_error; }
		return ret;
	}

	if (content_len != offset) {
		res_realloc = realloc(content, offset + 1);
		if (!res_realloc) { ret = -ENOMEM; goto handle_error; }
		content = res_realloc;

		content[offset] = '\0';

		if (content_len < offset) {
			memset(content + content_len, 0, offset - content_len);
		}
	}

	new_node = json_loads(content, JSON_DECODE_ANY, NULL);
	if (!new_node) { ret = -EINVAL; goto handle_error; }

	res_replace = replace_json_nodes(old_node, new_node, pd->root);
	if (res_replace) { ret = -ENOENT; goto handle_error; }

	ft = find_node_file_time(path, pd->ft);
	if (ft) {
		ft->mtime = now;
		ft->ctime = now;
	}
	else {
		add_node_to_list_ft(path, pd->ft, SET_MTIME | SET_CTIME);
	}

	return ret;

	handle_error:
		json_decref(new_node);
		free(content);
		return ret;
}
