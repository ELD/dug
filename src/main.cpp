#include "../headers/includes.h"

// TODO: Make this all not suck
// TODO: Also, understand how to set up a DNS query and UDP socket programming...

void close_socket(int fd)
{
    close(fd);
}

int main(int argc, char** argv) {
    int sockfd, n;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;

    if (argc < 3) {
        std::cerr << "Incorrect usage" << std::endl;
        exit(1);
    }

    ip_to_find = argv[1];
    uint8_t c_ip_to_find[] = {0x07, 0x69, 0x6d, 0x61, 0x67, 0x69, 0x6e, 0x65, 0x05, 0x6d, 0x69, 0x6e, 0x65, 0x73, 0x03, 0x65, 0x64, 0x75, 0x00};
    nameserver_to_query = argv[2];

    sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd < 0) {
        std::cerr << "Binding to socket failed" << std::endl;
        exit(1);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = PF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(nameserver_to_query.c_str());
    serveraddr.sin_port = htons(PORT_NO);

    DNSQueryHeader *query = make_query_header();
    DNSQueryQuestion *question = make_query_question(ip_to_find);

    size_t header_size = sizeof(*query);
    size_t question_size = sizeof(*question);
    size_t domain_to_query_size = sizeof c_ip_to_find;

    uint8_t *send_buf = new uint8_t[header_size + question_size];
    memcpy(send_buf, query, header_size);
    memcpy(send_buf + header_size, c_ip_to_find, domain_to_query_size);
    memcpy(send_buf + domain_to_query_size + header_size, question, sizeof(*question));

    socklen_t len = sizeof(serveraddr);
    n = (int) sendto(sockfd, send_buf, header_size + question_size + domain_to_query_size, 0, (const sockaddr *) &serveraddr, len);
    if (n < 0) {
        std::cerr << "Error in sending: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    char *buf;
    n = (int) recvfrom(sockfd, buf, strlen(buf), 0, (sockaddr *) &serveraddr, &len);
    if (n < 0) {
        std::cerr << "Error in receiving: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    std::cout << "From server: " << buf << std::endl;

    return 0;
}