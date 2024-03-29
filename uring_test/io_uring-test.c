#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <liburing.h>


#define QD 4

int main(int argc, char const *argv[])
{
    struct io_uring ring;
    int i, fd, ret, pending, done;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    struct iovec *iovecs;
    struct stat sb;
    ssize_t fsize;
    off_t offset;
    void *buf;

    if (argc < 2) {
        printf("%s: file\n", argv[0]);
        return 1;
    }

    ret = io_uring_queue_init(QD, &ring, 0);
    if (ret < 0) {
        fprintf(stderr, "queue init: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }

    fd = open(argv[1], O_RDONLY | O_DIRECT);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    if (fstat(fd, &sb) < 0) {
        perror("fstat");
        return EXIT_FAILURE;
    }

    // fsize = 0;
    iovecs = calloc(QD, sizeof(struct iovec));
    for (i = 0; i < QD; ++i) {
        if (posix_memalign(&buf, 4096, 4096))
            return 1;
        
        iovecs[i].iov_base = buf;
        iovecs[i].iov_len = 4096;
        // fsize += 4096;
    }

    offset = 0;
    i = 0;
    do {
        sqe = io_uring_get_sqe(&ring);
        if (!sqe)
            break;
        
        io_uring_prep_readv(sqe, fd, &iovecs[i], 1, offset);
        offset += iovecs[i].iov_len;
        i++;
        if (offset > sb.st_size)
            break;
    } while (1);

    ret = io_uring_submit(&ring);
    if (ret < 0) {
        fprintf(stderr, "io_uring_submit: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    } else if (ret != i) {
        fprintf(stderr, "io_uring_submit submitted less %d\n", ret);
        return 1;
    }

    done = 0;
    pending = ret;
    fsize = 0;
    for (i = 0; i < pending; ++i) {
        ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-ret));
            return 1;
        }

        done++;
        ret = 0;
        if (cqe->res != 4096 && cqe->res + fsize != sb.st_size) {
            fprintf(stderr, "ret = %d, wanted 4096", cqe->res);
            ret = 1;
        }
        fsize += cqe->res;
        io_uring_cqe_seen(&ring, cqe);
        if (ret)
            break;
    }

    printf("Submitted=%d, completed=%d, bytes=%lu\n", pending, done, fsize);
    close(fd);
    io_uring_queue_exit(&ring);
    return 0;
}
