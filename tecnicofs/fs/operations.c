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
                for(int i = 0; i<10;i++)
                    if (data_block_free(inode->i_data_block[i]) == -1) {
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

int createBlock(inode_t *inode, int blockIndex){
    //printf("new block: %d\n",blockIndex);
    if(blockIndex >= 10){
        int memIndex = data_block_alloc();
        void *indexBlock = data_block_get(inode->i_data_block[10]);
        if(indexBlock == NULL){
            inode->i_data_block[10] = data_block_alloc();
            indexBlock = data_block_get(inode->i_data_block[10]);
        } 
        memcpy(indexBlock + (blockIndex-10), &memIndex, sizeof(int));
        //printf("saving block:%d\n",memIndex);
        return memIndex;
    } else{
        inode->i_data_block[blockIndex] = data_block_alloc();
        return blockIndex;
    }
}

void *getBlock(inode_t *inode,int blockIndex){
    if(blockIndex >= 10){
        void *indexBlock = data_block_get(inode->i_data_block[10]);
        //if(indexBlock == NULL) printf("aaaaaaaaa\n");
        int memIndex = -1;
        memcpy(&memIndex, indexBlock+(blockIndex-10), sizeof(memIndex));
        //printf("getting block:%d %d\n",memIndex,data_block_get(memIndex) == NULL);
        //if(data_block_get(memIndex)== NULL) printf("aaaaaaa\n");
        if(memIndex==0) return NULL;
        //printf("block%d: %d %d\n",blockIndex, data_block_get(memIndex)==NULL,memIndex);
        return data_block_get(memIndex);
    } else{
        return data_block_get(inode->i_data_block[blockIndex]);
    }
}

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

    size_t wrote = 0;

    int blockIndex = (int)(file->of_offset / BLOCK_SIZE);
    while (to_write > 0) {
        //printf("block: %d\n",blockIndex);
        void *block = getBlock(inode,blockIndex);
        if (block == NULL) {
            printf("new block: %d\n", blockIndex);
            createBlock(inode,blockIndex);
            block = getBlock(inode, blockIndex);
        }
        if(blockIndex < (file->of_offset + to_write) / BLOCK_SIZE){
            size_t nextBlock = BLOCK_SIZE - (file->of_offset % BLOCK_SIZE);
            memcpy(block + (file->of_offset % BLOCK_SIZE), buffer, nextBlock);
            printf("block: %d writing: %d(%d -> %d)\n",blockIndex, (int)nextBlock,(int)file->of_offset, (int)(file->of_offset+nextBlock));
            inode -> i_size += nextBlock;
            wrote += nextBlock;
            buffer += nextBlock;
            file->of_offset += nextBlock;
            to_write -= nextBlock;
            blockIndex++;
        } else{
            memcpy(block + (file->of_offset % BLOCK_SIZE), buffer, to_write);
            printf("block: %d writing: %d(%d -> %d)\n",blockIndex, (int)to_write,(int)file->of_offset, (int)(file->of_offset+to_write));
            inode -> i_size += to_write;
            wrote += to_write;
            buffer += to_write;
            file->of_offset += to_write;
            to_write -= to_write;
        }
    }
    return (ssize_t)wrote;
}

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

    //printf("of_offsett: %d\n", (int)file->of_offset);

    size_t read = 0;
    int blockIndex = (int)(file->of_offset/BLOCK_SIZE);
    //printf("to_read: %d starting in block: %d\n", (int)to_read, blockIndex);

    while (to_read > 0) {
        //printf("block: %d(pos%ld)\n",blockIndex,file->of_offset);

        //void *block = data_block_get(inode->i_data_block[blockIndex]);
        void *block = getBlock(inode, blockIndex);

        if(file->of_offset/BLOCK_SIZE < (file->of_offset + to_read) / BLOCK_SIZE){
            size_t nextBlock = BLOCK_SIZE - (file->of_offset % BLOCK_SIZE);
            memcpy(buffer, block + (file->of_offset%BLOCK_SIZE), nextBlock);
            printf("block: %d reading: %d from: %d to: %d\n",blockIndex, (int)nextBlock,(int)file->of_offset,(int)(file->of_offset+nextBlock));
            read += nextBlock;
            buffer += nextBlock;
            file->of_offset += nextBlock;
            to_read -= nextBlock;
            blockIndex++;
        } else{
            memcpy(buffer, block + (file->of_offset%BLOCK_SIZE), to_read);
            printf("block: %d reading: %d from: %d to: %d\n",blockIndex, (int)to_read,(int)file->of_offset,(int)(file->of_offset+to_read));
            read += to_read;
            buffer += to_read;
            file->of_offset += to_read;
            to_read -= to_read;
        }
    }
    return (ssize_t)read;   
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
        //FILE* fd = fopen(dest_path,"w");
        //bytes_written = (long int)fwrite(buffer,1,(size_t)bytes_read,fd);
        bytes_written = tfs_write(dstFile, buffer, (size_t)bytes_read);
        if(bytes_read != bytes_written){
            printf("%ld %ld", bytes_read, bytes_written);
            return -2;
        }
    } while(bytes_read >= sizeof(buffer)-1);
    
    tfs_close(srcFile);
    tfs_close(dstFile);
    return 0;
}