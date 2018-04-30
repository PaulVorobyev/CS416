/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.

*/

#include "params.h"
#include "block.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"

int find_free(char bitmap[], int size) {
    int i = 0;
    for (; i < size; i++) {
        if (bitmap[i] == '0') return i;
    }

    return -1;
}

char *get_relative_path(char *full_path) {
    char *prev = malloc(PATH_MAX);
    char rel_path[PATH_MAX];
    strcpy(rel_path, full_path);
    char* token = strtok(rel_path, "/");

    while (token) {
        printf("token: %s\n", token);
        token = strtok(NULL, " ");
        strcpy(prev, token);
    }

    return prev;
}

void *get_block(int i) {
    void* buf = calloc(1, BLOCK_SIZE);
    block_read(i, buf);

    return buf;
}

inode *find_inode(const char *path) {
    int i = INODE_START;
    for (; i < INODE_END; i++) {
        inode *tmp = (inode*) get_block(i);

        // check if name is equal to given path
        if (strcmp(tmp->full_path, path) == 0){
            return tmp;
        }

        free(tmp);
    }

    return NULL;
}

int make_inode(const char *full_path, int is_dir) {
    int idx = find_free(SFS_DATA->inodes, MAX_INODES);

    if (idx == -1) {
        return -1;
    }

    inode *in = (inode*) get_block(idx + INODE_START);

    in->is_dir = is_dir;
    strcpy(in->full_path, full_path);
    char charbuf[PATH_MAX];
    strcpy(charbuf, full_path);
    strcpy(in->relative_path, get_relative_path(charbuf));

    block_write(idx + INODE_START, in);

    SFS_DATA->inodes[idx] = '1';

    free(in);

    return idx + INODE_START;
}


///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *sfs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "in bb-init\n");
    log_msg("\nsfs_init()\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());

    // init inode and datablock bitmaps
    int i = 0;
    for (; i < NUM_DATABLOCKS; i++) {
        if (i < MAX_INODES) SFS_DATA->inodes[i] = '0';
        SFS_DATA->datablocks[i] = '0';
    }

    // open the disk
    disk_open(SFS_DATA->diskfile);

    // init superblock
    int superblocknum = 462;
    int *buf = calloc(1, 512);
    *buf = superblocknum;
    block_write(SUPERBLOCK, buf);

    // init inodes
    inode *inbuf = calloc(1, 512);
    *inbuf = (inode) {
        .st_ino = -1,
        .st_size = 0,
        .st_blksize = BLOCK_SIZE,
        .full_path = 0,
        .relative_path = 0,
        .r = (refs) { 
            .children = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
            .indirect = -1
        },
        .is_dir = -1
    };
    i = INODE_START;
    int idx = 0;
    for (; i < INODE_END; i++) {
        inbuf->st_ino = idx++;
        block_write(i, inbuf);
    }

    // make '/'
    inbuf->is_dir = 1;
    strcpy(inbuf->full_path, "/");
    strcpy(inbuf->relative_path, "/");
    block_write(INODE_START, inbuf);

    log_msg("\nBRUHHHHHHH\n");
    return SFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */

int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    /* char fpath[PATH_MAX]; */
    
    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);

    inode *target = find_inode(path);

    if (!target) {
        // TODO: handle this err
        log_msg("\nsfs_getattr() I CANT FIND THE INODEEEE: %s\n", path);
    }

    // TODO: check if dir

    // values inside inode
    statbuf->st_ino = target->st_ino;
    statbuf->st_size = target->st_size;
    statbuf->st_blksize = target->st_blksize;
    statbuf->st_nlink = 2;
    statbuf->st_mode = S_IFDIR | 0755;    /* protection */

    // useless data
    statbuf->st_dev = 0;     /* ID of device containing file */
    statbuf->st_uid = 0;     /* user ID of owner */
    statbuf->st_gid = 0;     /* group ID of owner */
    statbuf->st_rdev = 0;    /* device ID (if special file) */
    statbuf->st_blocks = 0;  /* number of 512B blocks allocated */
    statbuf->st_atime = 0;   /* time of last access */
    statbuf->st_mtime = 0;   /* time of last modification */
    statbuf->st_ctime = 0; 

    log_msg("\nsfs_getattr() WE HAVE FILLED THE STATBUF\n", path);
    
    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 * * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);
    
    inode *target = find_inode(path);

    // file not found, so make it
    int child = -1;
    if (!target) {
        if ((child = make_inode(path, 0)) == -1) {
            // err
        }
    }

    // add to '/' children
    target = find_inode("/");
    refs *r = &target->r;
    while (1) {
        int i = 0;
        for (; i < 12; i++) {
            if (r->children[i] == -1) {
                r->children[i] = child;
                return retstat;
            }
        }

        if (r->indirect == -1) {
            int idx = find_free(SFS_DATA->datablocks, NUM_DATABLOCKS);
            if (idx == -1) {
                // TODO: err, no room for kid ref
            }

            r = (refs*) get_block(idx + DATABLOCK_START);
            *r = (refs) {
                .children = {child, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
                .indirect = -1
            };

            r->indirect = idx;

            block_write(idx, r);

            free(r);

            return retstat;
        }

        r = (refs*) get_block(r->indirect);
    }
    
    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    
    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);

    
    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    

    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);

   
    return retstat;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    
    
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
   
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    
    
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    
    
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    
    inode *target = find_inode(path);
    if (!target) {
        // TODO: handle this err
    }

    refs r = target->r;
    while (1) {
        int i = 0;
        for (; i < 12; i++) {
            if (r.children[i] == -1) {
                continue;
            }
            
            inode *inodeblock = calloc(1, 512);
            block_read(r.children[i], inodeblock);
            inode in = *inodeblock;
            if (filler(buf, in.relative_path, NULL, 0) != 0) {
                return 0;
            }
        }

        if (r.indirect == -1) {
            break;
        }

        refs *refblock = calloc(1, 512);
        block_read(r.indirect, refblock);
        r = *refblock;
        free(refblock);
    }
    
    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    
    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,

  .getattr = sfs_getattr,
  .create = sfs_create,
  .unlink = sfs_unlink,
  .open = sfs_open,
  .release = sfs_release,
  .read = sfs_read,
  .write = sfs_write,

  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    sfs_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
