#include "../headers/includes.h"

void close_socket(int fd)
{
    close(fd);
}

int main(int argc, char** argv) {
    int sockfd, n;
    const int portno = 53;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;

    if (argc < 3) {
        std::cerr << "Incorrect usage" << std::endl;
        exit(1);
    }

    ip_to_find = argv[1];
    nameserver_to_query = argv[2];

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "Binding to socket failed" << std::endl;
        exit(1);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = PF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(nameserver_to_query.c_str());
    serveraddr.sin_port = htons(portno);

    char* buf = (char *) "Hello";

    std::cout << "Sending packet to socket" << std::endl;
    n = (int) sendto(sockfd, buf, sizeof(buf), 0, (const sockaddr *) &serveraddr, sizeof(serveraddr));
    if (n < 0) {
        std::cerr << "Error in sending: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    std::cout << "Receiving response" << std::endl;
    n = (int) recvfrom(sockfd, buf, strlen(buf), 0, (sockaddr *) &serveraddr, (socklen_t *) sizeof(serveraddr));
    if (n < 0) {
        std::cerr << "Error in receiving: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    std::cout << "From server: " << buf << std::endl;

    return 0;
}