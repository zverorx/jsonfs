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
 * @brief Contains definition of functions for working with deserialized JSON file.
 */

#include <jansson.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "common.h"
#include "json_operations.h"

json_t *find_json_node(const char *path, json_t *root)
{
	char *path_dup = NULL;
	char *saveptr = NULL;

	CHECK_POINTER(path, NULL);
	CHECK_POINTER(root, NULL);

	if (strcmp(path, "/") == 0) { return root; }

	path_dup = strdup(path);
	CHECK_POINTER(path_dup, NULL);

	char *key = strtok_r(path_dup, "/", &saveptr);
	if (!key) { goto handle_error; }

	json_t *curr_obj = root;

	while(key) {
		if (!json_is_object(curr_obj)) { goto handle_error; }
		curr_obj = json_object_get(curr_obj, key);
		if (!curr_obj) { goto handle_error; }
		key = strtok_r(NULL, "/", &saveptr);
	}

	free(path_dup);
	return curr_obj;

	handle_error:
		free(path_dup);
		return NULL;
}

int replace_json_nodes(json_t *old_node, json_t *new_node, json_t *root)
{
	json_t *parent = NULL;
	const char *key = NULL;
	int res_find;
	int res_set;

	CHECK_POINTER(old_node, -EFAULT);
	CHECK_POINTER(new_node, -EFAULT);
	CHECK_POINTER(root, -EFAULT);

	res_find = find_parent_and_key(root, old_node, &parent, &key);
	if (res_find) { return res_find; }

	if (json_is_object(parent)) {
		res_set = json_object_set(parent, key, new_node);	
	}

	if (res_set) { return -EINVAL; }
	return 0;
}

json_t *normalize_json(json_t *root, int is_root)
{
	json_t *obj = NULL;
	json_t *value = NULL;
	const char *key = NULL;
	char *transform_key;
	json_t *json_copy_ret = NULL;
	json_t *converted_val = NULL;
	size_t i;
	
	CHECK_POINTER(root, NULL);
	obj = json_object();
	CHECK_POINTER(obj, NULL);

	if (json_is_object(root)) {
        json_object_foreach(root, key, value) {
            converted_val = normalize_json(value, 0);
            CHECK_POINTER(converted_val, NULL);
			if (strchr(key, '/')) {
				transform_key = replace_slash(key);
            	json_object_set_new(obj, transform_key, converted_val);
			}
			else {
            	json_object_set_new(obj, key, converted_val);
			}
        }
	}
	else if (json_is_array(root)) {
		value = NULL;
		converted_val = NULL;
        json_array_foreach(root, i, value) {
            char key[SHRT_SIZE];
            snprintf(key, sizeof(key), "%s%zu", SPECIAL_PREFIX, i);

            converted_val = normalize_json(value, 0);
            CHECK_POINTER(converted_val, NULL);
            json_object_set_new(obj, key, converted_val);
        }
	} 
	else {
		if (is_root) {
			json_copy_ret = json_copy(root);
			CHECK_POINTER(json_copy_ret, NULL);
			json_object_set_new(obj, SPECIAL_PREFIX"scalar", json_copy_ret);
		}
		else {
			obj = json_copy(root);
			CHECK_POINTER(obj, NULL);
		}
    }

	return obj;
}

json_t *denormalize_json(json_t *root, int is_root)
{
	json_t *root_dup = NULL;
    json_t *scal_value = NULL;
	json_t *node_with_slash[MID_SIZE];
	json_t *parent = NULL;
	json_t *node_dup = NULL;
	const char *key = NULL;
	char *transform_key = NULL;
	int res_find_parent;
	int prefix_found;
	int count;

	(void) is_root;

	if (!json_is_object(root)) { return NULL; }

	root_dup = json_deep_copy(root);
	if (!root_dup) { return NULL; }

	scal_value = json_object_get(root_dup, SPECIAL_PREFIX"scalar");
    if (scal_value) { return scal_value; }

	prefix_found = spec_prefix_is_present(root_dup);
	if (!prefix_found) { return root_dup; }

	count = find_keys_with_spec_slash(root_dup, node_with_slash, MID_SIZE, 0);
	for (int i = count - 1; i >=  0; i--) {
		res_find_parent = find_parent_and_key(root_dup, node_with_slash[i], &parent, &key);
		if (res_find_parent) { goto handle_error; }

		transform_key = reverse_replace_slash(key);
		node_dup = json_deep_copy(node_with_slash[i]);
		
		json_object_set_new(parent, transform_key, node_dup);
		json_object_del(parent, key);
	}

    return root_dup;
	handle_error:
		json_decref(root_dup);
		return NULL;
}

int find_parent_and_key(json_t *root, json_t *node, json_t **parent, 
					    const char **key)
{
	json_t *v = NULL;
	const char *k = NULL;
	int res_find;

	CHECK_POINTER(root, -EFAULT);
    CHECK_POINTER(node, -EFAULT);
    CHECK_POINTER(parent, -EFAULT);
    CHECK_POINTER(key, -EFAULT);

	if (json_equal(root, node)) { return -EINVAL; }

	if (json_is_object(root)) {
		json_object_foreach(root, k, v) {
			if (node == v) {
				*parent = root;
				*key = k;
				return 0;
			}

			res_find = find_parent_and_key(v, node, parent, key);
			if (!res_find) { return 0; }
		}
	}

	return -ENOENT;
}

int count_subdirs(json_t *obj)
{
	int count = 0;
	const char *key = NULL;
	json_t *value = NULL;

	if (!obj || !json_is_object(obj)) {
		return 0;
	}

	json_object_foreach(obj, key, value) {
		if (json_is_object(value)) {
			count++;
		}
	}

	return count;
}

int is_special_file(const char *path)
{
	if (strcmp("/.status", path) == 0 ||
		strcmp("/.save", path) == 0) {
			return 1;
	}
	return 0;
}

int separate_filepath(const char *path, char **parent_path, char **basename)
{
	char *path_dup = NULL;
	char *last_slash = NULL;

	CHECK_POINTER(path, -1);
	CHECK_POINTER(parent_path, -1);
	CHECK_POINTER(basename, -1);

	*parent_path = NULL;
	*basename = NULL;

	path_dup = strdup(path);
	CHECK_POINTER(path_dup, -1);

	last_slash = strrchr(path_dup, '/');

	if (last_slash) {
		*last_slash = '\0';

		*basename = strdup(last_slash + 1);
		if (!*basename) { goto handle_error; }

		if (path_dup[0] == '\0') {
			*parent_path = strdup("/");
			if (!*parent_path) { goto handle_error; }
		}
		else {
			*parent_path = path_dup;
			path_dup = NULL;
		}
	}
	else {
		*basename = path_dup;
		path_dup = NULL;
		if (!*basename) { goto handle_error; }

		*parent_path = strdup(".");
		if (!*parent_path) { goto handle_error; }
	}

	free(path_dup);
	return 0;

	handle_error:
		free(path_dup);
		free(*parent_path);
		free(*basename);
		return -1;
}

char *replace_slash(const char *key)
{
	char *key_dup = NULL;
	char *new_key = NULL;
	char *part_of_key = NULL;
	char *saveptr = NULL;
	size_t offset = 0;
	int count_slash = 0;
	int is_first_part = 1;

	CHECK_POINTER(key, NULL);

	key_dup = strdup(key);
	if (!key_dup) { goto handle_error; }

	for (int i = 0; i < strlen(key_dup); i++) {
		if (key_dup[i] == '/') { count_slash++; }
	}

	new_key = calloc(1, strlen(key) + strlen(SPECIAL_SLASH) * count_slash);
	if (!new_key) { goto handle_error; }

	part_of_key = strtok_r(key_dup, "/", &saveptr);
	while (part_of_key) {
		if (!is_first_part) {
            snprintf(new_key + offset, 
                     strlen(SPECIAL_SLASH) + 1,
                     "%s",
                     SPECIAL_SLASH);
            offset += strlen(SPECIAL_SLASH);
        }
        is_first_part = 0;

        snprintf(new_key + offset, 
                 strlen(part_of_key) + 1,
                 "%s",
                 part_of_key);
        offset += strlen(part_of_key);

        part_of_key = strtok_r(NULL, "/", &saveptr);
	}

	free(key_dup);
	return new_key;

	handle_error:
		free(key_dup);
		free(new_key);
		return NULL;
}

char *reverse_replace_slash(const char *key)
{
	char *key_dup = NULL;
    char *ptr = NULL;

	CHECK_POINTER(key, NULL);

	key_dup = strdup(key);
	if (!key_dup) { goto handle_error; }

	ptr = key_dup;
    while ((ptr = strstr(ptr, SPECIAL_SLASH)) != NULL) {
        *ptr = '/';
        memmove(ptr + 1, ptr + strlen(SPECIAL_SLASH), 
                strlen(ptr + strlen(SPECIAL_SLASH)) + 1);
        ptr++;
    }
   
    return key_dup;

	handle_error:
		free(key_dup);
		return NULL;
}

int spec_prefix_is_present(json_t *root)
{
	const char *key = NULL;
	json_t *value = NULL;
	int found = 0;

	json_object_foreach(root, key, value) {
		if (strstr(key, SPECIAL_PREFIX)) {
			return 1;
		}

		if (json_is_object(value)) { 
			found = spec_prefix_is_present(value); 
		}

		if (found) { return 1; }
	}

	return 0;
}

int find_keys_with_spec_slash(json_t *root, json_t **results, int max_results, int count)
{
    const char *key;
    json_t *value = NULL;
    
    json_object_foreach(root, key, value) {
        if (strstr(key, SPECIAL_SLASH)) {
            if (count < max_results) {
                results[count] = value;
				count++;
            }
        }
        
        if (json_is_object(value)) {
            count = find_keys_with_spec_slash(value, results, max_results, count);
        }
    }
    
    return count;
}
