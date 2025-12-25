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
 * @brief Converts JSON to object-only representation for filesystem.
 * 
 * @param root JSON value (can be obtained using json_load_file).
 * @param is_root Flag indicating if this is the root level (1) or nested (0).
 * 				  It is assumed that the caller will pass the value 1.
 * 
 * @return New independent JSON object:
 * 		   - Arrays become {"@0":..., "@1":...} with a SPECIAL_PREFIX in the keys.
 *         - Scalars at root become {"@scalar": ...}.
 * 		   - The "/" characters in the keys are replaced with SPECIAL_SLASH.
 * 
 * @note SPECIAL_PREFIX provide unambiguous identification of
 *       converted arrays (@0, @1...) and root scalars (@scalar).
 *       Caller must json_decref() the result.
 * 
 * @see denormalize_json
 */
json_t *normalize_json(json_t *root, int is_root);

/**
 * @brief Converts JSON from object-only representation to diverse.
 * 
 * @param root JSON value (result of normalize_json).
 * @return New JSON value in standard polymorphic form:
 * 		   - Objects whose child element keys start with SPECIAL_PREFIX become arrays.
 * 		   - The value with the @scalar key at the root becomes a scalar.
 *		   - The SPECIAL_SLASH in the keys is replaced by the "/" character.
 * 
 * @note Caller must json_decref() the result.
 * @note Used with normalized JSON.
 *  
 * @see normalize_json
 */
json_t *denormalize_json(json_t *root);

/**
 * @brief Finds a JSON node by its absolute path in the filesystem.
 *
 * Traverses the JSON object hierarchy starting from the given root,
 * following the path components separated by '/'. Returns the node
 * at the specified path, or NULL if the path is invalid or any
 * intermediate component is not a JSON object.
 *
 * @param path Absolute path, must not be NULL.
 * @param root Root JSON object to start traversal from, must not be NULL.
 * 
 * @return Pointer to the found JSON node, or NULL on failure.
 * 
 * @note Used with normalized JSON.
 */
json_t *find_json_node(const char *path, json_t *root);

/**
 * @brief Find parent and key for given JSON value.
 * 
 * @param root Root node to start search from.
 * @param node Node to find parent and key for. 
 * @param parent[out] Found parent node.
 * @param key[out] Node's key in parent object.
 * 
 * @return 0 on success, -ENOENT if not found, -EINVAL if node is root.
 * 
 * @note Used with normalized JSON.
 */
int find_parent_and_key(json_t *root, json_t *node, json_t **parent, 
					    const char **key);

/**
 * @brief Checking for keys containing SPECIAL_PREFIX.
 * 
 * @param root Root node to start search from.
 * 
 * @return 1 if there is, 0 otherwise.
 * 
 * @note Does not take into account SPECIAL_SLASH.
 * @note Used with normalized JSON.
 */
int spec_prefix_is_present(json_t *root);

/**
 * @brief Search for keys containing SPECIAL_SLASH.
 * 
 * @param root Root node to start search from.
 * @param results Storage of links to found nodes in root.
 * @param max_results Size of the results.
 * @param count The number of nodes found in root, an incremental value, is 0 when called.
 * 
 * @return Increment of the count parameter, the number of nodes found.
 * 
 * @note Used with normalized JSON.
 */
int find_keys_with_spec_slash(json_t *root, json_t **results, int max_results, int count);

/**
 * @brief Search for keys containing SPECIAL_PREFIX at the beginning.
 *
 * Such keys are perceived as elements of an array.
 * Their parent is written to the array list, the results parameter. 
 *  
 * @param root Root node to start search from.
 * @param results Storage of links to found nodes in root.
 * @param max_results Size of the results.
 * @param count The number of nodes found in root, an incremental value, is 0 when called.
 * 
 * @return Increment of the count parameter, the number of nodes found.
 * 
 * @note Used with normalized JSON.
 */
int find_array_in_normal_root(json_t *root, json_t **results, int max_results, int count);

/**
 * @brief Replace with a new node in deserialized JSON. 
 * 
 * Finds parent of old_node and replaces it with new_node.
 * Works for objects only.
 * 
 * @param old_node Node to be replaced.
 * @param new_node New node to insert (in case of an error, it requires json_decref()).
 * @param root Root of deserialized JSON to search in.
 * 
 * @return 0 on success, negative error code on failure.
 * 
 * @note Used with normalized JSON.
 */
int replace_json_nodes(json_t *old_node, json_t *new_node, json_t *root);

/**
 * @brief Counts immediate subdirectories in a JSON directory.
 *        A subdirectory is a direct child JSON object.
 * 
 * @param obj JSON object representing a directory (must be non-NULL).
 * 
 * @return Number of direct child objects (subdirectories).
 * 
 * @note Used with normalized JSON.
 */
int count_subdirs(json_t *obj);

/**
 * @brief Checks if the given path corresponds to a special service file.
 * 
 * Special files are virtual files used for filesystem control operations:
 * - /.status - shows filesystem status (SAVED or UNSAVED).
 * - /.save - triggers saving changes (writing to a file causes saving).
 * 
 * @param path The absolute file path to check.
 * 
 * @return 1 if the path is a special file, 0 otherwise.
 */
int is_special_file(const char *path);

/**
 * @brief Replaces the "/" character in the key with SPECIAL_SLASH.
 * 
 * @param key JSON key with "/".
 * 
 * @return New key with SPWCIAL_SLASH. The caller must be free.
 * 
 * @see SPECIAL_SLASH
 * @see normalize_json
 */
char *replace_slash(const char *key);

/**
 * @brief Replaces the SPECIAL_SLASH in the key with "/".
 * 
 * @param key JSON key with SPECIAL_SLASH.
 * 
 * @return New key with "/". The caller must be free.
 * 
 * @see SPECIAL_SLASH
 * @see denormalize_json
 */
char *reverse_replace_slash(const char *key);

/**
 * @brief Separate file path into parent directory and basename.
 * 
 * @param path The absolute file path to for separation.
 * @param parent_path[out] Parent directory path.
 *                         Returns "/" for root, "." for current directory,
 *                         or parent path without trailing slash. 
 * @param basename[out] Name of file/directory without parent path.
 * 						Returns empty string for root path "/",
 * 
 * @return 0 on success, -1 on failure.
 */
int separate_filepath(const char *path, char **parent_path, char **basename);

#endif /* JSON_OPERATIONS_H_SENTRY */
