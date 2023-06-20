# OS_HW5

## Team Members:
21900805 Yewon Hong

## myfuse

myfuse is a simple FUSE-based file system that loads file system structure from a JSON file and provides read-only access to the files and directories specified in the JSON.

## Requirements

- FUSE library (version 3.x)
- json-c library (version 0.13 or later)

## Compilation

To compile the myfuse program, execute the following command:

```
gcc -Wall -o myfuse myfuse.c -lfuse -ljson-c -lpthread
```

## Run

To run the myfuse program, execute the following command:

```
mkdir myfuse_dir

./myfuse ./myfuse_dir fs.json
```

## File System Structure JSON
The JSON file provided to the myfuse program should have the following structure:

fs.json
```
[
  {
    "inode": 1,
    "type": "dir",
    "entries": [
      {
        "name": "file1.txt",
        "inode": 2
      },
      {
        "name": "file2.txt",
        "inode": 3
      }
    ]
  },
  {
    "inode": 2,
    "type": "reg",
    "data": "This is the content of file1.txt"
  },
  {
    "inode": 3,
    "type": "reg",
    "data": "This is the content of file2.txt"
  }
]
```
- Each object in the JSON array represents a file or directory in the file system.
- The inode field specifies the unique identifier for the file or directory.
- The type field specifies the type of the entry. It can be either "dir" for directories or "reg" for regular files.
- For directories, the entries field is an array of objects representing the directory entries. Each entry object has a name field and an inode field specifying the name and inode of the entry, respectively.
- For regular files, the data field contains the content of the file.

## Functions
The following functions are implemented in the myfuse program:

### load_file_system
```
void load_file_system(const char* json_file)
```

- Description: Loads the file system structure from a JSON file and populates the file_system array.
- Parameters:
  - json_file: The path to the JSON file.
  - Return Type: void

### fs_getattr

```
static int fs_getattr(const char* path, struct stat* stbuf)
```

- Description: Retrieves the attributes of a file or directory specified by the given path.
- Parameters:
  - path: The path to the file or directory.
  - stbuf: Pointer to the struct stat object where the attributes will be stored.
- Return Type: int
  - 0 if successful.
  - '-ENOENT' if the file or directory does not exist.

### fs_readdir

```
static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
```

- Description: Reads the contents of a directory specified by the given path.
- Parameters:
  - path: The path to the directory.
- buf: Buffer to store the directory entries.
  - filler: Function to add directory entries to buf.
  - offset: The directory read offset.
  - fi: Pointer to the struct fuse_file_info object.
- Return Type: int
  - 0 if successful.
  - '-ENOENT' if the directory does not exist.

###fs_read

```
static int fs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
```

- Description: Reads the contents of a file specified by the given path.
- Parameters:
  - path: The path to the file.
  - buf: Buffer to store the read data.
  - size: The size of the data to read.
  - offset: The read offset.
  - fi: Pointer to the struct fuse_file_info object.
- Return Type: int
  - The number of bytes read if successful.
  - '-ENOENT' if the file does not exist.

## Issues & Limitations
One issue encountered during the execution of the program is a segmentation fault. It appears that there might be problems in synchronizing the functions, and this issue has not been resolved. Prior to using locks, folders and files were created in the mounted directory, but they were not created accurately.

## Contact Information
21900805 Yewon Hong (21900805@handong.ac.kr)
