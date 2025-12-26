# jsonfs

A FUSE-based file system designed for editing a JSON file.

## Prerequisites

Check for the following programs:

### Required:

* `gcc`
* `make`
* `libjansson-dev`
* `libfuse3-dev`

>P.S. To install libjansson-dev and libfuse3-dev, use your system's package manager.

### Optional:

* `curl` and `unzip` — if you plan to download the archive,
* `git` — if you prefer to clone the repository.

## Download

You can obtain the source code in one of two ways:

### Download the archive:

```bash
curl -L -o jsonfs.zip https://github.com/zverorx/jsonfs/archive/refs/tags/latest.zip
```

```bash
unzip jsonfs.zip
```

### Clone a repository:

```bash
git clone https://github.com/zverorx/jsonfs.git
```

## Compilation and installation

Go to the project directory, and run:

```bash
make && sudo make install
```

To find out more about Makefile features, run:

```bash
make help
```

## Usage

Mounting:

```bash
jsonfs <json_file> <mount_point> [fuse_options]
```

Unmounting:

```bash
fusermount3 -u <mount_point>
```

## Example

If the original JSON file looks like this:

```json
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
```

then the mount point will contain the following:

```
.
├── interests
│   ├── @0: "photography" 
│   ├── @1: "cooking"
│   └── @2: "skiing"
├── phone: null
├── profile
│   ├── apartment: 45
│   ├── city: "Saint Petersburg"
│   └── street: "Nevsky Prospect"
└── user: "Ivan Petrov"
 ```

The key is the file name, the value is the file contents.

The '@' symbol is a special prefix, which means that such a key does not exist in the original JSON file, but it is needed to represent data as a file system, for example, array indexes or a root scalar.

>Full information is provided in the docs directory.

## License

This project is licensed under the GPLv3. See the LICENSE file for more details.

## Feedback

To contact the developer, you can email zveror1806@gmail.com.
