#include "operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tfs_init() {
    state_init();

    /* create root inode */
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }

    return 0;
}

int tfs_destroy() {
    state_destroy();
    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}


int tfs_lookup(char const *name) {
    if (!valid_pathname(name)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(ROOT_DIR_INUM, name);
}

int tfs_open(char const *name, int flags) {
    int inum;
    size_t offset;

    /* Checks if the path name is valid */
    if (!valid_pathname(name)) {
        return -1;
    }

    inum = tfs_lookup(name);
    if (inum >= 0) {
        /* The file already exists */
        inode_t *inode = inode_get(inum);
        if (inode == NULL) {
            return -1;
        }

        /* Trucate (if requested) */
        if (flags & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                if (data_block_free(inode->i_data_block[0]) == -1) {
                    return -1;
                }
                inode->i_size = 0;
            }
        }
        /* Determine initial offset */
        if (flags & TFS_O_APPEND) {
            offset = inode->i_size;
        } else {
            offset = 0;
        }
    } else if (flags & TFS_O_CREAT) {
        /* The file doesn't exist; the flags specify that it should be created*/
        /* Create inode */
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1;
        }
        /* Add entry in the root directory */
        if (add_dir_entry(ROOT_DIR_INUM, inum, name + 1) == -1) {
            inode_delete(inum);
            return -1;
        }
        offset = 0;
    } else {
        return -1;
    }

    /* Finally, add entry to the open file table and
     * return the corresponding handle */
    return add_to_open_file_table(inum, offset);

    /* Note: for simplification, if file was created with TFS_O_CREAT and there
     * is an error adding an entry to the open file table, the file is not
     * opened but it remains created */
}


int tfs_close(int fhandle) { return remove_from_open_file_table(fhandle); }

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }

    /* Determine how many bytes to write */
    /*if (to_write + file->of_offset > BLOCK_SIZE) {
        to_write = BLOCK_SIZE - file->of_offset;
    }*/
    size_t reallyWritten = 0;

    while(to_write > 0){
        int blockIndex = inode->i_size / BLOCK_SIZE;

        void *block = data_block_get(inode->i_data_block[blockIndex]);
        if (block == NULL) {
            inode->i_data_block[blockIndex] = data_block_alloc();
        }
        if (to_write > BLOCK_SIZE - file -> of_offset){
            int written = BLOCK_SIZE - file -> of_offset;
            to_write -= written;
            memcpy(block + file->of_offset, buffer, written);
            inode -> i_size += written;
            file -> of_offset = 0;
            reallyWritten += written;
            buffer += written;
        }
        else {
            memcpy(block + file->of_offset, buffer, to_write);
            inode -> i_size += to_write;
            file -> of_offset += to_write;
            reallyWritten += to_write;
            break;
        }
    }
    return reallyWritten;
}

/*
    if (to_write > 0 ) {
        if (inode->i_size == 0) {
            //If empty file, allocate new block
            inode->i_data_block[0] = data_block_alloc();
        }

        void *block = data_block_get(inode->i_data_block[0]);
        if (block == NULL) {
            return -1;
        }

        //Perform the actual write
        memcpy(block + file->of_offset, buffer, to_write);

        //The offset associated with the file handle is
        //incremented accordingly
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }
    return (ssize_t)to_write;
}
*/

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    //From the open file table entry, we get the inode
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }

    //Determine how many bytes to read
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    if (file->of_offset + to_read >= BLOCK_SIZE) {
        return -1;
    }

    int i = 0;

    while(to_read > 0){
        void *block = data_block_get(inode->i_data_block[i]);
        if (block == NULL) {
            break;
            //return -1;
        }

        //Perform the actual read
        memcpy(buffer, block + file->of_offset, to_read);
        //The offset associated with the file handle is
        //incremented accordingly
        file->of_offset += to_read;
        buffer += to_read;
        i++;
    }
    /*if (to_read > 0) {
        void *block = data_block_get(inode->i_data_block);
        if (block == NULL) {
            return -1;
        }

        //Perform the actual read
        memcpy(buffer, block + file->of_offset, to_read);
        //The offset associated with the file handle is
        //incremented accordingly
        file->of_offset += to_read;
    }*/

    return (ssize_t)to_read;
}

int tfs_copy_to_external_fs(char const *source_path, char const *dest_path){

   char buffer[128];
   memset(buffer,0,sizeof(buffer));

    long int bytes_read, bytes_written;
    int srcFile, dstFile;

    if(tfs_lookup(source_path) == -1){
        return -1;
    }
    if (tfs_lookup(dest_path) != 0){
        dstFile = tfs_open(dest_path, TFS_O_CREAT);
    }
    srcFile = tfs_open(source_path, 0);
    dstFile = tfs_open(dest_path,TFS_O_TRUNC);

    do{
        bytes_read = tfs_read(srcFile, buffer, sizeof(buffer) - 1);
        FILE* fd = fopen(dest_path,"w");
        bytes_written = (long int)fwrite(buffer,1,(size_t)bytes_read,fd);
        //bytes_written = tfs_write(dstFile, buffer, (size_t)bytes_read);
        if(bytes_read != bytes_written){
            printf("%ld %ld", bytes_read, bytes_written);
            return -2;
        }
    } while(bytes_read >= sizeof(buffer)-1);
    
    tfs_close(srcFile);
    tfs_close(dstFile);
    return 0;
}