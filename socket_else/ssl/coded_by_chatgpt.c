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
#define CERT_FILE "cert.pem"
#define KEY_FILE "cert.key"

typedef struct conn_s conn_t;
struct conn_s {
    int fd;
    SSL *ssl;
};

static void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void make_socket_non_blocking(int sfd) {
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        error_exit("fcntl get");
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        error_exit("fcntl set");
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

    return sfd;
}

SSL_CTX *init_ssl_ctx(const char *cert_file, const char *key_file) {
    SSL_CTX *ctx;

    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD *method;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        error_exit("Unable to create SSL context");
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        error_exit("Unable to set certificate");
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        error_exit("Unable to set private key");
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
            error_exit("ssl handshake");
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
        error_exit("listen");
    }

    efd = epoll_create1(0);
    if (efd == -1) {
        error_exit("epoll create");
    }

    conn_t *conn = (conn_t *) malloc(sizeof(conn_t));
    conn->fd = sfd;
    event.data.ptr = conn;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        error_exit("epoll_ctl");
    }

    events = calloc(MAXEVENTS, sizeof(event));

    SSL_CTX *ctx = init_ssl_ctx(CERT_FILE, KEY_FILE);
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
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept(fd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            error_exit("accept");
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

                    make_socket_non_blocking(infd);

                    conn_t *new_conn = (conn_t *) malloc(sizeof(conn_t));
                    new_conn->fd = infd;
                    new_conn->ssl = SSL_new(ctx);
                    if (new_conn->ssl == NULL) {
                        ERR_print_errors_fp(stderr);
                        error_exit("SSL_new");
                    }
                    SSL_set_fd(new_conn->ssl, infd);
                    SSL_set_accept_state(new_conn->ssl);
                    event.data.ptr = new_conn;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        error_exit("epoll_ctl");
                    }
                }
            } else { // data from socket
                SSL *ssl = ((conn_t *) events[i].data.ptr)->ssl;
                if (!SSL_is_init_finished(ssl)) {
                    ssl_handshake(ssl);
                    continue;
                }

                ssize_t count;
                char buf[512];

                while (1) {
                    count = SSL_read(ssl, buf, sizeof(buf));
                    if (count == -1) {
                        if (errno != EAGAIN) {
                            error_exit("read");
                        }
                        break;
                    } else if (count == 0) {
                        break;
                    }

                    printf("Data: %.*s\n", (int) count, buf);
                }

                if (count == 0) {
                    printf("Closing connection on descriptor %d\n", fd);
                    close(fd);
                    SSL_free(ssl);
                    free(events[i].data.ptr);
                }
            }
        }
    }

    free(events);
    close(sfd);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    EVP_cleanup();
    return EXIT_SUCCESS;
}
