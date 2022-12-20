#include "operations.h"
#include "config.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "betterassert.h"

#define HARD_LINK_FLAG  (0)
#define SOFT_LINK_FLAG  (1)
#define MAX_INODE_COUNT (64)

pthread_mutex_t operations_rwlock[MAX_INODE_COUNT];

/**
 * Index to root_dir is always 0. operations_rwlock[0] only for root_dir
*/

tfs_params tfs_default_params() {
    tfs_params params = {
        .max_inode_count = MAX_INODE_COUNT,
        .max_block_count = 1024,
        .max_open_files_count = 16,
        .block_size = 1024,
        .thread_flag = 0,       // default state doesnt use pthread locks 
    };
    return params;
}

int tfs_init(tfs_params const *params_ptr) {
    tfs_params params;
    if (params_ptr != NULL) {
        params = *params_ptr;
    } else {
        params = tfs_default_params();
    }

    if (state_init(params) != 0) {
        return -1;
    }

    // create root inode
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }
    if (params.thread_flag == 1)
        for (int i = 0; i < MAX_INODE_COUNT; i++) { 
            if (pthread_mutex_init(&(operations_rwlock[i]), NULL) != 0)
                return -1;
        }
    

    return 0;
}

int tfs_destroy() {
    if (state_destroy() != 0) {
        return -1;
    }
    if (tfs_default_params().thread_flag == 1)
        for (int i = 0; i < MAX_INODE_COUNT; i++) {
            if(pthread_mutex_destroy(&operations_rwlock[i]) != 0) return -1;
        }

    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}

/**
 * Looks for a file.
 *
 * Note: as a simplification, only a plain directory space (root directory only)
 * is supported.
 *
 * Input:
 *   - name: absolute path name
 *   - root_inode: the root directory inode
 * Returns the inumber of the file, -1 if unsuccessful.
 */
static int tfs_lookup(char const *name, inode_t const *root_inode) {
    if (!valid_pathname(name) || root_inode != inode_get(ROOT_DIR_INUM)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(root_inode, name);
}

int tfs_open(char const *name, tfs_file_mode_t mode) {
    // Checks if the path name is valid
    if (!valid_pathname(name)) {
        return -1;
    }
    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
    ALWAYS_ASSERT(root_dir_inode != NULL,
                  "tfs_open: root dir inode must exist");
    int inum = tfs_lookup(name, root_dir_inode);
    size_t offset;

    if (inum >= 0) {
        // The file already exists
        inode_t *inode = inode_get(inum);
        ALWAYS_ASSERT(inode != NULL,
                      "tfs_open: directory files must have an inode");

        // Soft link check
        if (inode->flag == SOFT_LINK_FLAG) { 
            inum = tfs_lookup(inode->name, root_dir_inode);
            if (inum == -1) {
                if(mode & TFS_O_CREAT) {
                    inum = tfs_open(inode->name, TFS_O_CREAT);
                    inode = inode_get(inum);
                    return inum;
                }
                return -1;
            }
            if (inode->flag == SOFT_LINK_FLAG) 
                inode = inode_get(tfs_open(inode->name, mode));
            else
                inode = inode_get(inum);
        }

        // Truncate (if requested)
        if (mode & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                data_block_free(inode->i_data_block);
                inode->i_size = 0;
            }
        }
        // Determine initial offset
        if (mode & TFS_O_APPEND) {
            offset = inode->i_size;
        } else {
            offset = 0;
        }
    } else if (mode & TFS_O_CREAT) {
        // The file does not exist; the mode specified that it should be created
        // Create inode
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1; // no space in inode table
        }

        // Add entry in the root directory
        if (add_dir_entry(root_dir_inode, name + 1, inum) == -1) {
            inode_delete(inum);
            return -1; // no space in directory
        }

        offset = 0;
    } else {
        return -1;
    }
    // Finally, add entry to the open file table and return the corresponding
    // handle
    return add_to_open_file_table(inum, offset);
    // Note: for simplification, if file was created with TFS_O_CREAT and there
    // is an error adding an entry to the open file table, the file is not
    // opened but it remains created
}
    /**
     * target = file in the TFS system
     * link_name = name of the link used
     * Ao criar um soft link "/path_A/soft_link" para um ficheiro "/path_B/file_name"
     */
int tfs_sym_link(char const *target, char const *link_name) {
    if(!valid_pathname(target) || !valid_pathname(link_name)) return -1;

    
    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
    int soft_index = inode_create(T_DIRECTORY);

    if(soft_index < 0) return -1;
    pthread_mutex_lock(&(operations_rwlock[soft_index]));
    inode_t * soft_link = inode_get(soft_index);

    if (add_dir_entry(root_dir_inode, link_name + 1, soft_index) == -1) {
        inode_delete(soft_index);
        pthread_mutex_unlock(&(operations_rwlock[soft_index]));
        return -1; // no space in directory
    }

    soft_link->flag = SOFT_LINK_FLAG;
    soft_link->name = malloc(sizeof(char *));
    memcpy(soft_link->name, target, strlen(target) + 1);
    pthread_mutex_unlock(&(operations_rwlock[soft_index]));
    return 0;
}

int tfs_link(char const *target, char const *link_name) {
    if(!valid_pathname(target) || !valid_pathname(link_name)) return -1;
    

    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);

    int idx = tfs_lookup(target, root_dir_inode);
    if(idx < 0) return -1;

    pthread_mutex_lock(&(operations_rwlock[idx]));

    inode_t * targ = inode_get(idx);

    if (targ->flag == SOFT_LINK_FLAG) {
        pthread_mutex_unlock(&(operations_rwlock[idx]));
        return -1;
    }

    if (add_dir_entry(root_dir_inode, link_name + 1, idx) == -1){
        pthread_mutex_unlock(&(operations_rwlock[idx]));
        return -1; // no space in directory
    }

    targ->flag = HARD_LINK_FLAG; // not necessary
    targ->hard_link_count++;
    pthread_mutex_unlock(&(operations_rwlock[idx]));
    return 0;
}

int tfs_close(int fhandle) {    
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1; // invalid fd
    }

    remove_from_open_file_table(fhandle);
    
    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {

    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    //  From the open file table entry, we get the inode
    int idx = file->of_inumber;
    inode_t *inode = inode_get(idx);
    ALWAYS_ASSERT(inode != NULL, "tfs_write: inode of open file deleted");

    // Determine how many bytes to write
    size_t block_size = state_block_size();
    pthread_mutex_lock(&(operations_rwlock[idx]));
    if (to_write + file->of_offset > block_size) {
        to_write = block_size - file->of_offset;
    }

    if (to_write > 0) {
        if (inode->i_size == 0) {
            // If empty file, allocate new block
            int bnum = data_block_alloc();
            if (bnum == -1) {
                pthread_mutex_unlock(&(operations_rwlock[idx]));
                return -1; // no space
            }

            inode->i_data_block = bnum;
        }

        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_write: data block deleted mid-write");

        // Perform the actual write
        memcpy(block + file->of_offset, buffer, to_write);

        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }
    pthread_mutex_unlock(&(operations_rwlock[idx]));
    return (ssize_t)to_write;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    int idx = file->of_inumber;
    // From the open file table entry, we get the inode
    inode_t const *inode = inode_get(idx);
    ALWAYS_ASSERT(inode != NULL, "tfs_read: inode of open file deleted");
    pthread_mutex_lock(&(operations_rwlock[idx]));
    // Determine how many bytes to read
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    if (to_read > 0) {
        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_read: data block deleted mid-read");

        // Perform the actual read
        memcpy(buffer, block + file->of_offset, to_read);
        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_read;
    }
    pthread_mutex_unlock(&(operations_rwlock[idx]));
    return (ssize_t)to_read;
}

int tfs_unlink(char const *target) {
    if (!valid_pathname(target)) return -1;

    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);

    int idx = tfs_lookup(target, root_dir_inode);
    
    if (idx < 0) return -1;
    pthread_mutex_lock(&(operations_rwlock[idx]));
    inode_t * node = inode_get(idx);

    if (node->flag == SOFT_LINK_FLAG) {
        free(node->name);
        inode_delete(idx);
        //return clear_dir_entry(root_dir_inode, target + 1);
    }
    
    else if (node->flag == HARD_LINK_FLAG) { 
        if(node->hard_link_count == 1) 
            inode_delete(idx);
            
        node->hard_link_count--;
        //return clear_dir_entry(root_dir_inode, target + 1);
    }
    pthread_mutex_unlock(&(operations_rwlock[idx]));
    return clear_dir_entry(root_dir_inode, target + 1);
}

int tfs_copy_from_external_fs(char const *source_path, char const *dest_path) {
    if(!valid_pathname(dest_path)) return -1;
    
    FILE * file = fopen(source_path,"r");
    if(file == NULL) return -1;

    size_t block = tfs_default_params().block_size;


    
    int dest = tfs_open(dest_path, TFS_O_CREAT | TFS_O_TRUNC);
    if(dest == -1) return -1;
    char buffer[block];
    memset(buffer, 0, block);

    int flag = 0;
    if(tfs_write(dest, &buffer, fread(&buffer, sizeof(char), block-1, file)) == -1)  flag = 1;
    
    if(tfs_close(dest) != 0 || fclose(file) != 0) return -1;
    if(flag) return -1;

    return 0;
}
