# jsonfs
A file system that represents a JSON file as a directory tree,
where JSON objects become directories, and all other JSON types become
files containing the string representation of their values.

>The file system is read-only.

---

## Prerequisites

Check for the following programs:

### Required:
* `gcc`
* `make`
* `libjansson-dev`
* `libfuse3-dev`
>P.S. To install libjansson-dev and libfuse3-dev, use your system's package manager.

### Optional:

- `curl` and `unzip` — if you plan to download the archive.
- `git` — if you prefer to clone the repository.


## Download
You can obtain the source code in one of two ways:

### From the archive:
```
curl -L -o jsonfs-v1.1.0.zip https://github.com/zverorx/jsonfs/archive/refs/tags/v1.1.0.zip
```
```
unzip jsonfs-v1.1.0.zip
```

### From the Git repository (latest development version):
```
git clone https://github.com/zverorx/jsonfs.git
```

## Compilation and installation
Go to the source directory, and run:
```
make && sudo make install
```

To find out more about Makefile features, run:
```
make help
```


## Usage

Mounting:
```
jsonfs <json_file> <mount_point> [ FUSE options... ]
```

Unmounting:
```
fusermount3 -u <mount_point>
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
    "key": "value",
	"empty_obj": { 
	},
	"nested_obj": {
		"empty_str": ""
	}
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
│   ├── empty_obj
│   ├── key
│   └── nested_obj
│       └── empty_str
└── str

 ```
Each file contains the string representation of its JSON value (e.g. cat obj/key -> "value") 
To check functionality, you can use a test script. To do this, review the README.md file in the test directory at the project.

## License
This project is licensed under the GPLv3. See the LICENSE file for more details.
## Feedback
To contact the developer, you can email zveror1806@gmail.com
