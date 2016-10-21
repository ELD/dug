#include "../headers/includes.h"

// TODO: Refactor to properly delete buffers and not copy data (point to position in buffer)
int main(int argc, char **argv)
{
    int sockfd;
    ssize_t n;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;

    if (argc < 3) {
        std::cerr << "Incorrect usage" << std::endl;
        exit(1);
    }

    ip_to_find = (char *)domain_to_dns_format(argv[1]);
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

    // Hehe this is fun, and dangerous. Man, do I love C++!
    uint8_t *send_buf = new uint8_t[total_size];
    DNSQueryHeader *header = (DNSQueryHeader *)send_buf;
    make_query_header(header);

    char *domain = (char *)(send_buf + header_size);
    strcpy(domain, ip_to_find.c_str());

    DNSQueryQuestion *question = (DNSQueryQuestion *)(send_buf + header_size + domain_to_query_size);
    make_query_question(question);

    socklen_t len = sizeof(serveraddr);
    n = sendto(sockfd, send_buf, total_size, 0, (const sockaddr *)&serveraddr, len);
    if (n < 0) {
        std::cerr << "Error in sending: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    // Max DNS packet size is 512 bytes as per RFC 1035 (?)
    uint8_t buf[512];
    n = recvfrom(sockfd, buf, sizeof(buf), 0, (sockaddr *)&serveraddr, &len);
    if (n < 0) {
        std::cerr << "Error in receiving: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    // buf contains header + domain + question + answer
    // Read the domain in the answer section to figure out where the rest of the answer begins

    size_t domain_offset = total_size;
    if (is_pointer(buf[total_size])) {
        domain_offset = buf[total_size + 1];
    }

    std::string answer_name = read_name(buf, domain_offset);

    std::cout << "Domain? " << answer_name << std::endl;

    size_t answer_offset = total_size;
    if (is_pointer(buf[total_size])) {
        answer_offset += 2;
    } else {
        answer_offset += answer_name.size() + 1;
    }

    DNSAnswerSegment *answer = (DNSAnswerSegment *)&buf[answer_offset];

    std::cout << "type: " << decode_answer_type(answer->type) << std::endl;
    std::cout << "answer class: " << ntohs(answer->responseClass) << std::endl;

    close_socket(sockfd);

    return 0;
}