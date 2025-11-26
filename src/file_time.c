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
 * @brief Contains definition of functions for working with struct file_time.
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "file_time.h"

void free_file_time(struct file_time *ft)
{
	free(ft->path);
	free(ft);
	return;
}

struct file_time *add_node_to_list_ft(const char* path, struct file_time *root, 
									  enum set_time flags)
{
	time_t now;
	char *path_dup = NULL;
	struct file_time *new_node = NULL;
	struct file_time *last = NULL;

	CHECK_POINTER(path, NULL);

	last = root;
	while(last) {
		if (strcmp(path, last->path) == 0) {
			return NULL;
		}
		last = last->next_node;
	}

	path_dup = strdup(path);
	CHECK_POINTER(path_dup, NULL);

	new_node = calloc(1, sizeof(struct file_time));
	CHECK_POINTER(new_node, NULL);
	
	now = time(NULL);
	new_node->path = path_dup;

	if (flags & SET_ATIME || !root) { new_node->atime = now; }
	else { new_node->atime = root->atime; }

	if (flags & SET_MTIME || !root) { new_node->mtime = now; }
	else { new_node->mtime = root->mtime; }

	if (flags & SET_CTIME || !root) { new_node->ctime = now; }
	else { new_node->ctime = root->ctime; }

	new_node->next_node = NULL;

	if(!root) { return new_node; }

	last = root;
	while(last->next_node) {
		last = last->next_node;
	}

	last->next_node = new_node;

	return new_node;
}

int remove_node_to_list_ft(const char* path, struct file_time *root)
{
	struct file_time *node = NULL;
	struct file_time *parent = NULL;
	char *parent_path = NULL;
	size_t path_size;

	CHECK_POINTER(path, -1);
	CHECK_POINTER(root, -1);

	node = find_node_file_time(path, root);
	CHECK_POINTER(node, -1);

	parent_path = strdup(path);
	path_size = strlen(path);

	for (int i = path_size - 1; i >= 0 && path[i] != '/'; i--) {
		parent_path[i] = '\0';
	}

#	if 0
	fprintf(stderr, "Parent path: %s\n", parent_path);
#	endif

	parent = find_node_file_time(parent_path, root);
	CHECK_POINTER(parent, -1);
	parent->next_node = node->next_node;

	free(parent_path);
	free_file_time(node);

	return 0;
}

struct file_time *find_node_file_time(const char *path, struct file_time *root)
{
	struct file_time *res = NULL;
	struct file_time *curr = NULL;

	CHECK_POINTER(path, NULL);
	CHECK_POINTER(root, NULL);

	curr = root;
	while(curr) {
		if (strcmp(path, curr->path) == 0) {
			res = curr;
			break;
		}
		curr = curr->next_node;
	}

	return res;
}
