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
 * @param path  Absolute path (e.g., "/foo/bar"), must not be NULL.
 * @param root  Root JSON object to start traversal from, must not be NULL.
 * @return      Pointer to the found JSON node, or NULL on failure.
 */
json_t *find_node_by_path(const char *path, json_t *root);

/**
 * @brief Creates and initializes a json_private_data structure.
 *
 * Allocates memory for the structure, assigns the JSON root object,
 * and duplicates the given file path.
 * 
 * @param json_root JSON root object (must not be NULL).
 * @param path 		Path to the JSON source file (must be a valid C string).
 *
 * @return Pointer to a new json_private_data instance on success, or NULL on failure.
 */
struct json_private_data *init_private_data(json_t *json_root, const char *path);

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
 * @brief Counts subdirectories in a JSON directory node.
 *        A subdirectory is a child JSON object.
 * @param obj JSON object representing a directory (must be non-NULL).
 * @return Number of child objects (subdirectories).
 */
int count_subdirs(json_t *obj);

#endif /* JSONFS_H_SENTRY */
