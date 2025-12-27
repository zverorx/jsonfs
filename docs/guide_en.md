# User guide

## Table of contents

* [Introduction](#introduction)
* [Prerequisites](#prerequisites)
* [Obtaining the Project](#obtaining-the-project)
    * [Download Archive](#download-archive)
    * [Clone repository](#clone-repository)
* [Building](#building)
    * [Compilation and Installation](#compilation-and-installation)
    * [Uninstallation](#uninstallation)
    * [Cleaning](#cleaning)
    * [Hints](#hints)
* [Filesystem Organization](#filesystem-organization)
    * [General Principles](#general-principles)
    * [Deserialization and Serialization](#deserialization-and-serialization)
        * [Arrays](#arrays)
        * [Top-level Primitive](#top-level-primitive)
        * [Slash in Keys](#slash-in-keys)
        * [Changing the Special Prefix](#changing-the-special-prefix)
    * [Special Files](#special-files)
    * [File Attributes](#file-attributes)
* [Usage](#usage)
    * [Mounting and Unmounting](#mounting-and-unmounting)
        * [Mounting](#mounting)
        * [Unmounting](#unmounting)
    * [Functionality](#functionality)
    * [Notes on Reading and Writing](#notes-on-reading-and-writing)
    * [Saving](#saving)
* [Usage Errors](#usage-errors)
    * [Build Errors](#build-errors)
    * [Mounting Error](#mounting-error)
    * [File Write Errors](#file-write-errors)
    * [Incorrect Saving](#incorrect-saving)
* [Feedback](#feedback)

## Introduction

The program is a FUSE-based filesystem used for editing JSON format files.

## Prerequisites

Check for the following programs:

### Required:

* `gcc`
* `make`
* `libjansson-dev`
* `libfuse3-dev`

> **P.S.** For installing libfuse3-dev and libjansson-dev use your package manager.

### Optional:

* `curl` and unzip — if downloading the archive,
* `git` — if planning to clone the repository.

## Obtaining the project

You have two options:

### Download archive:

```bash
curl -L -o jsonfs.zip https://github.com/zverorx/jsonfs/archive/refs/tags/latest.zip
```

```bash
unzip jsonfs.zip
```

### Clone repository:

```bash
git clone https://github.com/zverorx/jsonfs.git
```

## Building

make should be run exclusively in the project directory.

### Compilation and installation

Go to the project directory and execute:

```bash
make && sudo make install
```

The executable will be in the bin/ directory. The obj/ directory will contain intermediate compilation files (object files).

Installation implies moving bin/jsonfs to /usr/local/bin/. You can change this path if desired by overriding the PREFIX variable.

Example:

```bash
sudo make install PREFIX=mydir
```

Then bin/jsonfs will be moved to mydir/.

### Uninstallation

Uninstallation works similarly to installation; the jsonfs file will be removed from /usr/local/bin/.

```bash
sudo make uninstall
```

The location from which to uninstall can also be changed with the PREFIX variable:

```bash
sudo make uninstall PREFIX=mydir
```

Then jsonfs will be removed from mydir/.

### Cleaning

Makefile provides the ability to delete files that are compilation results.

> The contents of obj/ and bin/ are described in [Compilation and Installation](#compilation-and-installation).

Deleting obj/:

```bash
make clean
```

Deleting obj/ and bin/:

```bash
make distclean
```

### Hints

To recall the capabilities of the project's Makefile, you don't have to reread the manual; just use:

```bash
make help
```

## Filesystem organization

### General principles

* Valid JSON complies with RFC 8259 standard; top-level primitive is supported.
* Key - file name, value - file content.
* Preservation of the sequence of "key": value pairs is not guaranteed.
* Arrays and objects become directories; all other types become files.
* For unambiguous identification of arrays, objects, primitives, and some symbols, a special prefix is used (see [Deserialization and Serialization](#deserialization-and-serialization)).
* The filesystem implements all necessary functions familiar in other systems, as well as the ability to save after editing (see [Usage](#usage)).
* The default value (e.g., when creating a file) is 0.
* All files are virtual, meaning they are not written to external media and are stored in memory; content is determined when accessing the file.
* The filesystem works exclusively with textual data, i.e., binary files are not supported.

### Deserialization and serialization

Deserialization of the mounted file and serialization during saving is implemented using the libjansson library, so if you encounter data limitations, it may be useful to review their documentation at https://jansson.readthedocs.io/en/latest/index.html.

Available primitive values include: `true`, `false`, `null`, integer, floating-point number, "string".

Some values cannot be represented within any filesystem for natural reasons, as there are issues with determining file names for array indices, top-level primitives, and invalid characters. To solve this problem, a special prefix `@` is used. Below are cases of its use.

#### Arrays

During deserialization, an array becomes an object containing keys starting with the special prefix. After it comes the ordinal number, starting from 0.

Example:

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

Will be represented in the filesystem as:

```
.
├── interests
│ ├── @0 (contains "photography")
│ ├── @1 (contains "cooking")
│ └── @2 (contains "skiing")
├── phone
├── profile
│ ├── apartment
│ ├── city
│ └── street
└── user
```

During serialization, there is an important point:

> **WARNING**: IF AT LEAST ONE FILE OR DIRECTORY HAS THE SPECIAL PREFIX AS ITS FIRST CHARACTERS, THEN ITS PARENT WILL BE SERIALIZED AS AN ARRAY, INCLUDING THE ROOT! EXCEPTION: TOP-LEVEL PRIMITIVE AND SPECIAL SLASH NOTATION, BOTH MENTIONED LATER.

#### Top-level primitive

During deserialization of a top-level primitive (sometimes called a scalar in jsonfs), it is wrapped in an object, and the value is given the key `@scalar`.

Example:

```json
123
```

Converts to:

```
.
└── @scalar (contains 123)
```

It works the same way in reverse, but:

> **WARNING**: IF A FILE NAMED `@scalar` IS ENCOUNTERED IN THE ROOT OF THE FILESYSTEM, THEN WHEN SAVING TO A JSON FILE, ONLY THE PRIMITIVE WILL BE THERE, ALL OTHER FILES WILL BE IGNORED. IF `@scalar` IS NOT IN THE ROOT, IT WILL BE AN ARRAY ELEMENT.

Example:

```
.
├── file1
├── file2
└── @scalar (contains 123)
```

When saved, becomes:

```json
123
```

#### Slash in keys

During deserialization, in all keys where there is a `/` character, it will be replaced with `@2F`. During serialization, similarly, `@2F` will be replaced with `/`.

> **WARNING**: IF `@2F` ARE THE FIRST CHARACTERS IN THE NAME, THE FILE WILL NOT BE TREATED AS AN ARRAY ELEMENT.

#### Changing the special prefix

The program does not provide the ability to change the special prefix, scalar designation, or slash notation, but this can be done by editing the source code. To do this, open include/common.h and find the macros SPECIAL_PREFIX, SPECIAL_SLASH, SCALAR_NAME; the prefix, slash, and scalar are defined there. Simply write the string that suits you there, in case `@` should be treated as a regular character. After that, [recompile the project](#compilation-and-installation).

### Special files

Besides JSON files, there are special ones, such as the familiar `.` and `..`, representing the current and parent directories, respectively.

To manage serialization, `.save` and `.status` were introduced. When writing to the first one, the process of converting the system image structure will begin, taking into account the special prefix, and saving to the mounted file. The second file can only be read, and it has one of the following values: SAVED and UNSAVED, showing whether there are unsaved changes.

These are the only files that do not participate in serialization at all. They cannot be deleted.

### File attributes

* uid and gid are determined during mounting (cannot be changed).
* File size equals the length of the string representation of the data.
* Directory size is 0, except for .. of the filesystem root.
* Permissions (cannot be changed):
    * for JSON directories: 0775,
    * for JSON files: 0666,
    * for `.status`: 0444,
    * for `.save`: 0666.
* Time:
    * atime (last read time),
    * mtime (last write time),
    * ctime (last metadata change time).
    Upon creation, they are initialized with the creation time.
* Links:
    * for directories: 2 + number of subdirectories,
    * for files: 1.

## Usage

### Mounting and unmounting

#### Mounting

```bash
jsonfs <json_file> <mount_point> [fuse_options]
```

* json_file is a required parameter, must be a valid JSON format file compliant with RFC 8259.
* mount_point is a required parameter. A directory that must be empty and match user permissions.
* fuse_options is an optional parameter for the FUSE module, usually `-f` or `-d` is used for debugging.

#### Unmounting

```bash
fusermount3 -u <mount_point>
```

* mount_point is a required parameter. The directory where jsonfs was mounted.

### Functionality

The following functions are defined in src/fuse_callbacks.c. They show the functional completeness of the system. For more detailed information about their purpose, use `man 2 [function]`.

* getattr (get file attributes),
* mknod (create file),
* mkdir (create directory),
* unlink (delete file),
* rmdir (delete directory),
* rename (rename, move),
* truncate (change file size),
* open (open file),
* read (read),
* write (write),
* readdir (read directory),
* destroy (clean up data during unmounting),
* utimens (manage timestamps).

This guide will not describe how to perform basic file operations. Study this as part of mastering your command shell.

### Notes on reading and writing

The filesystem supports working with text editors such as vim, nano. Since binary files are not supported (see [General Principles](#general-principles)), the system blocks the creation of swap files from editors, which can lead to undefined behavior, but so far no errors related to this have been observed.

File contents do not have `\n` at the end, so it is recommended to use `echo` after output, for example:

```bash
cat phone ; echo
```

### Saving

Familiarize yourself with what [special files](#special-files) are. The save trigger fires exactly at the start of writing, so it doesn't matter what exactly you write there; its content will not change and will always be equal to [the default value](#general-principles). Saving is done to the same file you mounted, so it is recommended to make copies. If you delete the original JSON while the filesystem is running, it will be created with the same name upon saving.

The save command might look like this:

```bash
echo '1' | cat > .save
```

Status can be viewed like this:

```bash
cat .status ; echo
```

## Usage errors

### Build errors

If you encounter an error during compilation, you need to:

* check the fulfillment of [prerequisites](#prerequisites). Depending on the distribution, the names for libfuse3-dev and libjansson-dev may differ. Also note that packages have the suffix `-dev`, which is important,
* check the integrity of the project. Try downloading again (see [Obtaining the Project](#obtaining-the-project)).

### Mounting error

If after the [mounting](#mounting) command, stderr says jsonfs: failed to initialize filesystem, then the problem is most likely related to the JSON file you passed as a parameter. Open it in a text editor with syntax highlighting and check.

### File write errors

Most often related to an attempt to write an invalid literal. If you use cat or a similar program for writing, stderr will show write error: Invalid argument, and for text editors there will be an error that only allows exiting the program without saving.

Pay attention to whether `null`, `true`, or `false` are written without errors. The value might be too large for integer type or too precise for floating point. Strings must be strictly in `""`. Often the error is with this data type if the user appends data using `cat >> file` or writes with an offset.

Due to the problem with processing binary files and blocking swap file creation, the problem might be related to this, so try changing the text editor.

### Incorrect saving

How serialization occurs is described in detail in [Deserialization and Serialization](#deserialization-and-serialization). First, find files in the system that have the `@` symbol, and then look at the cases where they are used.

Below are the non-obvious and important aspects of the system's operation, reiterated:

> **WARNING**: IF AT LEAST ONE FILE OR DIRECTORY HAS THE SPECIAL PREFIX AS ITS FIRST CHARACTERS, THEN ITS PARENT WILL BE SERIALIZED AS AN ARRAY, INCLUDING THE ROOT! EXCEPTION: TOP-LEVEL PRIMITIVE AND SPECIAL SLASH NOTATION, BOTH MENTIONED LATER.

> **WARNING**: IF A FILE NAMED `@scalar` IS ENCOUNTERED IN THE ROOT OF THE FILESYSTEM, THEN WHEN SAVING TO A JSON FILE, ONLY THE PRIMITIVE WILL BE THERE, ALL OTHER FILES WILL BE IGNORED. IF `@scalar` IS NOT IN THE ROOT, IT WILL BE AN ARRAY ELEMENT.

> **WARNING**: IF CHARACTER_AT2F ARE THE FIRST CHARACTERS IN THE NAME, THE FILE WILL NOT BE TREATED AS AN ARRAY ELEMENT.

## Feedback

To contact the developer, you can use the email zveror1806@gmail.com.