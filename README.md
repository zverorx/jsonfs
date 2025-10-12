# jsonfs
A file system that allows you to organize a json file as a tree of directories and files.
>The project is under development.
>The file system is currently read-only.

---

## Prerequisites
Check for the following:
* gcc
* make
* libjansson-dev
* libfuse3-dev

To install, use your package manager.

## Compilation
Go to the source directory, and run:
```
make
```

To find out more about Makefile features, run:
```
make help
```
>WARNING: The install and uninstall targets don't work yet.

## Usage
```
./jsonfs <json_file> <mount_point> [ FUSE options... ]
```
## Example
If the original JSON file looks like this:
```
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
```
then the mount point will contain the following files and directories:
 ```
 .
├── arr
├── bool
├── float
├── int
├── nil
├── obj
│   └── key
└── str
 ```
Each file contains the string representation of its JSON value (e.g. cat tmp/int -> 42) 
To check functionality, you can use a test script. To do this, review the README.md file in the test directory at the project root.

## License
This project is licensed under the GPLv3 license. See the LICENSE file for more details.

## Feedback
To contact the developer, you can email zveror1806@gmail.com
