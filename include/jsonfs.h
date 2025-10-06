/**
 * @file jsonfs.h
 */

#ifndef JSONFS_H_SENTRY
#define JSONFS_H_SENTRY

/**
 *
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
 * @return struct fuse_operations with implemented callbacks.
 */
struct fuse_operations jsonfs_get_fuse_op(void);

#endif /* JSONFS_H_SENTRY */
