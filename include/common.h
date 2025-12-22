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
 * @brief Common macros for JSONFS.
 */

#ifndef COMMON_H_SENTRY
#define COMMON_H_SENTRY

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
 * - array indices: @0, @1, @2
 * - scalar values: @scalar
 */
#define SPECIAL_PREFIX	"@"

/**
 * @def SPECIAL_PREFIX_SLASH
 * @brief A special representation of the "/" symbol if it is in the key name.
 * 
 * Example:
 * "key/key" in JSON file
 * key@2Fkey as a file
 */
#define SPECIAL_SLASH	SPECIAL_PREFIX"2F"

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
 * @param BUFF The buffer passed to the readdir() operation.
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

#endif /* COMMON_H_SENTRY */
