#include "../headers/includes.h"

// TODO: Refactor to properly delete buffers and not copy data (point to position in buffer)
int main(int argc, char **argv) {
    int sockfd;
    ssize_t n;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;

    if (argc < 3) {
        std::cerr << "Incorrect usage" << std::endl;
        exit(1);
    }

    ip_to_find = (char *) domain_to_dns_format(argv[1]);
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

    size_t header_size = sizeof(DNSQueryHeader);
    size_t question_size = sizeof(DNSQueryQuestion);
    // Need to add one since c_str() adds a null terminator
    size_t domain_to_query_size = ip_to_find.size() + 1;
    size_t total_size = header_size + question_size + domain_to_query_size;
    std::cout << "Size of header: " << header_size << std::endl <<
              "Size of question: " << question_size << std::endl <<
              "Size of domain c_str: " << ip_to_find.size() + 1 << std::endl;

    // Absolutely dangerous, why would you do this?!?!
    uint8_t *send_buf = new uint8_t[total_size];
    DNSQueryHeader *header = (DNSQueryHeader *) send_buf;
    header->id = htons(getpid());
    header->qr = 0;
    header->opcode = 0;
    header->aa = 0;
    header->tc = 0;
    header->rd = 0;
    header->ra = 0;
    header->z = 0;
    header->rcode = 0;
    header->qdcount = ntohs(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;

    char *domain = (char *) (send_buf + header_size);
    strcpy(domain, ip_to_find.c_str());

    DNSQueryQuestion *question = (DNSQueryQuestion *) (send_buf + header_size + domain_to_query_size);
    question->qtype = ntohs(1);
    question->qclass = ntohs(1);

    socklen_t len = sizeof(serveraddr);
    n = sendto(sockfd, send_buf, total_size, 0,
                     (const sockaddr *) &serveraddr, len);
    if (n < 0) {
        std::cerr << "Error in sending: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    uint8_t buf[65527];
    n = recvfrom(sockfd, buf, 65527, 0, (sockaddr *) &serveraddr, &len);
    if (n < 0) {
        std::cerr << "Error in receiving: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    // buf contains header + domain + question + answer
    DNSAnswerSegment *answer = nullptr;
    answer = (DNSAnswerSegment *) &buf[total_size + 1];

    std::cout << "domain offset is " << get_domain_offset_from_answer(answer->nameOffset) << std::endl;
    std::cout << "type is " << answer->type << std::endl;

    close_socket(sockfd);

    return 0;
}