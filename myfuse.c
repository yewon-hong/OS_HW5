#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <json-c/json.h>
#include <pthread.h>

#define MAX_FILES 128
#define MAX_NAME_LENGTH 255
#define MAX_CONTENT_LENGTH 4098
#define MAX_DIRECTORY_ENTRIES 16

typedef struct {
    char name[MAX_NAME_LENGTH];
    int inode;
} Entry;

typedef struct {
    int inode;
    char type[MAX_NAME_LENGTH];
    char data[MAX_CONTENT_LENGTH];
    Entry entries[MAX_DIRECTORY_ENTRIES];
    int num_entries;
} Node;

Node file_system[MAX_FILES];
int num_files = 0;

pthread_mutex_t fs_mutex;

void load_file_system(const char* json_file) {
    FILE* file = fopen(json_file, "r");
    if (file == NULL) {
        perror("Error opening JSON file");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_CONTENT_LENGTH];
    memset(buffer, 0, sizeof(buffer));

    fread(buffer, sizeof(buffer), 1, file);
    fclose(file);

    struct json_object* root_obj = json_tokener_parse(buffer);
    int num_nodes = json_object_array_length(root_obj);

    for (int i = 0; i < num_nodes; i++) {
        struct json_object* node_obj = json_object_array_get_idx(root_obj, i);

        struct json_object* inode_obj;
        if (json_object_object_get_ex(node_obj, "inode", &inode_obj)) {
            int inode = json_object_get_int(inode_obj);

            struct json_object* type_obj;
            if (json_object_object_get_ex(node_obj, "type", &type_obj)) {
                const char* type = json_object_get_string(type_obj);

                pthread_mutex_lock(&fs_mutex);

                strcpy(file_system[num_files].type, type);
                file_system[num_files].inode = inode;

                if (strcmp(type, "reg") == 0) {
                    struct json_object* data_obj;
                    if (json_object_object_get_ex(node_obj, "data", &data_obj)) {
                        const char* data = json_object_get_string(data_obj);
                        strcpy(file_system[num_files].data, data);
                    }
                } else if (strcmp(type, "dir") == 0) {
                    struct json_object* entries_obj;
                    if (json_object_object_get_ex(node_obj, "entries", &entries_obj)) {
                        int num_entries = json_object_array_length(entries_obj);
                        file_system[num_files].num_entries = num_entries;

                        for (int j = 0; j < num_entries; j++) {
                            struct json_object* entry_obj = json_object_array_get_idx(entries_obj, j);

                            struct json_object* name_obj;
                            if (json_object_object_get_ex(entry_obj, "name", &name_obj)) {
                                const char* name = json_object_get_string(name_obj);

                                struct json_object* entry_inode_obj;
                                if (json_object_object_get_ex(entry_obj, "inode", &entry_inode_obj)) {
                                    int entry_inode = json_object_get_int(entry_inode_obj);

                                    strcpy(file_system[num_files].entries[j].name, name);
                                    file_system[num_files].entries[j].inode = entry_inode;
                                }
                            }
                        }

                        for (int j = num_entries; j < MAX_DIRECTORY_ENTRIES; j++) {
                            memset(&file_system[num_files].entries[j], 0, sizeof(Entry));
                        }
                    }
                }

                num_files++;

                pthread_mutex_unlock(&fs_mutex);
            }
        }
    }

    json_object_put(root_obj);
}

static int fs_getattr(const char* path, struct stat* stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    pthread_mutex_lock(&fs_mutex);

    for (int i = 0; i < num_files; i++) {
        if (strcmp(file_system[i].type, "dir") == 0 && strcmp(file_system[i].entries[0].name, path + 1) == 0) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

            pthread_mutex_unlock(&fs_mutex);
            return 0;
        } else if (strcmp(file_system[i].type, "reg") == 0 && strcmp(file_system[i].entries[0].name, path + 1) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            stbuf->st_size = strlen(file_system[i].data);

            pthread_mutex_unlock(&fs_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&fs_mutex);
    return -ENOENT;
}



static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    (void)offset;
    (void)fi;

    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    pthread_mutex_lock(&fs_mutex);

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (int i = 0; i < num_files; i++) {
        if (strcmp(file_system[i].type, "dir") == 0) {
            for (int j = 0; j < file_system[i].num_entries; j++) {
                filler(buf, file_system[i].entries[j].name, NULL, 0);
            }
        }
    }

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}


static int fs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    (void)fi;

    pthread_mutex_lock(&fs_mutex);

    for (int i = 0; i < num_files; i++) {
        if (strcmp(file_system[i].type, "reg") == 0 && strcmp(file_system[i].entries[0].name, path + 1) == 0) {
            size_t len = strlen(file_system[i].data);

            if (offset < len) {
                if (offset + size > len) {
                    size = len - offset;
                }
                memcpy(buf, file_system[i].data + offset, size);
            } else {
                size = 0;
            }

            pthread_mutex_unlock(&fs_mutex);
            return size;
        }
    }

    pthread_mutex_unlock(&fs_mutex);
    return -ENOENT;
}

static struct fuse_operations fs_operations = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .read = fs_read,
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <json_file> <mount_point>\n", argv[0]);
        return -1;
    }

    load_file_system(argv[1]);

    pthread_mutex_init(&fs_mutex, NULL);

    return fuse_main(argc - 1, argv + 1, &fs_operations, NULL);
}
