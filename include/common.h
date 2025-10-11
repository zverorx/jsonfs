/**
 * @file
 * @brief Common types, macros, and private data for JSONFS.
 */

#ifndef COMMON_H_SENTRY
#define COMMON_H_SENTRY

#include <jansson.h>

/* ================================= */
/*              Macros               */
/* ================================= */

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

/* ================================= */
/*            Structures             */
/* ================================= */

/**
 * @struct json_private_data
 * @brief Private data structure passed to FUSE callbacks.
 */
struct json_private_data {
	json_t *root;				/**< Root of the parsed JSON doc */
	char *path_to_json_file;	/**< Path to the source JSON file */
};

#endif /* COMMON_H_SENTRY */
