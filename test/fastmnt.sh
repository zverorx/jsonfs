#!/bin/bash
# Compiling and mounting jsonfs with FUSE flag -f.

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$test_dir/ex_obj.json"
mount_point="$test_dir/mnt"

mkdir "$mount_point"
cd "$test_dir"/..
make BUILD=debug
"$exec_file" "$json_file" "$mount_point" -f
