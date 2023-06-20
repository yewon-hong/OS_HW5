# myfuse

myfuse is a simple FUSE-based file system that loads file system structure from a JSON file and provides read-only access to the files and directories specified in the JSON.

## Requirements

- FUSE library (version 3.x)
- json-c library (version 0.13 or later)

## Compilation

To compile the myfuse program, execute the following command:

```shell
gcc -Wall -o myfuse myfuse.c -lfuse -ljson-c -lpthread

## Run

To run the myfuse program, execute the following command:

```shell
mkdir myfuse_dir
./myfuse ./myfuse_dir fs.json

Usage
Run the myfuse program with the following command:

shell
./myfuse <json_file> <mount_point>
<json_file>: The path to the JSON file that specifies the file system structure.
<mount_point>: The path to an empty directory where the file system will be mounted.
