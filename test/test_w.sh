#!/bin/bash

# This script is designed for testing jsonfs.
# Runs jsonfs in foreground mode for testing.
# FUSE debug output is displayed after test completion.

set -e

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$test_dir/ex_obj.json"
mount_point="$test_dir/mnt"
log_file="$test_dir/log.txt"

if [ ! -f "$exec_file" ] ; then
    echo "Error: not found $exec_file" >&2
    exit 1
fi

if [ ! -f "$json_file" ] ; then
    echo "Error: JSON file not found" >&2
    exit 1
fi

########## Mounting ##########

mkdir -p "$mount_point"
touch "$log_file"

"$exec_file" "$json_file" "$mount_point" -f > "$log_file" 2>&1 &
sleep 1

if ! mountpoint -q "$mount_point" 
then
    rmdir "$mount_point"
    echo "Error: mount failure" >&2
    exit 1
    else cd "$mount_point"
fi

trap 'cd $test_dir ; sync ;                   \
     fusermount3 -u $mount_point &>/dev/null ;\
     echo ;                                   \
     echo "=== FUSE operations log ===" ;     \
     cat "$log_file" ;                        \
     rmdir $mount_point ;                     \
     rm "$log_file"' ERR EXIT

########## TEST 1 ##########

echo -n "msg: string file before: "
cat string
echo

echo -n "H" | dd of="$mount_point/string" bs=1 seek=1 conv=notrunc

echo -n "msg: string file after: "
cat string
echo

########## TEST 2 ##########

echo -n "msg: float file before: "
cat float
echo

echo -n "2.98" | cat > "$mount_point/float"

echo -n "msg: float file after: "
cat float
echo

exit 0
