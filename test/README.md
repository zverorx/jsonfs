# Test
The test.sh script mounts example.json located in the root of the test directory. The script also creates a mount point within it and then deletes it after unmounting. Inside, the 'ls -l' and 'cat' commands are executed to display the file's attributes and contents.

---

## Prerequisites
You must compile jsonfs (see README.md in the root of the project).
The script must also be given execution rights using the following command:
```
chmod u+x test.sh
```
> WARNING: The test must be run from the test directory, since the script contains relative paths.

## Usage
```
./test.sh
```
The script takes no arguments and uses hardcoded paths.

## Example
Script execution:
```
**************************************************
example.json:
{
  "str": "hello",
  "int": 42,
  "float": 3.14,
  "bool": true,
  "nil": null,
  "arr": [1, "x", false],
  "obj": {
    "key": "value"
  }
}
**************************************************

**************************************************
.:
total 0
-r--r--r-- 1 user user 15 Oct 12 14:45 arr
-r--r--r-- 1 user user  4 Oct 12 14:45 bool
-r--r--r-- 1 user user 18 Oct 12 14:45 float
-r--r--r-- 1 user user  2 Oct 12 14:45 int
-r--r--r-- 1 user user  4 Oct 12 14:45 nil
dr-xr-xr-x 2 user user  0 Oct 12 14:45 obj
-r--r--r-- 1 user user  7 Oct 12 14:45 str

./obj:
total 0
-r--r--r-- 1 user user 7 Oct 12 14:45 key
**************************************************

**************************************************
./str: "hello"
./int: 42
./float: 3.1400000000000001
./bool: true
./nil: null
./arr: [1, "x", false]
./obj/key: "value"
**************************************************
```

The first block (red in the terminal) shows the contents of the example.json file.

The second block (green in the terminal) shows the result of the 'ls -l' command.

The third block (blue in the terminal) shows the result of 'cat',
where the files from the 'find' command output are passed as parameters.
