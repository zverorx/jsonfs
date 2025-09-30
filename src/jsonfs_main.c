/**
 * @file jsonfs_main.c
 */

#define FUSE_USE_VERSION 	35
/* XXX */
#define JSONFILE			argv[1]

/* Includes */
#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "fuse_callbacks.h"

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
static struct json_private_data *init_private_data(json_t *json_root, const char *path);

int main(int argc, char **argv)
{
	json_t *root;
	json_error_t js_error;
	int ret;

	/* TODO: Decide how to handle input, pay attention to the -o option */
	if (argc != 3) { 
		return EXIT_FAILURE; 
	}

	root = json_load_file(JSONFILE, 0, &js_error);
	CHECK_POINTER(root, EXIT_FAILURE);

	struct json_private_data *pd = init_private_data(root, JSONFILE);
	if (!pd) {
		json_decref(root);
		return EXIT_FAILURE;
	}

	struct fuse_operations op = jsonfs_get_fuse_op();

	/* XXX */
	char *fuse_argv[] = { argv[0], argv[2] };
	struct fuse_args args = FUSE_ARGS_INIT(2, fuse_argv);

	ret = fuse_main(args.argc, args.argv, &op, pd);

	return ret;
}

static struct json_private_data *init_private_data(json_t *json_root, const char *path)
{
	CHECK_POINTER(json_root, NULL);
	CHECK_POINTER(path, NULL);

	struct json_private_data *pd = calloc(1, sizeof(struct json_private_data));
	CHECK_POINTER(pd, NULL);

	pd->root = json_root;
	pd->path_to_json_file = strdup(path);
	if (!pd->path_to_json_file) {
		free(pd);
		return NULL;
	}

	return pd;
}
