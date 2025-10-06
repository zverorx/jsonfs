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
#include "jsonfs.h"

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
