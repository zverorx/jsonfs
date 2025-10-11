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

#endif /* JSONFS_H_SENTRY */
