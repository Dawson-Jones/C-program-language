#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define MAXEVENTS 64

typedef struct conn_s conn_t;
struct conn_s {
    int fd;
    SSL *ssl;
};

static void make_socket_non_blocking(int sfd) {
    int flags, s;

    // file control
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        exit(EXIT_FAILURE);
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl set");
        exit(EXIT_FAILURE);
    }
}

static int create_and_bind(const char *port) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    printf("port: %s\n", port);

    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break; /* Success */
        }

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        return -1;
    }
    freeaddrinfo(result);

    // 使用 sfd 通信
    return sfd;
}

SSL_CTX *init_ssl_ctx() {
    SSL_CTX *ctx;

    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD *method;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        perror("Unable to create SSL context");
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        perror("Unable to set certificate");
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "cert.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        perror("Unable to set private key");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

int ssl_handshake(SSL *ssl) {
    int n, sslerr;

    n = SSL_do_handshake(ssl);
    if (n == 1) {
        printf("ssl connection established\n");
        return 1;
    } else {
        sslerr = SSL_get_error(ssl, n);
        switch (sslerr) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            return EAGAIN;
        default:
            ERR_print_errors_fp(stderr);
            perror("ssl handshake");
            exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char const *argv[]) {
    int sfd, s;
    int efd;

    struct epoll_event event;
    struct epoll_event *events;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = create_and_bind(argv[1]);
    if (sfd == -1)
        abort();
    
    make_socket_non_blocking(sfd);

    if (listen(sfd, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll create");
        exit(EXIT_FAILURE);
    }

    conn_t *conn = (conn_t *) malloc(sizeof(conn_t));
    conn->fd = sfd;
    event.data.ptr = conn;
    // EPOLLET ，表示使用边沿触发模式（Edge-Triggered，ET）的事件通知模式
    // ET: 当文件描述符上有新的数据可读或者可写时，只会触发一次事件通知。
    // 如果不立即读取或写入数据，后续的事件将不会被触发。
    // 因此，需要确保及时读取或写入数据，以保证后续的事件能够被及时触发。
    // 对应的是水平触发模式（Level-Triggered，LT），
    // 当文件描述符上有新的数据可读或者可写时，会不断触发事件通知，
    // 直到数据被读取或者写入。
    // 默认情况下，epoll 函数使用 LT 模式，通过设置 EPOLLET 标志位，可以将其切换为 ET 模式。
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    // `MAXEVENTS` 的值是限制每次调用 `epoll_wait` 函数时可以处理的事件数量上限。
    // 不是限制整个应用程序的并发连接数量。
    // 如果客户端连接的数量远远大于这个值，那么在多次调用 `epoll_wait` 时会分批处理这些连接。
    // 只要内存允许，epoll 是可以处理大量的客户端连接的。
    events = calloc(MAXEVENTS, sizeof(event));

    SSL_CTX *ctx = init_ssl_ctx();
    while (1) {
        int n, i, fd;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; ++i) {
            fd = ((conn_t *) events[i].data.ptr)->fd;
            if (events[i].events & EPOLLERR ||
                events[i].events & EPOLLHUP ||
                !(events[i].events & EPOLLIN)) {
                if (events[i].events & EPOLLERR)
                    fprintf(stderr, "epoll error: EPOLLERR\n");
                if (events[i].events & EPOLLHUP)
                    fprintf(stderr, "epoll error: EPOLLHUP\n");
                close(fd);
                free(events[i].data.ptr);
                continue;
            } else if (sfd == fd) { // new socket
                // 可能这次通知有多个新的连接
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept(fd, &in_addr, &in_len);
                    if (infd == -1) {
                        /* We have processed all incoming connections. */
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    s = getnameinfo(&in_addr, in_len, 
                        hbuf, sizeof(hbuf), 
                        sbuf, sizeof(sbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV
                    );
                    if (s == 0) {
                        printf("Accepted connection on descriptor %d "
                            "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                    // 将新的 fd 添加到 epoll 中
                    make_socket_non_blocking(infd);
                    conn_t *conn = (conn_t *) malloc(sizeof(conn_t));
                    conn->fd = infd;

                    SSL *ssl = SSL_new(ctx);
                    SSL_set_accept_state(ssl);
                    SSL_set_fd(ssl, infd);
                    conn->ssl = ssl;

                    event.data.ptr = conn;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        perror("epoll_ctl");
                        exit(EXIT_FAILURE);
                    }
                }
            } else {    // EPOLL_IN and not listen fd
                /* ssl handshake */
                conn_t *cur_conn = events[i].data.ptr;
                SSL *ssl = cur_conn->ssl;
                if (!SSL_is_init_finished(ssl)) {
                    ssl_handshake(ssl);
                    continue;
                }

                int done = 0;
                int sslerr;
                ssize_t count;
                char buf[512];

                count = SSL_read(ssl, buf, sizeof(buf));
                if (count > 0) {
                    count = SSL_write(ssl, buf, count);
                } else if (count < 0) {
                    sslerr = SSL_get_error(ssl, count);
                    if (sslerr != SSL_ERROR_WANT_READ && sslerr != SSL_ERROR_WANT_WRITE) {
                        ERR_print_errors_fp(stderr);
                        done = 1;
                    }
                } else /*(count == 0)*/ {
                    done = 1;
                }

                if (done) {
                    printf("Close connection on descriptor %d\n", fd);
                    // 当关闭一个文件描述符, 会自动的从 epoll 实例中删除
                    // epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                    SSL_free(ssl);
                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    close(fd);
                    free(events[i].data.ptr); 
                }
            }
        }
    }

    // clean
    free(events);
    // epoll_ctl(efd, EPOLL_CTL_DEL, sfd, NULL);
    close(sfd);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}
