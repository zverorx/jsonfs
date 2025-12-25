#!/bin/bash
# Compiling and mounting jsonfs with FUSE flags.

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$test_dir/ex_obj.json"
mount_point="$test_dir/mnt"
options=$1

if [ "$options" != "-f" ] && [ "$options" != "-d" ] ; then
    echo "Usage: $0 [-f|-d]"
    exit 1
fi

mkdir "$mount_point"
cd "$test_dir"/..
make BUILD=debug
"$exec_file" "$json_file" "$mount_point" "$options"
