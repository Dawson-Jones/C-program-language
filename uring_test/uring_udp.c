#include <sys/socket.h>
#include <liburing.h>


#define PORT 8000


int setup_listening_socket(int port)
{
    int sock_fd;
    struct sockaddr_in srv_addr;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
}



int main(int argc, char const *argv[])
{

    return 0;
}
