/*
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  There are a couple of symbols that need to be #defined before
  #including all the headers.
*/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

// maintain bbfs state in here
#include <limits.h>
#include <stdio.h>

/* USEFUL STUFF FOR SFS */

typedef struct datablock_entry {
    char is_free;
    int index;
    struct datablock_entry next;
} datablock_entry;

typedef struct _inode {
    ino_t     st_ino;     /* inode number */
    nlink_t   st_nlink;   /* number of hard links */
    off_t     st_size;    /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    char *filename;
    datablock_entry *datablock_index;
} inode;

struct sfs_state {
    FILE *logfile;
    char *diskfile;
    FILE *disk;
    char *inodes;
    datablock_entry *datablocks;
};

#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#define NUM_DATABLOCKS 31000
#define MAX_INODES 200
#define NUM_BLOCKS 31250
// (1) superblock| (200) inodes| (31000) datablocks

#endif
