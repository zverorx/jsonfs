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
 * @brief JSONFS filesystem logic interface.
 *
 * Contains declarations of functions from jsonfs.c and specifications for them.
 */

#ifndef JSONFS_H_SENTRY
#define JSONFS_H_SENTRY

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
json_t *find_node_by_path(const char *path, json_t *root);

/**
 * @brief Creates and initializes a jsonfs_private_data structure.
 *
 * Allocates and initializes all structure fields.
 * 
 * @param json_root JSON root object (must not be NULL).
 * @param path Path to the JSON source file (must be a valid C string).
 *
 * @return Pointer to a new jsonfs_private_data instance on success, or NULL on failure.
 */
struct jsonfs_private_data *init_private_data(json_t *json_root, const char *path);

/**
 * @brief Gives the fuse_operations structure.
 * @return Struct fuse_operations with implemented callbacks.
 */
struct fuse_operations get_fuse_op(void);

/**
 * @brief  Prepares arguments for fuse_main().
 * @param argc Argument count from main().
 * @param argv Argument vector from main().
 * @return A struct with adjusted argc/argv; caller must free fuse_argv.
 */
struct private_args get_fuse_args(int argc, char **argv);

/**
 * @brief Counts immediate subdirectories in a JSON directory node.
 *        A subdirectory is a direct child JSON object.
 * @param obj JSON object representing a directory (must be non-NULL).
 * @return Number of direct child objects (subdirectories).
 */
int count_subdirs(json_t *obj);

/**
 * @brief Converts JSON to object-only representation for filesystem.
 * @param root JSON value (object, array, or scalar).
 * @param is_root Flag indicating if this is the root level (1) or nested (0).
 * @return New independent JSON object:
 *         - Arrays become {"_$0":..., "_$1":...} with underscore-prefixed keys.
 *         - Scalars at root become {"_$scalar": value}.
 * @note Underscore-dollars prefixes provide unambiguous identification of
 *       converted arrays (_$0, _$1...) and root scalars (_$scalar).
 *       Caller must json_decref() the result.
 */
json_t *convert_to_obj(json_t *root, int is_root);

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
 * 
 */
int replace_json_value(const char *path, const char *buffer, size_t size,
					   off_t offset, struct jsonfs_private_data *pd);

/**
 * 
 */
int replace_nodes(json_t *old_node, json_t *new_node, 
				  struct jsonfs_private_data *pd);

/**
 * 
 */				  
int find_parent_key_index(json_t *root, json_t *node, json_t **parent, 
						  const char **key);

#endif /* JSONFS_H_SENTRY */
