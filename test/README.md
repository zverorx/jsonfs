# Test

`test.sh` automatically checks JSONFS by:

- Mounting the JSON file
- Running `ls -Rla` and `cat` to check the structure and content  
- Cleaning the mount point after testing
- Supports color-coded output parameters
---

## Usage
```
./test.sh <json_file> [rgb] 
```
Options:
- r - output of the first block (red), needed to demonstrate the contents of the original JSON file
- g - output of the second block (green), needed to demonstrate the structure of the FS
- b - output of the third block (blue), needed to demonstrate the contents of the FS files

>NOTE: You must compile jsonfs before using it (see README.md at the root of the project).

## Example

### Script execution
```
**************************************************
/home/user/example.json:
{
  "user": "Ivan Petrov", 
  "interests": ["photography", "cooking", "skiing"],
  "phone": null,
  "profile": {
    "city": "Saint Petersburg",
    "street": "Nevsky Prospect",
    "apartment": 45
  }
}
**************************************************

**************************************************
.:
total 4
dr-xr-xr-x 4 user user    0 Oct 26 00:52 .
drwxr-xr-x 3 user user 4096 Oct 26 00:52 ..
dr-xr-xr-x 2 user user    0 Oct 26 00:52 interests
-r--r--r-- 1 user user    4 Oct 26 00:52 phone
dr-xr-xr-x 2 user user    0 Oct 26 00:52 profile
-r--r--r-- 1 user user   13 Oct 26 00:52 user

./interests:
total 0
dr-xr-xr-x 2 user user  0 Oct 26 00:52  .
dr-xr-xr-x 4 user user  0 Oct 26 00:52  ..
-r--r--r-- 1 user user 13 Oct 26 00:52 '_$0'
-r--r--r-- 1 user user  9 Oct 26 00:52 '_$1'
-r--r--r-- 1 user user  8 Oct 26 00:52 '_$2'

./profile:
total 0
dr-xr-xr-x 2 user user  0 Oct 26 00:52 .
dr-xr-xr-x 4 user user  0 Oct 26 00:52 ..
-r--r--r-- 1 user user  2 Oct 26 00:52 apartment
-r--r--r-- 1 user user 18 Oct 26 00:52 city
-r--r--r-- 1 user user 17 Oct 26 00:52 street
**************************************************

**************************************************
./user: "Ivan Petrov"
./interests/_$0: "photography"
./interests/_$1: "cooking"
./interests/_$2: "skiing"
./phone: null
./profile/city: "Saint Petersburg"
./profile/street: "Nevsky Prospect"
./profile/apartment: 45
**************************************************

```

### Explanation

- The first block (red in the terminal) shows the contents of the example.json file.

- The second block (green in the terminal) shows the result of the `ls -lRa` command.

- The third block (blue in the terminal) shows the result of `cat`,
where the files from the `find` command output are passed as parameters.

### Examples of JSON files

The test/ directory contains examples of JSON files:
* ex_obj.json - file with an object at its root.
* ex_arr.json - file with an array of objects at its root.
* ex_scal.json - file with a scalar value at its root.
