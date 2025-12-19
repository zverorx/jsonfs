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
 * @brief Contains declarations of functions for working with deserialized JSON file.
 */

#ifndef JSON_OPERATIONS_H_SENTRY
#define JSON_OPERATIONS_H_SENTRY

#include <jansson.h>

/**
 * @brief Finds a JSON node by its absolute path in the tree.
 *
 * Traverses the JSON object hierarchy starting from the given root,
 * following the path components separated by '/'. Returns the node
 * at the specified path, or NULL if the path is invalid or any
 * intermediate component is not a JSON object.
 *
 * @param path Absolute path (e.g., "/foo/bar"), must not be NULL.
 * @param root Root JSON object to start traversal from, must not be NULL.
 * @return Pointer to the found JSON node, or NULL on failure.
 */
json_t *find_json_node(const char *path, json_t *root);

/**
 * @brief Replace a JSON node with new node in the tree.
 * 
 * Finds parent of old_node and replaces it with new_node.
 * Works for objects only.
 * 
 * @param old_node Node to be replaced (must exist in tree).
 * @param new_node New node to insert.
 * @param root Root of JSON tree to search in.
 * @return 0 on success, negative error code on failure.
 */

int replace_json_nodes(json_t *old_node, json_t *new_node, json_t *root);

/**
 * @brief Converts JSON to object-only representation for filesystem.
 * @param root JSON value (object, array, or scalar).
 * @param is_root Flag indicating if this is the root level (1) or nested (0).
 * @return New independent JSON object:
 *         - Arrays become {"@0":..., "@1":...} with underscore-prefixed keys.
 *         - Scalars at root become {"@scalar": value}.
 * @note SPECIAL_PREFIX provide unambiguous identification of
 *       converted arrays (@0, @1...) and root scalars (@scalar).
 *       Caller must json_decref() the result.
 */
json_t *normalize_json(json_t *root, int is_root);

/**
 * @brief Converts JSON from object-only representation to diverse.
 * @param root JSON value (result of normalize_json).
 * @param is_root Flag indicating if this is the root level (1) or nested (0).
 * @return New JSON value in standard polymorphic form:
 *         - Objects with sequential "@" keys become arrays.
 *         - Objects with "@scalar" key become scalar.
 * @note Caller must json_decref() the result.
 * @see normalize_json
 */
json_t *denormalize_json(json_t *root, int is_root);

/**
 * @brief Find parent and key for given JSON node.
 * 
 * Recursively searches JSON tree for node's parent.
 * Searches only for objects, not arrays.
 * 
 * @param root Root node to start search from.
 * @param node Node to find parent and key for. 
 * @param parent[out] Found parent node.
 * @param key[out] Node's key in parent object.
 * @return 0 on success, -ENOENT if not found, -EINVAL if node is root.
 */
int find_parent_and_key(json_t *root, json_t *node, json_t **parent, 
					    const char **key);

/**
 * @brief Counts immediate subdirectories in a JSON directory node.
 *        A subdirectory is a direct child JSON object.
 * @param obj JSON object representing a directory (must be non-NULL).
 * @return Number of direct child objects (subdirectories).
 */
int count_subdirs(json_t *obj);

/**
 * @brief Checks if the given path corresponds to a special service file.
 * 
 * Special files are virtual files used for filesystem control operations:
 * - /.status - shows filesystem status.
 * - /.save - triggers saving changes.
 * 
 * @param path File path to check (must be absolute path starting with '/').
 * @return 1 if the path is a special file, 0 otherwise.
 * 
 * @see getattr_special_file()
 */
int is_special_file(const char *path);

/**
 * @brief Separate file path into parent directory and basename.
 * 
 * @param path File path to for separation.
 * @param parent_path[out] Pointer to parent directory path.
 *                         Returns "/" for root, "." for current directory,
 *                         or parent path without trailing slash.
 * @param basename[out] Name of file/directory without parent path.
 * 						Returns empty string for root path "/",
 * 
 * @return 0 on success, -1 on failure.
 */
int separate_filepath(const char *path, char **parent_path, char **basename);

/**
 * @brief Replaces the '/' character in the key with SPECIAL_SLASH.
 * @param key JSON key.
 * @return New key. The caller must be free.
 * @see SPECIAL_SLASH
 * @see normalize_json
 */
char *replace_slash(const char *key);

/**
 * @brief Replaces the SPECIAL_SLASH in the key with '/'.
 * @param key JSON key.
 * @return New key. The caller must be free.
 * @see SPECIAL_SLASH
 * @see denormalize_json
 */
char *reverse_replace_slash(const char *key);

/**
 * @brief Checking for keys containing "@".
 * @param root Root node to start search from.
 * @return 1 if there is, 0 otherwise.
 */
int spec_prefix_is_present(json_t *root);

/**
 * @brief Search for keys containing SPECIAL_SLASH./
 * @param root Root node to start search from.
 * @param results Storage of links to found nodes in root.
 * @param max_results Size of the results.
 * @param count The number of nodes found in root, an incremental value, is 0 when called.
 * @return Increment of the count parameter, the number of nodes found.
 */
int find_keys_with_spec_slash(json_t *root, json_t **results, int max_results, int count);

#endif /* JSON_OPERATIONS_H_SENTRY */