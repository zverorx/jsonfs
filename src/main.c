/**
 * @file
 * @brief Entry point of the JSONFS.
 *
 * Processes command line parameters and calls fuse_main().
 */

#define FUSE_USE_VERSION 	35

/* Includes */
#include <jansson.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "jsonfs.h"

int main(int argc, char **argv)
{
	json_t *root = NULL;
	json_error_t js_error;
	const char *json_file = NULL;
	int ret;

	if (argc < 3) { 
		return EXIT_FAILURE; 
	}

	json_file = argv[1];

	root = json_load_file(json_file, 0, &js_error);
	CHECK_POINTER(root, EXIT_FAILURE);

	struct json_private_data *pd = init_private_data(root, json_file);
	if (!pd) {
		json_decref(root);
		return EXIT_FAILURE;
	}

	struct fuse_operations op = get_fuse_op();
	struct private_args args = get_fuse_args(argc, argv);

	ret = fuse_main(args.fuse_argc, args.fuse_argv, &op, pd);
	free(args.fuse_argv);
	return ret;
}
