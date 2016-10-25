#include "../headers/includes.h"

int main(int argc, const char *argv[])
{
    int sockfd;
    ssize_t n;
    std::string ip_to_find, nameserver_to_query;
    struct sockaddr_in serveraddr;
    std::vector<std::string> valid_record_types{"A", "CNAME", "NS", "MX", "SOA", "PTR"};
    bool found_answer = false, debug = false;

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
            ("debug,d", "Print program trace")
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

    if (vmap.count("debug")) {
        debug = true;
    }
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // Opening connection
    //////////////////////////////////////////////////////////////////////////////////
    // TODO: Begin loop here
    ip_to_find = (char *)domain_to_dns_format(vmap["domain"].as<std::string>());
    nameserver_to_query = vmap["server"].as<std::string>();

    if (std::find(valid_record_types.begin(), valid_record_types.end(), record_type) == valid_record_types.end()) {
        std::cout << "That record type is not supported." << std::endl;
        return 1;
    }

    uint8_t *ans_buf = send_and_recv(ip_to_find, nameserver_to_query);

    size_t header_size = sizeof(DNSQueryHeader);
    size_t question_size = sizeof(DNSQueryQuestion);
    // Need to add one since c_str() adds a null terminator
    size_t domain_to_query_size = ip_to_find.size() + 1;
    size_t total_size = header_size + question_size + domain_to_query_size;

    // TODO: If no answers provided, do recursive requests
    // TODO: If no authoritative or additional records, start from the root server and work your way down
    DNSQueryHeader *header = (DNSQueryHeader *)ans_buf;
    // Because of byte order, we have to do some extra work to fill the second 16 bit value properly
    decode_header(header, ntohs(((ans_buf[3] << 8) | ans_buf[2])));

    if (header->rcode != 0) {
        std::cout << "An error occurred: " << get_dns_error(header->rcode) << std::endl;
        return 1;
    }

    if (header->ancount < 1 && header->arcount < 1 && header->nscount < 1) {
        std::cout << "No records were found for the specified query" << std::endl;
        return 1;
    }

    if (debug) {
        if (header->ancount < 1) {
            std::cout << "No answers were found, checking additional sections" << std::endl;
        }

        if (header->nscount > 0) {
            std::cout << "Authority records found, checking with them to continue the query" << std::endl;
        }

        if (header->arcount > 0) {
            std::cout << "Additional records found, using these as helpers to continue the query" << std::endl;
        }
    }

    // buf contains header + domain + question + answer
    // Read the domain in the answer section to figure out where the rest of the answer begins
    if (header->ancount > 0) {
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

        // Read answer section if it exists
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
        } else if (decode_answer_type(answer->type) == "CNAME") {
            if (header->aa == 1) {
                std::cout << "Authoritative answer: " << std::endl;
            } else {
                std::cout << "Non-authoritative answer: " << std::endl;
            }
        } else if (decode_answer_type(answer->type) == "NS") {
            if (header->aa == 1) {
                std::cout << "Authoritative answer: " << std::endl;
            } else {
                std::cout << "Non-authoritative answer: " << std::endl;
            }
        }

        return 0;
    }

    // Read authority section if it exists
    if (header->nscount > 0) {

    }

    // Read additional section if exists
    if (header->arcount > 0) {

    }

//    delete[] send_buf;
    delete[] ans_buf;
    close_socket(sockfd);

    return 0;
}