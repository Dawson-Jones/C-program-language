#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
#include <sys/stat.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>

#define SERVER_STRING       "Server: simplehttpd/0.1\r\n"
#define DEFAULT_SERVER_PORT 8000
#define QUEUE_DEPTH         256
#define READ_SZ             4096


enum event {
    EVENT_TYPE_ACCEPT,
    EVENT_TYPE_READ_SOCK,
    EVENT_TYPE_READ_FILE,
    EVENT_TYPE_WRITE,
};

struct context {
    enum event event_type;
    int iovec_count;
    int client_socket;
    int path_fd;
    off_t file_size;
    struct iovec iov[];
};

struct io_uring ring;

const char *unimplemented_content = \
    "HTTP/1.1 400 Bad Request\r\n"
    "Content-type: text/html\r\n"
    "\r\n"
    "<html> <head><title>SimpleHTTPd: Unimplemented</title></head>"
    "<body>"
    "<h1>Bad Request (Unimplemented)</h1>"
    "<p>Your client sent a request SimpleHTTPd did not understand and it is probably not your fault.</p>"
    "</body>"
    "</html>";


const char *http_404_content = \
    "HTTP/1.1 404 Not Found\r\n"
    "Content-type: text/html\r\n"
    "\r\n"
    "<html> <head><title>SimpleHTTPd: Not Found</title></head>"
    "<body>"
    "<h1>Not Found</h1>"
    "<p>Your client is asking for an object that was not found on this server.</p>"
    "</body>"
    "</html>";


void strtolower(char *str) {
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}


void fatal_error(const char *syscall) {
    perror(syscall);
    exit(EXIT_FAILURE);
}


void *zh_malloc(size_t size) {
    void *buf = malloc(size);
    if (!buf) 
        fatal_error("malloc");

    return buf;
}


int setup_listening_socket(int port) {
    int sock;
    struct sockaddr_in srv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        fatal_error("socket");
    
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        fatal_error("setsockopt(SO_REUSEADDR)");
    
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
        fatal_error("bind");
    
    if (listen(sock, 10) < 0)
        fatal_error("listen");
    
    return sock;
}


int add_accept_request(
    int listening_socket, 
    struct sockaddr_in *client_addr, 
    socklen_t *client_addr_len
) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    struct context *ctx = zh_malloc(sizeof(*ctx));
    ctx->event_type = EVENT_TYPE_ACCEPT;

    io_uring_prep_accept(sqe, listening_socket, 
                (struct sockaddr *) client_addr, 
                client_addr_len, 0);
    io_uring_sqe_set_data(sqe, ctx);
    io_uring_submit(&ring);

    return 0;
}

int add_read_request(int client_socket) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    struct context *ctx = zh_malloc(sizeof(*ctx) + sizeof(struct iovec));
    ctx->iov[0].iov_base = zh_malloc(READ_SZ);
    ctx->iov[0].iov_len = READ_SZ;
    ctx->event_type = EVENT_TYPE_READ_SOCK;
    ctx->client_socket = client_socket;

    memset(ctx->iov[0].iov_base, 0, READ_SZ);

    // linux 5.5 has support for readv but not for recv() or read()
    io_uring_prep_readv(sqe, client_socket, &ctx->iov[0], 1, 0);
    io_uring_sqe_set_data(sqe, ctx);
    io_uring_submit(&ring);

    return 0;
}

int add_write_request(struct context *ctx) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    ctx->event_type = EVENT_TYPE_WRITE;

    io_uring_prep_writev(sqe, ctx->client_socket, ctx->iov, ctx->iovec_count, 0);
    io_uring_sqe_set_data(sqe, ctx);
    io_uring_submit(&ring);

    return 0;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    
    return dot + 1;
}

void send_headers(const char *path, long content_length, struct iovec *iov) {
    char small_case_path[1024];
    char buf[1024];
    strcpy(small_case_path, path);
    strtolower(small_case_path);

    char *str = "HTTP/1.1 200 OK\r\n";
    unsigned long slen = strlen(str);
    iov[0].iov_base = zh_malloc(slen);
    iov[0].iov_len = slen;
    memcpy(iov[0].iov_base, str, slen);

    slen = strlen(SERVER_STRING);
    iov[1].iov_base = zh_malloc(slen);
    iov[1].iov_len = slen;
    memcpy(iov[1].iov_base, SERVER_STRING, slen);

    const char *file_ext = get_filename_ext(small_case_path);
    if (strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "jpeg") == 0) {
        str = "Content-Type: image/jpeg\r\n";
    }
    if (strcmp(file_ext, "png") == 0) {
        str = "Content-Type: image/png\r\n";
    }
    if (strcmp(file_ext, "gif") == 0) {
        str = "Content-Type: image/gif\r\n";
    }
    if (strcmp(file_ext, "htm") == 0 || strcmp(file_ext, "html") == 0) {
        str = "Content-Type: text/html\r\n";
    }
    if (strcmp(file_ext, "css") == 0) {
        str = "Content-Type: text/css\r\n";
    }
    if (strcmp(file_ext, "js") == 0) {
        str = "Content-Type: application/javascript\r\n";
    }
    if (strcmp(file_ext, "txt") == 0) {
        str = "Content-Type: text/plain\r\n";
    }

    slen = strlen(str);
    iov[2].iov_base = zh_malloc(slen);
    iov[2].iov_len = slen;
    memcpy(iov[2].iov_base, str, slen);

    sprintf(buf, "Content-Length: %ld\r\n", content_length);
    slen = strlen(buf);
    iov[3].iov_base = zh_malloc(slen);
    iov[3].iov_len = slen;
    memcpy(iov[3].iov_base, buf, slen);

    str = "\r\n";
    slen = strlen(str);
    iov[4].iov_base = zh_malloc(slen);
    iov[4].iov_len = slen;
    memcpy(iov[4].iov_base, str, slen);
}


void copy_file_contents(const char *path, long file_size, struct context *ctx) {
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        fatal_error("open");
    printf("open: %s, fd: %d\n", path, fd);
    
    off_t remaining = file_size;
    off_t offset = 0;
    int current_block = 0;
    struct iovec *iovs = ctx->iov;
    while (remaining > 0) {
        off_t bytes_to_read = remaining > READ_SZ ? READ_SZ : remaining;
        offset += bytes_to_read;
        iovs[current_block].iov_base = zh_malloc(bytes_to_read);
        iovs[current_block].iov_len = bytes_to_read;
        current_block++;
        remaining -= bytes_to_read;
    }
    
    ctx->path_fd = fd;
    ctx->event_type = EVENT_TYPE_READ_FILE;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_readv(sqe, fd, iovs, ctx->iovec_count, 0);
    io_uring_sqe_set_data(sqe, ctx);
    io_uring_submit(&ring);
}

void _send_static_string_content(const char *str, int client_socket) {
    struct context *ctx = zh_malloc(sizeof(*ctx) + sizeof(struct iovec));
    unsigned long slen = strlen(str);
    ctx->iovec_count = 1;
    ctx->client_socket = client_socket;
    ctx->iov[0].iov_base = zh_malloc(slen);
    ctx->iov[0].iov_len = slen;
    memcpy(ctx->iov[0].iov_base, str, slen);
    add_write_request(ctx);
}

void handle_unimplemented_method(int client_socket) {
   _send_static_string_content(unimplemented_content, client_socket);
}

void handle_http_404(int client_socket) {
    _send_static_string_content(http_404_content, client_socket);
}

void handle_get_method(char *path, int client_socket) {
    char final_path[1024];
    struct context *ctx;

    if (path[strlen(path) - 1] == '/') {
        strcpy(final_path, "public");
        strcat(final_path, path);
        strcat(final_path, "index.html");
    } else {
        strcpy(final_path, "public");
        strcat(final_path, path);
    }

    struct stat path_stat;
    if (stat(final_path, &path_stat) < 0) {
        printf("404 Not Found: %s (%s)\n", final_path, path);
        handle_http_404(client_socket);
        return;
    }

    if (S_ISREG(path_stat.st_mode)) {
        int iovec_count = 5;

        ctx = zh_malloc(sizeof(*ctx) + sizeof(struct iovec) * iovec_count);
        ctx->iovec_count = iovec_count;
        ctx->client_socket = client_socket;

        send_headers(final_path, path_stat.st_size, ctx->iov);
        add_write_request(ctx);
    #ifndef SENDFILE
        iovec_count = path_stat.st_size / READ_SZ;
        if (path_stat.st_size % READ_SZ != 0)
            iovec_count++;
        ctx = zh_malloc(sizeof(*ctx) + sizeof(struct iovec) * iovec_count);
        ctx->iovec_count = iovec_count;
        ctx->client_socket = client_socket;
        printf("%s %ld bytes\n", final_path, path_stat.st_size);
        copy_file_contents(final_path, path_stat.st_size, ctx);
        // move to EVENT_TYPE_READ_FILE;
        // add_write_request(ctx);
    #else
        ctx->path_fd = open(final_path, O_RDONLY);
        ctx->file_size = path_stat.st_size;
    #endif
    } else {
        handle_http_404(client_socket);
        printf("404 Not Found: %s\n", final_path);
    }
}

void handle_http_method(char *method_buffer, int client_socket) {
    char *method, *path, *saveptr;
    
    method = strtok_r(method_buffer, " ", &saveptr);
    strtolower(method);
    path = strtok_r(NULL, " ", &saveptr);

    if (strcmp(method, "get") == 0) {
        handle_get_method(path, client_socket);
    } else {
        handle_unimplemented_method(client_socket);
    }
}

int get_line(const char *src, char *dest, int dest_sz) {
    for (int i = 0; i < dest_sz; i++) {
        dest[i] = src[i];
        if (src[i] == '\r' && src[i + 1] == '\n') {
            dest[i] = '\0';
            return 0;
        }
    }

    return 1;
}


int handle_client_request(struct context *ctx) {
    char http_request[1024];
    if (get_line(ctx->iov[0].iov_base, http_request, sizeof(http_request))) {
        fprintf(stderr, "Malformed request\n");
        exit(1);
    }

    handle_http_method(http_request, ctx->client_socket);

    return 0;
}

void server_loop(int listening_socket) {
    struct io_uring_cqe *cqe;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    add_accept_request(listening_socket, &client_addr, &client_addr_len);

    while (1) {
        int ret = io_uring_wait_cqe(&ring, &cqe);
        // struct request *req = (struct request *) cqe->user_data
        struct context *ctx = (struct context *) io_uring_cqe_get_data(cqe);
        if (ret < 0) {
            fatal_error("io_uring_wait_cqe");
        }
        if (cqe->res < 0) {
            fprintf(stderr, "Async request failed: %s for event: %d\n", 
                strerror(-cqe->res), ctx->event_type);
            exit(EXIT_FAILURE);
        }

        switch (ctx->event_type) {
        case EVENT_TYPE_ACCEPT:
            printf("Accepted connection from %s:%d\n", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            add_accept_request(listening_socket, &client_addr, &client_addr_len);
            printf("client fd: %d\n", cqe->res);
            add_read_request(cqe->res);
            free(ctx);
            break;
        case EVENT_TYPE_READ_SOCK:
            if (cqe->res == 0) {
                fprintf(stderr, "Empty request! ");
                fprintf(stderr, "Maybe EOF, should be close client fd: %d\n", ctx->client_socket);
                close(ctx->client_socket);
            } else {
                handle_client_request(ctx);
            }

            free(ctx->iov[0].iov_base);
            free(ctx);
            break;
        case EVENT_TYPE_READ_FILE:
            if (cqe->res == 0) {
                fprintf(stderr, "Empty read");
                printf("close fd: %d\n", ctx->path_fd);
                close(ctx->path_fd);
                // close(ctx->client_socket);
                break;
            }

            add_write_request(ctx);
            printf("read file over, close fd: %d\n", ctx->path_fd);
            close(ctx->path_fd);
            break;
        case EVENT_TYPE_WRITE:
        #ifdef SENDFILE
            printf("sendfile %ld bytes to client\n", ctx->file_size);
            sendfile(ctx->client_socket, ctx->path_fd, NULL, ctx->file_size);
            close(ctx->path_fd);
        #endif
            printf("write over, start free\n");
            for (int i = 0; i < ctx->iovec_count; i++) {
                free(ctx->iov[i].iov_base);
            }
            // 写完就关闭? 短链接? 不是很好
            // close(ctx->client_socket);
            if (ctx->path_fd) {
                // 虽然关闭了, 但是 path_fd 值还存在
                // 有path_fd 说明是body, 
                // body 写完后, 将 client socket 放入 read 队列中
                add_read_request(ctx->client_socket);
            }

            free(ctx);
            break;
        }

        io_uring_cqe_seen(&ring, cqe);
    }
    
}

void sigint_handler(int sig) {
    printf("Caught SIGINT, exiting...\n");
    io_uring_queue_exit(&ring);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    int lis_sock = setup_listening_socket(DEFAULT_SERVER_PORT);
    signal(SIGINT, sigint_handler);
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    server_loop(lis_sock);
    return 0;
}
