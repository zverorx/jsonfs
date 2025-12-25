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
 * @brief Structures and functions for getting arguments to fuse_main().
 */

 #ifndef JSONFS_H_SENTRY
 #define JSONFS_H_SENTRY

/* ================================= */
/*             Structures            */
/* ================================= */

/**
 * @struct jsonfs_private_data
 * @brief Private filesystem data. 
 * 
 * This structure is allocated in main() and passed to fuse_main(),
 * then made available via fuse_get_context()->private_data in all callbacks.
 * 
 * @see init_private_data
 * @see destroy_private_data
 */
struct jsonfs_private_data {
	json_t *root;				/**< Deserialized JSON document */
	char *path_to_json_file;	/**< Absolute path to the source JSON file */
	struct file_time *ft;		/**< Head of the file times linked list */
	time_t mount_time;			/**< Filesystem mount time */
	uid_t uid;					/**< User ID */
	gid_t gid; 					/**< Group ID */
	int is_saved;				/**< Save state: 1=no unsaved changes, 0=has unsaved changes */	
};

/**
 * @struct private_args
 * @brief Arguments for fuse_main().
 * 
 * @see get_fuse_args
 */
struct private_args {
	char **fuse_argv;	/**< argv for fuse_main() */
	int fuse_argc;		/**< argc for fuse_main() */
};

/* ================================= */
/*            Declarations           */
/* ================================= */

/**
 * @brief Gives the fuse_operations structure.
 * @return Struct fuse_operations with implemented callbacks.
 */
struct fuse_operations get_fuse_op(void);

/**
 * @brief  Prepares arguments for fuse_main().
 * 
 * @param argc Argument count from main().
 * @param argv Argument vector from main().
 * 
 * @return A struct with adjusted argc/argv; caller must free fuse_argv.
 * 
 * @note If no memory has been allocated inside for fuse_argv, 
 * 		 exit(EXIT_FAILURE) is executed.
 */
struct private_args get_fuse_args(int argc, char **argv);

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
 * @brief Destroys a jsonfs_private_data structure.
 * @param pd Pointer to the jsonfs_private_data structure to destroy.
 */
void destroy_private_data(struct jsonfs_private_data *pd);

#endif /* JSONFS_H_SENTRY */
