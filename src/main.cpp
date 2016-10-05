#include "../headers/includes.h"

int main(int argc, char** argv) {
    int sockfd, n;
    const int portno = 53;
    std::string hostname, destination;
    struct sockaddr_in serveraddr;
    struct hostent *server;

    if (argc < 3) {
        std::cerr << "Incorrect usage" << std::endl;
        exit(1);
    }

    hostname = argv[1];
    destination = argv[2];

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "Binding to socket failed" << std::endl;
        exit(1);
    }

    server = gethostbyname(hostname.c_str());

    if (server == nullptr) {
        std::cerr << "Error, no such host" << std::endl;
        exit(1);
    }

    // I use memset since bzero and memset have no functional difference, just a preference
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = PF_INET;
    // I use memcpy instead of bcpy same as above
    memcpy((char *)serveraddr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
    serveraddr.sin_port = htons(portno);

    char* buf = (char *) "Hello";

    int ret = (int) sendto(sockfd, buf, sizeof(buf), 0, (const sockaddr *) &serveraddr, sizeof(serveraddr));

    n = (int) recvfrom(sockfd, buf, strlen(buf), 0, (sockaddr *) &serveraddr, (socklen_t *) sizeof(serveraddr));

    std::cout << "From server: " << buf << std::endl;

    return 0;
}