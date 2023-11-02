#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
#include <fcntl.h>
#include "readv_utils.h"

#define QUEUE_DEPTH 1
#define BLOCK_SZ    1024


struct file_info {
    off_t file_sz;
    struct iovec iovecs[];
};

int get_completion_and_print(struct io_uring *ring) {
    struct io_uring_cqe *cqe;
    int ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }
    if (cqe->res < 0) {
        fprintf(stderr, "Async readv failed.\n");
        return 1;
    }

    struct file_info *fi = io_uring_cqe_get_data(cqe);
    int blocks = (int) fi->file_sz / BLOCK_SZ;
    if (fi->file_sz % BLOCK_SZ)
        blocks++;
    
    for (int i = 0; i < blocks; ++i) {
        output_to_console(fi->iovecs[i].iov_base, fi->iovecs[i].iov_len);
    }

    io_uring_cqe_seen(ring, cqe);
    return 0;
}

int submit_read_request(const char *filename, struct io_uring *ring) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        return 1;
    }

    off_t file_sz = get_file_size(file_fd);
    off_t bytes_remaining = file_sz;
    int current_block = 0;
    int blocks = (int) file_sz / BLOCK_SZ;
    if (file_sz % BLOCK_SZ)
        blocks++;
    struct file_info *fi = malloc(sizeof(struct file_info) + 
        blocks * sizeof(struct iovec));
    
    off_t offset = 0;
    while (bytes_remaining) {
        off_t byte_to_read = bytes_remaining;
        if (byte_to_read > BLOCK_SZ) {
            byte_to_read = BLOCK_SZ;
        }

        offset += byte_to_read;
        fi->iovecs[current_block].iov_len = byte_to_read;
        fi->iovecs[current_block].iov_base = malloc(BLOCK_SZ);
        // void *buf;
        // if (posix_memalign(&buf, BLOCK_SZ, BLOCK_SZ)) {
        //     perror("posix_memalign");
        //     return 1;
        // }
        // fi->iovecs[current_block].iov_base = buf;
        current_block++;
        bytes_remaining -= byte_to_read;
    }
    fi->file_sz = file_sz;

    // get an sqe
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    // setup a readv operation
    io_uring_prep_readv(sqe, file_fd, fi->iovecs, blocks, 0);
    // set user data
    io_uring_sqe_set_data(sqe, fi);
    // submit the request
    io_uring_submit(ring);

    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [file name] <[file name...]>\n", argv[0]);
        return 1;
    }

    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    for (int i = 1; i < argc; ++i) {
        int ret = submit_read_request(argv[i], &ring);
        if (ret) {
            fprintf(stderr, "Error reading file: %s\n", argv[i]);
            return 1;
        }

        get_completion_and_print(&ring);
    }
    
    io_uring_queue_exit(&ring);
    return 0;
}
