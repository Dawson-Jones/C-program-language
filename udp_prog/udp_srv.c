#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
	
#define PORT	55001
#define MAXLINE 1024

struct segroute {
    int vxlan_flag;
    // __be32 vx_vni: 24;
    // __u8 reserverd;
    int vx_vni;
    int seg_list[3];
};
	
// Driver code
int main() {
	int sockfd;
	char buffer[MAXLINE];
	char *hello = "Hello from server";
	struct sockaddr_in servaddr, cliaddr;
		
	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);
		
	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	int len, n;
	
	// len = sizeof(cliaddr); //len is value/result
	
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,
				&len);
	buffer[n] = '\0';
	// printf("Client : %s\n", buffer);
    printf("received-----%d\n", n);
    struct segroute *sr = (struct segroute *) buffer;
    printf("flag: 0x%x\n", sr->vxlan_flag);
    printf("vni: %x\n", sr->vx_vni);
    printf("flag: 0x%x\n", sr->seg_list[0]);
    printf("flag: %d\n", sr->seg_list[1]);
    printf("flag: %d\n", sr->seg_list[2]);

    char *rest = sr + 1;
    printf("%s\n", rest);

	sendto(sockfd, (const char *)hello, strlen(hello),
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			len);
	printf("Hello message sent.\n");
		
	return 0;
}
