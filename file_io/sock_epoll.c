#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 100


void setnonblocking(int sock)
{
	int opts;
	if ((opts = fcntl(sock, F_GETFL)) < 0) {
		perror("fcntl(sock, GETFL");
		exit(1);
	}

	opts |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(sock, SETFL, opts)");
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	int listen_fd, connfd;
	int epfd, nfds;
	int port_number;
	ssize_t bytes_len;
	char line[MAXLINE], ip_str[64];
	struct sockaddr_in cli_addr, srv_addr;
	socklen_t cli_len = sizeof(cli_addr);
	struct epoll_event ev, events[20];

	if (argc == 2) {
		if ((port_number = atoi(argv[1])) < 0) {
			fprintf(stderr, "Usage: %s port number\n", argv[1]);
			return 1;
		}
	} else {
		fprintf(stderr, "Usage: %s port number\n", argv[1]);
		return 1;
	}

	epfd = epoll_create(256);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	setnonblocking(listen_fd);
	ev.data.fd = listen_fd;
	ev.events = EPOLLIN | EPOLLET;
	// register epoll event
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	inet_aton("0.0.0.0", &srv_addr.sin_addr);
	srv_addr.sin_port = htons(port_number);
	bind(listen_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
	listen(listen_fd, 20);

	for ( ; ; ) {
		nfds = epoll_wait(epfd, events, 20, 500);

		for (int i = 0; i < nfds; ++i) {
			if ((connfd = events[i].data.fd) == listen_fd) {	// new cli socket
				if ((connfd = accept(listen_fd, (struct sockaddr *) &cli_addr, &cli_len)) < 0) {
					perror("connfd < 0");
					exit(1);
				}
				setnonblocking(connfd);

				char *ip_str = inet_ntoa(cli_addr.sin_addr);
				printf("accept a connection from %s:%d\n", ip_str, ntohs(cli_addr.sin_port));

				events[i].data.fd = connfd;
				events[i].events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &events[i]);
			} else if (events[i].events & EPOLLIN) {	// already connected cli socket
				getpeername(events[i].data.fd, (struct sockaddr *) &cli_addr, &cli_len);
				inet_ntop(AF_INET, &cli_addr.sin_addr, ip_str, sizeof(ip_str));
				printf("receive from %s:%d\n", ip_str, ntohs(cli_addr.sin_port));
				
				if ((bytes_len = read(connfd, line, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
						printf("econnreset\n");
						close(connfd);
						events[i].data.fd = -1;
					} else {
						printf("readline error\n");
					}
				} else if (bytes_len == 0) {
					printf("read 0 bytes from socket\n");
					close(connfd);
					events[i].data.fd = -1;
				} else { // bytes_len > 0
					printf("%s\n", line);
					write(connfd, line, bytes_len);
				}
			}
		}
	}

	close(epfd);
	return 0;
}