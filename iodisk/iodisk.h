#ifndef _IODISK_H_
#define _IODISK_H_

#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#define IODISK_VERSION "1.0.0"
#define IODISK_AUTHOR  "dannoy"
#define IODISK_CONTACT "dannoy.lee@gmail.com"

#define MAX_FILE 128
#define MAX_TEST 128

enum {
    TEST_READ  = 0x1,
    TEST_WRITE = 0x2,
    TEST_READ_RANDOM  = 0x4,
    TEST_WRITE_RANDOM = 0x8,
    TEST_READ_WRITE = 0x10,
    TEST_TYPE_MAX = TEST_READ_WRITE,
};

enum {
    DATA_0,
    DATA_1,
    DATA_R, /* generate random data */
};

enum {
    SYNC_SYNC = 0x1,
    SYNC_FSYNC,
    SYNC_FDATASYNC,
    SYNC_ODIRECT,
    SYNC_OSYNC,
    SYNC_ODIRSYNC, //O_DIRECT | O_SYNC
};

#define IODISK_USE_MMAP 0

struct tctx {
    int rw_type; 
    /*int64_t block_size;*/
    int block_size;
    int data_type;
    int sync_type;
    int use_mmap;
    int test_rounds;
    int file_size;
    /*int64_t file_size;*/
    char file[MAX_FILE][128];
    char device[128];
    int verbose;
    int random;
    int alignment;


    /*used internally*/
    int file_num;
    int fd[MAX_FILE];
    double fd_iops[MAX_FILE][MAX_TEST];
    struct stat fd_st[MAX_FILE];
    void *fd_addr[MAX_FILE];
    struct timespec start_ts[MAX_FILE][MAX_TEST];
    struct timespec stop_ts[MAX_FILE][MAX_TEST];
    double speed[MAX_FILE][MAX_TEST];

    int (*iodisk_sync)(int fd);


    int open_sync_flag;
};

#ifdef _IODISK_DEBUG_
#   define DBG(fmt, args...) printf(fmt, ##args)
#else
#   define DBG(fmt, args...)
#endif

#endif
