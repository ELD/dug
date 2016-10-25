#include "../headers/includes.h"

int main(int argc, const char *argv[])
{
    std::string ip_to_find, nameserver_to_query;
    std::vector<std::string> valid_record_types{"A", "CNAME", "NS", "MX", "SOA", "PTR"};
    bool found_answer = false, debug = false;
    uint8_t *ans_buf = nullptr;

    auto pair = make_command_line_parser(argc, argv);
    auto options = pair.first;
    auto vmap = pair.second;

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

    if (std::find(valid_record_types.begin(), valid_record_types.end(), record_type) == valid_record_types.end()) {
        std::cout << "That record type is not supported." << std::endl;
        return 1;
    }

    while (!found_answer) {
        // TODO: Begin loop here
        ip_to_find = (char *) domain_to_dns_format(vmap["domain"].as<std::string>());
        nameserver_to_query = vmap["server"].as<std::string>();

        auto *ans_buf = send_and_recv(ip_to_find, nameserver_to_query, record_type);

        auto header_size = sizeof(DNSQueryHeader);
        auto question_size = sizeof(DNSQueryQuestion);
        // Need to add one since c_str() adds a null terminator
        auto domain_to_query_size = ip_to_find.size() + 1;
        auto total_size = header_size + question_size + domain_to_query_size;

        // TODO: If no answers provided, do recursive requests
        // TODO: If no authoritative or additional records, start from the root server and work your way down
        auto *header = (DNSQueryHeader *) ans_buf;
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
        // Read the domain in the answer section to figure out where the rest of the answer begins, count # of bytes from read_name()
        auto domain_offset = total_size;
        if (header->ancount > 0) {
            auto name_and_offset = read_name(ans_buf, domain_offset);

            auto answer_offset = total_size + name_and_offset.second;

            auto *answer = (DNSAnswerSegment *) &ans_buf[answer_offset];
            answer->responseClass = ntohs(answer->responseClass);
            answer->ttl = ntohl(answer->ttl);
            answer->rdlength = ntohs(answer->rdlength);
            answer->type = ntohs(answer->type);

            // Read answer section if it exists
            auto rd_data_start = answer_offset + sizeof(DNSAnswerSegment);
            if (decode_answer_type(answer->type) == "A" && record_type == decode_answer_type(answer->type)) {
                uint32_t ip_addr;
                // Don't worry about manually flipping byte order, just use ntohl()
                ip_addr = ans_buf[rd_data_start] << 24 |
                          ans_buf[rd_data_start + 1] << 16 |
                          ans_buf[rd_data_start + 2] << 8 |
                          ans_buf[rd_data_start + 3];
                auto ip = decode_ip(ntohl(ip_addr));
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ip << std::endl;
                } else {
                    std::cout << "Non-authoritative answer: " << ip << std::endl;
                }

                found_answer = true;
            } else if (decode_answer_type(answer->type) == "CNAME" && record_type == decode_answer_type(answer->type)) {
                std::pair<std::string, int> ns = read_name(ans_buf, rd_data_start);
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ns.first << std::endl;
                } else {
                    std::cout << "Non-authoritative answer: " << ns.first << std::endl;
                }

                found_answer = true;
            } else if (decode_answer_type(answer->type) == "NS" && record_type == decode_answer_type(answer->type)) {
                std::pair<std::string, int> ns = read_name(ans_buf, rd_data_start);
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ns.first << std::endl;
                } else {
                    std::cout << "Non-authoritative answer: " << ns.first << std::endl;
                }

                found_answer = true;
            }
        }

        if (!found_answer) {
            // Read authority section if it exists
            if (header->nscount > 0) {
            }

            // Read additional section if exists
            if (header->arcount > 0) {
            }
        }

        memset(ans_buf, 0, 512);
    }


    delete[] ans_buf;

    return 0;
}