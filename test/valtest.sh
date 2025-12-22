#!/bin/bash
# Testing via valgrind.

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$test_dir/ex_obj.json"
mount_point="$test_dir/mnt"

mkdir "$mount_point"
cd "$test_dir"/..
make BUILD=debug

valgrind --quiet            \
         --verbose          \
         --leak-check=full  \
         "$exec_file" "$json_file" "$mount_point"


########## TEST ##########

cd "$mount_point"
echo -n '123' | cat > int
echo -n '4' | cat >> int
echo -n 'П' | dd of="строка" bs=1 seek=1 conv=notrunc
cat float > /dev/null
mv bool bol
mv bol arr/
rm arr/bol
rm -r arr/
touch file

########## TEST ##########

cd "$test_dir"/..
sleep 1
fusermount3 -u "$mount_point"