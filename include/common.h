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
 * @brief Common types, macros, and private data for JSONFS.
 */

#ifndef COMMON_H_SENTRY
#define COMMON_H_SENTRY

#include <jansson.h>

/* ================================= */
/*              Enums                */
/* ================================= */

/**
 * @enum set_time
 * @see add_node_to_list_ft 
 */
enum set_time {
	SET_ATIME = 1,
	SET_MTIME = 2,
	SET_CTIME = 4
};

/* ================================= */
/*              Macros               */
/* ================================= */

/**
 * @def SHRT_SIZE
 * @brief Used as the size for buffers.
 */
#define SHRT_SIZE		32

/**
 * @def MID_SIZE
 * @brief Used as the size for buffers.
 * 
 * Is equal to two SHRT_SIZE.
 */
#define MID_SIZE		SHRT_SIZE * 2

/**
 * @def BIG_SIZE
 * @brief Used as the size for buffers.
 * 
 * Is equal to two MID_SIZE.
 */
#define BIG_SIZE		MID_SIZE * 2	

/**
 * @def SPECIAL_PREFIX
 * @brief Prefix for virtual files representing array elements and scalar values.
 * 
 * Used for files/directories that don't exist in original JSON:
 * - array indices: _$0, _$1, _$2
 * - scalar values: _$scalar
 */
#define SPECIAL_PREFIX	"_$"

/**
 * @def CHECK_POINTER
 * @brief Checks if a pointer is NULL.
 *
 * @param POINTER The pointer that needs to be checked.
 * @param ERROR Expression to return if the pointer is NULL.
 *
 * @warning ERROR must be compatible with the function's return type.
 */
#define CHECK_POINTER(POINTER, ERROR)	\
	do {								\
		if (!POINTER) {					\
			return ERROR;				\
		}								\
	} while(0)

/**
 * @def FILL_OR_RETURN 
 * @brief Adds a directory entry to readdir buffer. 
 * 
 * @param BUFF Buffer from fuse_fill_dir_t filler function.
 * @param NAME Name of the directory entry to add.
 *
 * @return Returns -ENOMEM if filler fails. 
 */
#define FILL_OR_RETURN(BUFF, NAME)								\
	do {														\
		if (filler(BUFF, NAME, NULL, 0, FUSE_FILL_DIR_PLUS)) {	\
			return -ENOMEM;										\
		}														\
	} while(0)

/* ================================= */
/*            Structures             */
/* ================================= */

/**
 * @struct jsonfs_private_data
 * @brief Private filesystem data. 
 * 
 * This structure is allocated in main() and passed to fuse_main(),
 * then made available via fuse_get_context()->private_data in all callbacks.
 */
struct jsonfs_private_data {
	json_t *root;				/**< Deserialized JSON document */
	char *path_to_json_file;	/**< Path to the source JSON file */
	struct file_time *ft;		/**< Head of the file times linked list */
	time_t mount_time;			/**< Filesystem mount time */
	uid_t uid;					/**< User ID */
	gid_t gid; 					/**< Group ID */
	int is_saved;				/**< Save state: 1=no unsaved changes, 0=has unsaved changes */	
};

/**
 * @struct file_time
 * @brief File time metadata structure.
 * @see add_node_to_list_ft
 * @see find_node_file_time 
 */
struct file_time {
    char *path;                 /**< File/directory path */
    time_t atime;               /**< Last access time */
    time_t mtime;               /**< Last modification time */
    time_t ctime;               /**< Last status change time */
    struct file_time *next_node;/**< Next node in singly-linked list */
};

/**
 * @struct private_args
 * @brief Arguments for fuse_main().
 */
struct private_args {
	char **fuse_argv;	/**< argv for fuse_main() */
	int fuse_argc;		/**< argc for fuse_main() */
};

#endif /* COMMON_H_SENTRY */
