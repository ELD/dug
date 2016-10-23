#include "../headers/includes.h"

int main(int argc, const char *argv[])
{
    int sockfd;
    ssize_t n;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;

    //////////////////////////////////////////////////////////////////////////////////
    // Command Line Arguments
    //////////////////////////////////////////////////////////////////////////////////
    po::options_description options{"Allowed Options"};
    options.add_options()
            ("help,h", "Help using dug")
            ("type,t",
             po::value<std::string>()->value_name("record type")->default_value("A"),
             "Type of the requested DNS record"
            )
            ("domain", po::value<std::string>(), "The domain to query DNS records for")
            ("server", po::value<std::string>(), "The DNS server to query");

    po::positional_options_description pa_options;
    pa_options.add("domain", 1);
    pa_options.add("server", 2);

    po::command_line_parser parser{argc, argv};
    parser.options(options).positional(pa_options).allow_unregistered();

    po::variables_map vmap;
    po::store(parser.run(), vmap);
    po::notify(vmap);

    if (vmap.count("help")) {
        std::cout << "Usage: dug [options] [domain] [server]" << std::endl;
        std::cout << options << std::endl;
        return 0;
    } else if (!vmap.count("domain") || !vmap.count("server")) {
        std::cout << "Too few arguments provided. Printing usage." << std::endl;
        std::cout << "Usage: dug [options] [domain] [server]" << std::endl;
        std::cout << options << std::endl;
        return 1;
    }

    std::string record_type;
    if (vmap.count("type")) {
        record_type = vmap["type"].as<std::string>();
    }
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // Opening connection
    //////////////////////////////////////////////////////////////////////////////////
    ip_to_find = (char *)domain_to_dns_format(vmap["domain"].as<std::string>());
    nameserver_to_query = vmap["server"].as<std::string>();

    if (record_type == "A") {

    } else {
        std::cout << "That record type is not supported." << std::endl;
        return 1;
    }

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

    //////////////////////////////////////////////////////////////////////////////////
    // Sending and receiving the data and parsing it
    //////////////////////////////////////////////////////////////////////////////////
    socklen_t len = sizeof(serveraddr);
    n = sendto(sockfd, send_buf, total_size, 0, (const sockaddr *)&serveraddr, len);
    if (n < 0) {
        std::cerr << "Error in sending: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    // Max DNS packet size is 512 bytes as per RFC 1035 (?)
    uint8_t *ans_buf = new uint8_t[512];
    n = recvfrom(sockfd, ans_buf, 512, 0, (sockaddr *)&serveraddr, &len);
    if (n < 0) {
        std::cerr << "Error in receiving: " << strerror(errno) << std::endl;
        close_socket(sockfd);
        exit(-1);
    }

    // TODO: If no answers provided, do recursive requests
    header = (DNSQueryHeader *)ans_buf;
    decode_header(header, ntohs(ans_buf[2]));
    header->id = (uint16_t) ntohs(*ans_buf);

    if (!header->rcode) {
        std::cout << "An error occurred: " << get_dns_error(header->rcode) << std::endl;
        return 1;
    }

    // buf contains header + domain + question + answer
    // Read the domain in the answer section to figure out where the rest of the answer begins
    size_t domain_offset = total_size;
    if (is_pointer(ans_buf[total_size])) {
        domain_offset = ans_buf[total_size + 1];
    }

    std::string answer_name = read_name(ans_buf, domain_offset);

    size_t answer_offset = total_size;
    if (is_pointer(ans_buf[total_size])) {
        answer_offset += 2;
    }
    else {
        answer_offset += answer_name.size() + 1;
    }

    DNSAnswerSegment *answer = (DNSAnswerSegment *)&ans_buf[answer_offset];

    if (decode_answer_type(answer->type) == "A") {
        size_t rd_data_start = answer_offset + sizeof(DNSAnswerSegment);
        uint32_t ip_addr;
        ip_addr = ans_buf[rd_data_start] |
                (ans_buf[rd_data_start + 1] << 8) |
                (ans_buf[rd_data_start + 2] << 16) |
                (ans_buf[rd_data_start + 3] << 24);
        std::string ip = decode_ip(ip_addr);
        if (header->aa == 1) {
            std::cout << "Authoritative answer: " << ip << std::endl;
        } else {
            std::cout << "Non-authoritative answer: " << ip << std::endl;
        }
    }

    delete[] send_buf;
    delete[] ans_buf;
    close_socket(sockfd);

    return 0;
}