# Test
The script test.sh mounts the JSON file located in the test/.
The script also creates a mount point in it, and then deletes it after unmounting.
Inside, the commands `ls  -lRa` and `cat` are executed to display the attributes and contents of the files.

---

## Usage
```
./test.sh [ JSON file ]
```
You can pass a JSON file to the parameters, if you do not do this, it will mount the default file from test/.
It can be called from anywhere.
>WARNING: You must compile jsonfs before using it (see README.md at the root of the project).

## Example

### Script execution
```
**************************************************
/home/user/prog/jsonfs/test/example.json:
{
  "str": "hello",
  "int": 42,
  "float": 3.14,
  "bool": true,
  "nil": null,
  "arr": [1, "x", false],
  "obj": {
    "key": "value",
	"empty_obj": { 
	},
	"nested_obj": {
		"empty_str": ""
	}
  }
}
**************************************************

**************************************************
.:
total 4
dr-xr-xr-x 3 user user    0 Oct 15 19:17 .
drwxr-xr-x 3 user user 4096 Oct 15 19:17 ..
-r--r--r-- 1 user user   15 Oct 15 19:17 arr
-r--r--r-- 1 user user    4 Oct 15 19:17 bool
-r--r--r-- 1 user user    4 Oct 15 19:17 float
-r--r--r-- 1 user user    2 Oct 15 19:17 int
-r--r--r-- 1 user user    4 Oct 15 19:17 nil
dr-xr-xr-x 4 user user    0 Oct 15 19:17 obj
-r--r--r-- 1 user user    7 Oct 15 19:17 str

./obj:
total 0
dr-xr-xr-x 4 user user 0 Oct 15 19:17 .
dr-xr-xr-x 3 user user 0 Oct 15 19:17 ..
dr-xr-xr-x 2 user user 0 Oct 15 19:17 empty_obj
-r--r--r-- 1 user user 7 Oct 15 19:17 key
dr-xr-xr-x 2 user user 0 Oct 15 19:17 nested_obj

./obj/empty_obj:
total 0
dr-xr-xr-x 2 user user 0 Oct 15 19:17 .
dr-xr-xr-x 4 user user 0 Oct 15 19:17 ..

./obj/nested_obj:
total 0
dr-xr-xr-x 2 user user 0 Oct 15 19:17 .
dr-xr-xr-x 4 user user 0 Oct 15 19:17 ..
-r--r--r-- 1 user user 2 Oct 15 19:17 empty_str
**************************************************

**************************************************
./str: "hello"
./int: 42
./float: 3.14
./bool: true
./nil: null
./arr: [1, "x", false]
./obj/key: "value"
./obj/nested_obj/empty_str: ""
**************************************************
```

### Explanation

- The first block (red in the terminal) shows the contents of the example.json file.

- The second block (green in the terminal) shows the result of the `ls -lRa` command.

- The third block (blue in the terminal) shows the result of `cat`,
where the files from the `find` command output are passed as parameters.

### Examples of JSON files

The test directory also contains examples of JSON files:
* ex_obj.json - file with an object at its root (used by default for the test).
* ex_arr.json - file with an array of objects at its root.
* ex_scal.json - file with a scalar value at its root.
