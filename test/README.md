# Test

A set of scripts for testing jsonfs.

* `test_a.sh` - checking attributes,
* `test_r.sh` - checking the read operation,
* `test_w.sh` - checking the write operation,
* `valtest.sh` - checking for memory leaks,
* `fastmnt.sh` - fast mounting.

The general principle of testing:

* mounting the JSON file,
* executing commands,
* unmounting.

---

## Usage

```
./test_a.sh <json_file>
```

```
./test_r.sh <json_file>
```

```
./test_w.sh
```

```
./valtest.sh
```

```
./fastmnt.sh [-f|-d]
```

> NOTE: You must compile jsonfs before using it (see README.md at the root of the project).
> For test_w.sh, valtest.sh and fastmnt.sh the test/ directory must contain an unchanged ex_obj.json file.

## Examples of JSON files

The test/ directory contains examples of JSON files:

* ex_obj.json - file with an object at its root.
* ex_arr.json - file with an array of objects at its root.
* ex_scal.json - file with a scalar value at its root.
