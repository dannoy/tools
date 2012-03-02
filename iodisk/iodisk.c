#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

//bug:iodisk -alignment

#include "iodisk.h"

void usage(void)
{
    printf("iodisk "IODISK_VERSION"\n");
    printf("author:"IODISK_AUTHOR"\n");
    printf("e-mail:"IODISK_CONTACT"\n");
    printf("\t-[rw]\tperform read or write test\n");
    printf("\t-b num[BmMkKgG]\tblock size [byte]\n");
    printf("\t-d num\twhen performing write test:\n");
    printf("\t\t\t0 for 0x0\n");
    printf("\t\t\t1 for 0x1\n");
    printf("\t\t\t2 for random data\n");
    printf("\t-s num\tsync type\n");
    printf("\t\t\t0 sync()\n");
    printf("\t\t\t1 fsync()\n");
    printf("\t\t\t2 fdatasync()\n");
    printf("\t\t\t3 open() with O_DIRECT\n");
    printf("\t\t\t4 open() with O_SYNC\n");
    printf("\t\t\t5 open() with O_DIRECT | O_SYNC\n");
    printf("\t-m\tuse mmap read & write\n");
    printf("\t-[h?]\tshow this message\n");
    printf("\n");

    exit(-1);
}

static void options_error(void)
{
    fprintf(stderr, "Parameters Error!\n");
    usage();
}

static void error(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(-1);
}

static void iodisk_verbose(struct tctx *tctx_, char *fmt, ...)
{
    va_list ap;

    if(tctx_->verbose) {
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        va_end(ap);
    }
}

static void iodisk_printf(struct tctx *tctx_, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fflush(stdout);
}

static double parse_options_bytes(char *str)
{
    double units = 1;
    double num = 0;
    switch(str[strlen(str) - 1]) {
        case'B':
            units = 1;
            break;
        case'k':
        case'K':
            units = 1024;
            break;
        case'm':
        case'M':
            units = 1024 * 1024;
            break;
        case'g':
        case'G':
            units = 1024 * 1024 * 1024;
            break;
        default:
            options_error();
            break;
    }
    str[strlen(str) - 1] = 0;
    num = atol(str);
    return num * units;
}

static int parse_options_data_type(char *str)
{
    int num = atol(optarg);
    int type = -1;
    switch(num) {
        case 0:
            type = DATA_0;
            break;
        case 1:
            type = DATA_1;
            break;
        case 2:
            type = DATA_R;
            break;
        default:
            options_error();
            break;
    }

    return type;
}

static int parse_options_sync_type(char *str)
{
    int num = atol(optarg);
    int type = -1;

    switch(num) {
        case 0:
            type = SYNC_SYNC;
            break;
        case 1:
            type = SYNC_FSYNC;
            break;
        case 2:
            type = SYNC_FDATASYNC;
            break;
        case 3:
            type = SYNC_ODIRECT;
            break;
        case 4:
            type = SYNC_OSYNC;
            break;
        case 5:
            type = SYNC_ODIRSYNC;
            break;
        default:
            break;
    }

    return type;
}

static struct option iodisk_options[] = {
    {"read", 0, 0, 0},
    {"write", 0, 0, 1},
    {"mmap", 0, 0, 2},
    {"block_size", 1, 0, 3},
    {"data", 1, 0, 4},
    {"sync", 1, 0, 5},
    {"device", 0, 0, 6},
    {"file", 1, 0, 7},
    {"verbose", 0, 0, 8},
    {"rounds", 1, 0, 9},
    {"file_size", 1, 0, 10},
    {"random", 0, 0, 11},
    {"read_write", 0, 0, 12},
    {"alignment", 0, 0, 13},
    {"help", 0, 0, 20},
    {0, 0, 0, 0},
};

static void _dump_tctx(struct tctx *tctx_)
{
    printf("rw_type 0x%x\n", tctx_->rw_type);
    //printf("block_size "PRId64"\n", tctx_->block_size);
    printf("block_size 0x%x\n", tctx_->block_size);
    printf("data_type 0x%x\n", tctx_->data_type);
    printf("sync_type 0x%x\n", tctx_->sync_type);
    printf("use_mmap 0x%x\n", tctx_->use_mmap);
    printf("test_rounds 0x%x\n", tctx_->test_rounds);
    //printf("file_size "PRId64"\n", tctx_->file_size);
    printf("file_size 0x%x\n", tctx_->file_size);
    printf("verbose 0x%x\n", tctx_->verbose);
    printf("file %s\n", tctx_->file[0]);
    printf("file_num 0x%x\n", tctx_->file_num);
    printf("device %s\n", tctx_->device);
}

static int parse_options(struct tctx *tctx_, int argc, char *argv[])
{
    int ret          = 0;
    int opt          = -1;
    int opt_index    = -1;
    struct tctx *ctx = tctx_;

    while(-1 != (opt = getopt_long( argc, argv, "" ,iodisk_options, &opt_index))) {
        //DBG("opt %d index %d arg %s\n", opt, opt_index, optarg);
        switch( opt ) {
            case 0:
                ctx->rw_type |= TEST_READ;  
                break;
            case 1:
                ctx->rw_type |= TEST_WRITE;  
                break;
            case 2:
                ctx->use_mmap = 1;
                break;
            case 3:
                ctx->block_size = parse_options_bytes(optarg);
                break;
            case 4:
                ctx->data_type = parse_options_data_type(optarg);
                break;
            case 5:
                ctx->sync_type = parse_options_sync_type(optarg);
                break;
            case 6:
                snprintf(ctx->device, sizeof(ctx->device), "%s", optarg);
                break;
            case 7:
                snprintf(ctx->file[0], sizeof(ctx->file[0]), "%s", optarg);
                ctx->file_num = 1;
                break;
            case 8:
                ctx->verbose = 1;
                break;
            case 9:
                ctx->test_rounds = atoi(optarg);
                break;
            case 10:
                ctx->file_size = parse_options_bytes(optarg);
                break;
            case 11:
                ctx->random = 1;
                break;
            case 12:
                ctx->rw_type |= TEST_READ_WRITE;
                break;
            case 13:
                ctx->alignment = 1;
                break;
            case 21:
            default:
                usage();
                break;
        }
    }

    /* Adjust internal flags */
    if(ctx->random) {
        if(ctx->rw_type & TEST_READ) {
            ctx->rw_type &= ~TEST_READ;
            ctx->rw_type |= TEST_READ_RANDOM;
        }
        if(ctx->rw_type & TEST_WRITE) {
            ctx->rw_type &= ~TEST_WRITE;
            ctx->rw_type |= TEST_WRITE_RANDOM;
        }
    }

    return ret;
}
static int iodisk_record_start(struct tctx *tctx_, int file_no, int round)
{
    struct tctx *ctx = tctx_;
    int ret = 0;

    ret = clock_gettime(CLOCK_REALTIME, &ctx->start_ts[file_no][round]);

    return ret;
}

static int iodisk_record_stop(struct tctx *tctx_, int file_no, int round)
{
    struct tctx *ctx = tctx_;
    int ret = 0;

    ret = clock_gettime(CLOCK_REALTIME, &ctx->stop_ts[file_no][round]);

    return ret;
}

static int sync2(int fd)
{
    sync();
    return 0;
}

static int dummy_sync(int fd)
{
    return 0;
}

static int build_internal_data(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int open_sync_flag = 0;

    switch(ctx->sync_type) {
        case SYNC_SYNC:
            ctx->iodisk_sync = sync2;
            break;
        case SYNC_FSYNC:
            ctx->iodisk_sync = fsync;
            break;
        case SYNC_FDATASYNC:
            ctx->iodisk_sync = fdatasync;
            break;
        case SYNC_ODIRECT:
            ctx->open_sync_flag = O_DIRECT;
            ctx->iodisk_sync = dummy_sync;
            break;
        case SYNC_OSYNC:
            ctx->open_sync_flag = O_SYNC;
            ctx->iodisk_sync = dummy_sync;
            break;
        case SYNC_ODIRSYNC:
            ctx->open_sync_flag = O_DIRECT | O_SYNC;
            ctx->iodisk_sync = dummy_sync;
            break;
        default:
            ;
            //error("Error:wrong --sync\n");
    }
    if((ctx->rw_type & TEST_WRITE) &&
            NULL == ctx->iodisk_sync) {
        error("No sync type\n");
    }

    return ret;
}

static int clear_disk_cache()
{
    int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
    int ret = 0;
    if(fd < 0) {
        perror("Error open drop_caches");
        error("");
    }
    ret = write(fd, "3\n", 2);
    ret |= close(fd);

    return ret;
}

static int prepare_read(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int index = 0;

    clear_disk_cache();
    while(index< ctx->file_num) {
        ctx->fd[index] = open(ctx->file[index], O_RDONLY | ctx->open_sync_flag);
        if(ctx->fd[index] < 0) {
            error("Cannot open file %s\n",strerror(errno));        
        }
        else if(-1 == fstat(ctx->fd[index], &ctx->fd_st[index])) {
            error("Cannot stat file %s:%s\n", ctx->file[index], strerror(errno));        
        }

    #if IODISK_USE_MMAP
        if(ctx->use_mmap) {
//From open(2)
//Applications should avoid mixing O_DIRECT and normal I/O to the same file, and especially to overlap‐
//ping byte regions in the same file.  Even when the file system correctly handles the coherency issues
//in this situation, overall I/O throughput is likely to be slower than using either mode alone.  Like‐
//wise, applications should avoid mixing mmap(2) of files with direct I/O to the same files.
            if(ctx->open_sync_flag & O_DIRECT) {
                error("cannot mixing SYNC_ODIRECT with mmap\n");
            }
            else if (MAP_FAILED == (ctx->fd_addr[index] = 
                    mmap(NULL, ctx->fd_st[index].st_size, 
                    PROT_READ, MAP_PRIVATE, 
                    ctx->fd[index], 0))) {
                error("Cannot mmap file %s\n",strerror(errno));        
            }
        }
    #endif
        ++index;
    }

    return ret;
}

static int finish_read(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int index = 0;

    while(index< ctx->file_num) {
    #if IODISK_USE_MMAP
        if(ctx->use_mmap) {
            munmap(ctx->fd_addr[index], ctx->fd_st[index].st_size);
        }
    #endif
        close(ctx->fd[index]);
        ++index;
    }

    return ret;
}

static int prepare_write(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int index = 0;

    clear_disk_cache();
    while(index< ctx->file_num) {
        ctx->fd[index] = open(ctx->file[index], 
                            O_CREAT | O_WRONLY | 
                            O_TRUNC | ctx->open_sync_flag,
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(ctx->fd[index] < 0) {
            error("Cannot open file %s\n",strerror(errno));        
        }
        ++index;
    }

    return ret;
}

static int finish_write(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int index = 0;

    while(index< ctx->file_num) {
        close(ctx->fd[index]);
        ++index;
    }

    return ret;
}

static int prepare_read_write(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int index = 0;

    clear_disk_cache();
    while(index< ctx->file_num) {
        ctx->fd[index] = open(ctx->file[index], O_RDWR | ctx->open_sync_flag);
        if(ctx->fd[index] < 0) {
            error("Cannot open file %s\n",strerror(errno));        
        }
        else if(-1 == fstat(ctx->fd[index], &ctx->fd_st[index])) {
            error("Cannot stat file %s:%s\n", ctx->file[index], strerror(errno));        
        }

    #if IODISK_USE_MMAP
        if(ctx->use_mmap) {
//From open(2)
//Applications should avoid mixing O_DIRECT and normal I/O to the same file, and especially to overlap‐
//ping byte regions in the same file.  Even when the file system correctly handles the coherency issues
//in this situation, overall I/O throughput is likely to be slower than using either mode alone.  Like‐
//wise, applications should avoid mixing mmap(2) of files with direct I/O to the same files.
            if(ctx->open_sync_flag & O_DIRECT) {
                error("cannot mixing SYNC_ODIRECT with mmap\n");
            }
            else if (MAP_FAILED == (ctx->fd_addr[index] = 
                    mmap(NULL, ctx->fd_st[index].st_size, 
                    PROT_READ, MAP_PRIVATE, 
                    ctx->fd[index], 0))) {
                error("Cannot mmap file %s\n",strerror(errno));        
            }
        }
    #endif
        ++index;
    }

    return ret;
}

static int finish_read_write(struct tctx *tctx_)
{
    return finish_read(tctx_);
}

static int iodisk_timed_read(struct tctx *tctx_, int file_no)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    ssize_t total_size = 0, size = 0;
    char buf[ctx->block_size];
    //char *buf = NULL;
    long buf_size = ctx->block_size;
    //char *content = NULL;

    //prepare_read(ctx);
    //content = (char *)malloc(ctx->fd_st[file_no].st_size * sizeof(char));
    //assert(content);
    //finish_read(ctx);

    //DBG("sizeof buf %d %ld %p %p\n", sizeof(buf), ctx->fd_st[file_no].st_size, buf, content);
    while(r < ctx->test_rounds) {
        total_size = size = 0;
        prepare_read(ctx);

        iodisk_record_start(ctx, file_no, r);
        #if IODISK_USE_MMAP
        if(ctx->use_mmap) {
            while(size >= 0 && total_size < ctx->fd_st[file_no].st_size) {
                size = ctx->fd_st[file_no].st_size - total_size;
                size = size >= ctx->block_size ? ctx->block_size : size;
                memcpy(buf, ctx->fd_addr[file_no] + total_size, size);
                total_size += size;
                ++ctx->fd_iops[file_no][r];
            }
        }
        else {
        #endif
            size = 1;
            while(size > 0 && total_size < ctx->fd_st[file_no].st_size) {
                //size = read(ctx->fd[file_no], buf, sizeof(buf));
                size = read(ctx->fd[file_no], buf, buf_size);
                //assert(size == buf_size);

                total_size += size;
                ++ctx->fd_iops[file_no][r];
            }
        #if IODISK_USE_MMAP
        }
        #endif

        iodisk_record_stop(ctx, file_no, r);
        finish_read(ctx);

        ++r;
    }

    ret = total_size == ctx->fd_st[file_no].st_size ? 0 : -1;

    return ret;
}

static int iodisk_timed_write(struct tctx *tctx_, int file_no)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    ssize_t total_size = 0, size = 0;
    size_t buf_size, alignment;
    char *buf = NULL;

    if(ctx->sync_type == SYNC_ODIRECT || ctx->sync_type == SYNC_ODIRSYNC) {
        alignment = fpathconf(ctx->fd[file_no], _PC_REC_XFER_ALIGN);
        if(ctx->block_size % alignment) {
            buf_size = ctx->block_size - ctx->block_size % buf_size;
            buf_size = buf_size ? buf_size : alignment;
            DBG("block_size adjust to 0x%x\n", buf_size);
        }
        else {
            buf_size = ctx->block_size;
        }
        assert(0 == posix_memalign((void **)&buf, alignment, buf_size));
    }
    else {
        buf_size = ctx->block_size;
        buf = malloc(sizeof(char) * buf_size);
    }

    /* Preparing data to be written */
    switch(ctx->data_type) {
        case DATA_0:
            memset(buf, 0x0, buf_size);
            break;
        case DATA_1:
            memset(buf, 0xff, buf_size);
            break;
        case DATA_R:
            {
                int i = 0;
                unsigned int seed = 0;
                while(i < buf_size) {
                    if(0 == seed) {
                        seed = time(0);
                    }
                    else {
                        --seed;
                    }
                    srandom(seed);
                    buf[i] = random() % 0xff;
                    ++i;
                }
            }
            break;
        default:
            error("data_type error\n");
            break;
    }

    while(r < ctx->test_rounds) {
        total_size = size = 0;
        prepare_write(ctx);
        
        iodisk_record_start(ctx, file_no, r);
        size = 1;
        while(size > 0 && total_size < ctx->file_size) {
            //DBG("total_size %d\n", total_size);
            ++ctx->fd_iops[file_no][r];
            size = write(ctx->fd[file_no], buf, buf_size);
            total_size += size;
        }
        ctx->iodisk_sync(ctx->fd[file_no]);
        iodisk_record_stop(ctx, file_no, r);
        finish_write(ctx);

        ++r;
    }
    //DBG("[WRITE]file %s total_size %d:%d\n", ctx->file[file_no], total_size, ctx->file_size);
    free(buf);
    ret = total_size == ctx->fd_st[file_no].st_size ? 0 : -1;

    return ret;
}

static int iodisk_timed_write_random(struct tctx *tctx_, int file_no)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    ssize_t total_size = 0, size = 0;
    //char buf[ctx->block_size];

    //if(ctx->sync_type == SYNC_ODIRECT ||
        //ctx->sync_type == SYNC_ODIRSYNC) {
        //error("Error:mixing O_DIRECT with lseek\n");
    //}
    size_t buf_size, alignment;
    char *buf = NULL;

    if(ctx->sync_type == SYNC_ODIRECT || ctx->sync_type == SYNC_ODIRSYNC) {
        alignment = fpathconf(ctx->fd[file_no], _PC_REC_XFER_ALIGN);
        if(ctx->block_size % alignment) {
            buf_size = ctx->block_size - ctx->block_size % buf_size;
            buf_size = buf_size ? buf_size : alignment;
            DBG("block_size adjust to 0x%x\n", buf_size);
        }
        else {
            buf_size = ctx->block_size;
        }
        assert(0 == posix_memalign((void **)&buf, alignment, buf_size));
    }
    else {
        buf_size = ctx->block_size;
        buf = malloc(sizeof(char) * buf_size);
    }
    ctx->block_size = buf_size;

    iodisk_printf(ctx, "Preparing ... ");

    /* Preparing data to be written */
    switch(ctx->data_type) {
        case DATA_0:
            memset(buf, 0x0, buf_size * sizeof(char));
            break;
        case DATA_1:
            memset(buf, 0xff, buf_size * sizeof(char));
            break;
        case DATA_R:
            {
                int i = 0;
                unsigned int seed = 0;
                while(i < buf_size) {
                    if(0 == seed) {
                        seed = time(0);
                    }
                    else {
                        --seed;
                    }
                    srandom(seed);
                    buf[i] = random() % 0xff;
                    ++i;
                }
            }
            break;
        default:
            error("data_type error\n");
            break;
    }

    /* Preparing random write offset */
    int p_num = ctx->file_size / ctx->block_size;
    if(ctx->file_size % ctx->block_size) {
        ++p_num;
    }
    int i = 0;
    unsigned int seed = 0;
    //long int position[p_num];
    //long int position_status[p_num];
    long int *position;
    long int *position_status;
    position = (long int *)malloc(sizeof(long int) * p_num);
    position_status = (long int *)malloc(sizeof(long int) * p_num);
    //memset(position_status, 0, sizeof(position_status));
    memset(position_status, 0, sizeof(sizeof(long int) * p_num));
    while(i < p_num) {
        while (1) {
            int r = 0;
            if(0 == seed) {
                seed = time(0);
            }
            else {
                --seed;
            }
            srandom(seed);
            r = random() % p_num;
            if(position_status[r]) {
                continue;
            }
            else {
                position_status[r] = 1;
            }
            if(ctx->alignment) {
                position[i] = r * ctx->block_size;
            }
            else {
                //position[i] = r * ctx->block_size + ctx->block_size / 2;
                //TODO
                position[i] = r * ctx->block_size;
            }
            //if(position[i] > ctx->file_size - ctx->block_size) {
                //position[i] = ctx->file_size - ctx->block_size;
            //}
            //position[i] &= ~(ctx->block_size - 1);
            break;
        }

        ++i;
    }
    iodisk_printf(ctx, "ok.\n");

    while(r < ctx->test_rounds) {
        total_size = size = 0;
        i = 0;
        prepare_write(ctx);
        
        iodisk_record_start(ctx, file_no, r);
        while(i < p_num) {
            //DBG("position %ld \n", position[i]);
            ret = lseek(ctx->fd[file_no], position[i], SEEK_SET);
            assert(ret != -1);
            size = write(ctx->fd[file_no], buf, buf_size * sizeof(char));
            ++i;
            ++ctx->fd_iops[file_no][r];
        }
        ctx->iodisk_sync(ctx->fd[file_no]);
        iodisk_record_stop(ctx, file_no, r);
        finish_write(ctx);

        ++r;
    }
    //DBG("[WRITE]file %s total_size %d:%d\n", ctx->file[file_no], total_size, ctx->file_size);
    //ret = total_size == ctx->fd_st[file_no].st_size ? 0 : -1;
    free(position);
    free(position_status);

    return ret;
}

static int iodisk_timed_read_random(struct tctx *tctx_, int file_no)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    ssize_t total_size = 0, size = 0;
    char buf[ctx->block_size];

    iodisk_printf(ctx, "Preparing ... ");
    /* Preparing random read offset */
    prepare_read(ctx);
    int p_num = ctx->fd_st[file_no].st_size / ctx->block_size;
    if(ctx->fd_st[file_no].st_size % ctx->block_size) {
        ++p_num;
    }
    int i = 0;
    unsigned int seed = 0;
    //long int position[p_num];
    //long int position_status[p_num];
    //memset(position_status, 0, sizeof(position_status));
    long int *position;
    long int *position_status;
    position = (long int *)malloc(sizeof(long int) * p_num);
    position_status = (long int *)malloc(sizeof(long int) * p_num);
    //memset(position_status, 0, sizeof(position_status));
    memset(position_status, 0, sizeof(sizeof(long int) * p_num));
    while(i < p_num) {
        while (1) {
            int r = 0;
            if(0 == seed) {
                seed = time(0);
            }
            else {
                --seed;
            }
            srandom(seed);
            r = random() % p_num;
            if(position_status[r]) {
                continue;
            }
            else {
                position_status[r] = 1;
            }
            if(ctx->alignment) {
                position[i] = r * ctx->block_size;
            }
            else {
                //position[i] = r * ctx->block_size + ctx->block_size / 2;
                //TODO
                position[i] = r * ctx->block_size;
            }
            break;
        }

        ++i;
    }
    finish_read(ctx);
    iodisk_printf(ctx, "ok.\n");

    while(r < ctx->test_rounds) {
        i = 0;
        prepare_read(ctx);
        
        iodisk_record_start(ctx, file_no, r);
        #if IODISK_USE_MMAP
        if(ctx->use_mmap) {
            while(i < p_num) {
                memcpy(buf, ctx->fd_addr[file_no] + position[i], ctx->block_size);
                ++i;
                ++ctx->fd_iops[file_no][r];
            }
        }
        else {
        #endif
            size = 1;
            while(i < p_num) {
                //DBG("position %ld\n", position[i]);
                ret = lseek(ctx->fd[file_no], position[i], SEEK_SET);
                assert(ret != -1);
                size = read(ctx->fd[file_no], buf, sizeof(buf));
                ++i;
                ++ctx->fd_iops[file_no][r];
            }
        #if IODISK_USE_MMAP
        }
        #endif

        iodisk_record_stop(ctx, file_no, r);
        finish_read(ctx);

        ++r;
    }
    //DBG("[READ]file %s total_size %d:%ld\n", ctx->file[file_no], 
                        //total_size, ctx->fd_st[file_no].st_size);
    free(position);
    free(position_status);
    ret = total_size == ctx->fd_st[file_no].st_size ? 0 : -1;

    return ret;
}

static int iodisk_timed_read_write(struct tctx *tctx_, int file_no)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    ssize_t total_size = 0, size = 0;
    char buf[ctx->block_size];

    /* Preparing random read offset */
    iodisk_printf(ctx, "Preparing ... ");

    switch(ctx->data_type) {
        case DATA_0:
            memset(buf, 0x0, sizeof(buf));
            break;
        case DATA_1:
            memset(buf, 0xff, sizeof(buf));
            break;
        case DATA_R:
            {
                int i = 0;
                unsigned int seed = 0;
                while(i < ctx->block_size) {
                    if(0 == seed) {
                        seed = time(0);
                    }
                    else {
                        --seed;
                    }
                    srandom(seed);
                    buf[i] = random() % 0xff;
                    ++i;
                }
            }
            break;
        default:
            error("data_type error\n");
            break;
    }

    prepare_read(ctx);
    int p_num = ctx->fd_st[file_no].st_size / ctx->block_size;
    if(ctx->file_size % ctx->block_size) {
        ++p_num;
    }
    int i = 0;
    unsigned int seed = 0;
    //long int position[p_num];
    //long int position_status[p_num];
    //memset(position_status, 0, sizeof(position_status));
    long int *position;
    long int *position_status;
    position = (long int *)malloc(sizeof(long int) * p_num);
    position_status = (long int *)malloc(sizeof(long int) * p_num);
    //memset(position_status, 0, sizeof(position_status));
    memset(position_status, 0, sizeof(sizeof(long int) * p_num));
    while(i < p_num) {
        while (1) {
            int r = 0;
            if(0 == seed) {
                seed = time(0);
            }
            else {
                --seed;
            }
            srandom(seed);
            r = random() % p_num;
            if(position_status[r]) {
                continue;
            }
            else {
                position_status[r] = 1;
            }
            if(ctx->alignment) {
                position[i] = r * ctx->block_size;
            }
            else {
                //position[i] = r * ctx->block_size + ctx->block_size / 2;
                //TODO
                position[i] = r * ctx->block_size;
            }
            break;
        }

        ++i;
    }
    finish_read(ctx);
    iodisk_printf(ctx, "ok.\n");

    /* randomly read & write,
    * and random file offset
    */
    while(r < ctx->test_rounds) {
        i = 0;
        size = total_size = 0;
        prepare_read_write(ctx);
        
        iodisk_record_start(ctx, file_no, r);

        while(i < p_num) {
            //DBG("%d\n",i);
            /* use position to determine 
            * whether reading or writing,
            * and read & write sequentially in the whole
            */
            //DBG("position %ld\n", position[i]);
            //if(i % 2) { /* Read */
            if((position[i] / ctx->block_size) % 2) { /* Read */
            #if IODISK_USE_MMAP
                if(ctx->use_mmap) {
                    //size = ctx->fd_st[file_no].st_size - total_size;
                    size = ctx->fd_st[file_no].st_size - position[i]
                    size = size >= ctx->block_size ? ctx->block_size : size;
                    //memcpy(buf, ctx->fd_addr[file_no] + total_size, size);
                    memcpy(buf, ctx->fd_addr[file_no] + position[i], size);
                }
                else {
            #endif
                    //ret = lseek(ctx->fd[file_no], total_size, SEEK_SET);
                    ret = lseek(ctx->fd[file_no], position[i], SEEK_SET);
                    assert(ret != -1);
                    size = read(ctx->fd[file_no], buf, sizeof(buf));

            #if IODISK_USE_MMAP
                }
            #endif
            }
            else { /* Write */
                //DBG("position %ld \n", position[i]);
                //ret = lseek(ctx->fd[file_no], total_size, SEEK_SET);
                ret = lseek(ctx->fd[file_no], position[i], SEEK_SET);
                assert(ret != -1);
                size = write(ctx->fd[file_no], buf, sizeof(buf));
            }

            total_size += size;
            ++i;
            ++ctx->fd_iops[file_no][r];
        }

        iodisk_record_stop(ctx, file_no, r);
        finish_read_write(ctx);

        ++r;
    }
    //DBG("[READ]file %s total_size %d:%ld\n", ctx->file[file_no], 
                        //total_size, ctx->fd_st[file_no].st_size);
    free(position);
    free(position_status);
    ret = total_size == ctx->fd_st[file_no].st_size ? 0 : -1;

    return ret;
}

static int iodisk_timed_report_round(struct tctx *tctx_, int file_no, int type)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int r = 0;
    double speed, duration;
    char *desp[TEST_TYPE_MAX + 1] = {
                [TEST_READ] = "Read" ,
                [TEST_WRITE] = "Write" ,
                [TEST_READ_RANDOM] = "Read Randomly", 
                [TEST_WRITE_RANDOM] = "Write Randomly", 
                [TEST_READ_WRITE] = "Read & Write", 
            };

    //iodisk_printf(ctx, "file %s:\nRead speed\t", ctx->file[file_no]);
    iodisk_printf(ctx, "%s \n", desp[type]);
    iodisk_printf(ctx, "Speed\t", desp[type]);

    for(r = 0; r < ctx->test_rounds; ++r) {
        duration = ctx->stop_ts[file_no][r].tv_nsec - ctx->start_ts[file_no][r].tv_nsec;
        duration = ctx->stop_ts[file_no][r].tv_sec - ctx->start_ts[file_no][r].tv_sec + duration * 0.000000001;
        //DBG("start %ld:%09ld\n", ctx->start_ts[file_no][r].tv_sec, ctx->start_ts[file_no][r].tv_nsec);
        //DBG("stop %ld:%09ld\n", ctx->stop_ts[file_no][r].tv_sec, ctx->stop_ts[file_no][r].tv_nsec);
        //DBG("duration %lf\n", duration);
        if(type & TEST_READ ||
            type & TEST_READ_RANDOM ||
            type & TEST_READ_WRITE) {
            speed = (double)ctx->fd_st[file_no].st_size / duration;
        }
        else if(type & TEST_WRITE ||
            type & TEST_WRITE_RANDOM) {
            speed = (double)ctx->file_size / duration;
        }
        //else if(type & TEST_READ_WRITE ) {
            //speed = (double)ctx->fd_st[file_no].st_size / duration;
        //}
        ctx->speed[file_no][r] = speed;
        speed /= (1024 * 1024);
        iodisk_printf(ctx, "%.2f  ", speed);

        ctx->fd_iops[file_no][r] /= duration;
        //DBG("\niops3 %d\n", (int)round(ctx->fd_iops[file_no][r]));
        //DBG("\niops3 %.0f\n", round(ctx->fd_iops[file_no][r]));
    }

    iodisk_printf(ctx, "\n");
    iodisk_printf(ctx, "IOPS\t");
    for(r = 0; r < ctx->test_rounds; ++r) {
        iodisk_printf(ctx, "%d  ", (int)round(ctx->fd_iops[file_no][r]));
    }
    iodisk_printf(ctx, "\n");

    return ret;
}

static int iodisk_timed_report_all_avg(struct tctx *tctx_, int type)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    double avg = 0;
    double avg_iops = 0;
    int file_no = 0;
    int r = 0;
    char *desp[TEST_TYPE_MAX + 1] = {
                [TEST_READ] = "Read" ,
                [TEST_WRITE] = "Write" ,
                [TEST_READ_RANDOM] = "Read Randomly", 
                [TEST_WRITE_RANDOM] = "Write Randomly", 
                [TEST_READ_WRITE] = "Read & Write", 
            };

    //iodisk_printf(ctx, "%s speed avg\t", desp[type]);
    iodisk_printf(ctx, "speed avg\t");


    while(file_no < ctx->file_num) {
        for(r = 0; r < ctx->test_rounds; ++r) {
            avg += ctx->speed[file_no][r];
            avg_iops += ctx->fd_iops[file_no][r];
        }
        ++file_no;
    }

    avg /= ctx->test_rounds;
    avg /= (1024 * 1024);
    iodisk_printf(ctx, "%.2f  ", avg);

    iodisk_printf(ctx, "\n");
    avg_iops /= ctx->test_rounds;
    iodisk_printf(ctx, "IOPS avg\t");
    iodisk_printf(ctx, "%d  ", (int)round(avg_iops));
    iodisk_printf(ctx, "\n");


    return ret;
}

static int start_test_read(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int file_no = 0;


    while(file_no < ctx->file_num) {
        iodisk_timed_read(ctx, file_no);
        iodisk_timed_report_round(ctx, file_no, TEST_READ);
        ++file_no;
    }
    iodisk_timed_report_all_avg(ctx, TEST_READ);


    return ret;
}

static int start_test_write(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int file_no = 0;

    while(file_no < ctx->file_num) {
        iodisk_timed_write(ctx, file_no);
        iodisk_timed_report_round(ctx, file_no, TEST_WRITE);
        ++file_no;
    }
    iodisk_timed_report_all_avg(ctx, TEST_WRITE);

    return ret;
}

static int start_test_write_random(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int file_no = 0;

    while(file_no < ctx->file_num) {
        iodisk_timed_write_random(ctx, file_no);
        iodisk_timed_report_round(ctx, file_no, TEST_WRITE_RANDOM);
        ++file_no;
    }
    iodisk_timed_report_all_avg(ctx, TEST_WRITE_RANDOM);

    return ret;
}

static int start_test_read_random(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int file_no = 0;


    while(file_no < ctx->file_num) {
        iodisk_timed_read_random(ctx, file_no);
        iodisk_timed_report_round(ctx, file_no, TEST_READ_RANDOM);
        ++file_no;
    }
    iodisk_timed_report_all_avg(ctx, TEST_READ_RANDOM);

    return ret;
}

static int start_test_read_write(struct tctx *tctx_)
{
    struct tctx *ctx = tctx_;
    int ret = 0;
    int file_no = 0;

    while(file_no < ctx->file_num) {
        iodisk_timed_read_write(ctx, file_no);
        iodisk_timed_report_round(ctx, file_no, TEST_READ_WRITE);
        ++file_no;
    }
    iodisk_timed_report_all_avg(ctx, TEST_READ_WRITE);

    return ret;
}

static int start_test(struct tctx *tctx_)
{
    int ret = 0;
    struct tctx *ctx = tctx_;

    /* perform write test first,
    * for no file case
    */
    if(ctx->rw_type & TEST_WRITE) {
        ret |= start_test_write(tctx_);
    }
    if(ctx->rw_type & TEST_READ) {
        ret |= start_test_read(tctx_);
    }
    if(ctx->rw_type & TEST_WRITE_RANDOM) {
        ret |= start_test_write_random(tctx_);
    }
    if(ctx->rw_type & TEST_READ_RANDOM) {
        ret |= start_test_read_random(tctx_);
    }
    if(ctx->rw_type & TEST_READ_WRITE) {
        ret |= start_test_read_write(tctx_);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    struct tctx tctx;

    memset(&tctx, 0, sizeof(struct tctx));

    ret = parse_options(&tctx, argc, argv);
    //_dump_tctx(&tctx);

    ret |= build_internal_data(&tctx);

    ret |= start_test(&tctx);

    return ret;
}
