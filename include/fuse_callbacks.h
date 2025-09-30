#ifndef FUSE_CALLBACKS_H_SENTRY
#define FUSE_CALLBACKS_H_SENTRY

/**
 * @brief Gives the fuse_operations structure.
 * @return struct fuse_operations with implemented callbacks.
 */
struct fuse_operations jsonfs_get_fuse_op(void);

#endif
