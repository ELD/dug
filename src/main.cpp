#include "../headers/includes.h"

int main(int argc, const char *argv[])
{
    std::string ip_to_find, nameserver_to_query, original_question;
    std::vector<std::string> valid_record_types{"A", "CNAME", "NS", "MX", "SOA", "PTR"};
    bool found_answer = false, debug = false, first_time = true;
    uint8_t *ans_buf = nullptr;

    auto pair = make_command_line_parser(argc, argv);
    auto options = pair.first;
    auto vmap = pair.second;

    if (vmap.count("help")) {
        std::cout << "Usage: dug [options] [domain] [server]" << std::endl;
        std::cout << options << std::endl;
        return 0;
    }
    else if (!vmap.count("domain") || !vmap.count("server")) {
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

    if (debug)
        std::cout << "Searching for record of type: " << record_type << std::endl;

    if (std::find(valid_record_types.begin(), valid_record_types.end(), record_type) == valid_record_types.end()) {
        std::cout << "That record type is not supported." << std::endl;
        return 1;
    }

    original_question = vmap["domain"].as<std::string>();
    ip_to_find = (char *)domain_to_dns_format(original_question);
    nameserver_to_query = vmap["server"].as<std::string>();
    while (!found_answer) {
        ans_buf = send_and_recv(ip_to_find, nameserver_to_query, record_type);

        auto header_size = sizeof(DNSQueryHeader);
        auto question_size = sizeof(DNSQueryQuestion);
        // Need to add one since c_str() adds a null terminator
        auto domain_to_query_size = ip_to_find.size() + 1;
        auto total_size = header_size + question_size + domain_to_query_size;

        auto *header = (DNSQueryHeader *)ans_buf;
        // Because of byte order, we have to do some extra work to fill the second 16 bit value properly
        decode_header(header, ntohs(((ans_buf[3] << 8) | ans_buf[2])));

        if (header->rcode != 0) {
            if (debug) {
                std::cout << "An error occurred: " << get_dns_error(header->rcode) << std::endl;
                if (first_time) {
                    std::cout << "Following up with: " << ROOT_NAMESERVER << std::endl;
                    first_time = false;
                }
                else {
                    return 1;
                }
            }

            nameserver_to_query = std::string(ROOT_NAMESERVER);
            continue;
        }

        if (header->aa == 1 && header->ancount == 0) {
            std::cout << "The requested domain does not appear to exist." << std::endl;
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
        // Read the domain in the answer section to figure out where the rest of the answer begins, count # of bytes
        // from read_name()
        auto domain_offset = total_size;
        if (header->ancount > 0) {
            auto name_and_offset = read_name(ans_buf, domain_offset);

            auto answer_offset = total_size + name_and_offset.second;

            auto *answer = (DNSAnswerSegment *)&ans_buf[answer_offset];

            if (debug)
                std::cout << "Answer domain: " << name_and_offset.first << std::endl;

            // Read answer section if it exists
            auto rd_data_start = answer_offset + sizeof(DNSAnswerSegment);
            if (decode_answer_type(ntohs(answer->type)) == "A" &&
                record_type == decode_answer_type(ntohs(answer->type)) && name_and_offset.first == original_question) {
                uint32_t ip_addr;
                // Don't worry about manually flipping byte order, just use ntohl()
                ip_addr = ans_buf[rd_data_start] << 24 | ans_buf[rd_data_start + 1] << 16 |
                          ans_buf[rd_data_start + 2] << 8 | ans_buf[rd_data_start + 3];
                auto ip = decode_ip(ntohl(ip_addr));
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ip << std::endl;
                }
                else {
                    std::cout << "Non-authoritative answer: " << ip << std::endl;
                }

                found_answer = true;
            }
            else if (decode_answer_type(ntohs(answer->type)) == "CNAME" &&
                     record_type == decode_answer_type(ntohs(answer->type))) {
                std::pair<std::string, int> ns = read_name(ans_buf, rd_data_start);
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ns.first << std::endl;
                }
                else {
                    std::cout << "Non-authoritative answer: " << ns.first << std::endl;
                }

                found_answer = true;
            }
            else if ((decode_answer_type(ntohs(answer->type)) == "NS" &&
                      record_type == decode_answer_type(ntohs(answer->type))) ||
                     (decode_answer_type(ntohs(answer->type)) == "PTR" &&
                      record_type == decode_answer_type(ntohs(answer->type)))) {
                std::pair<std::string, int> ns = read_name(ans_buf, rd_data_start);
                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << ns.first << std::endl;
                }
                else {
                    std::cout << "Non-authoritative answer: " << ns.first << std::endl;
                }

                found_answer = true;
            }
            else if (decode_answer_type(ntohs(answer->type)) == "MX" &&
                     record_type == decode_answer_type(ntohs(answer->type))) {
                uint16_t preference = ans_buf[rd_data_start] << 8 | ans_buf[rd_data_start + 1];
                auto mx_record = read_name(ans_buf, rd_data_start + 2);

                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << preference << " " << mx_record.first << std::endl;
                }
                else {
                    std::cout << "Non-authoritative answer: " << preference << " " << mx_record.first << std::endl;
                }

                found_answer = true;
            }
            else if (decode_answer_type(ntohs(answer->type)) == "SOA" &&
                     record_type == decode_answer_type(ntohs(answer->type))) {
                auto primary_ns = read_name(ans_buf, rd_data_start);
                auto admin_mb = read_name(ans_buf, rd_data_start + primary_ns.second);

                if (header->aa == 1) {
                    std::cout << "Authoritative answer: " << primary_ns.first << "\t" << admin_mb.first << std::endl;
                }
                else {
                    std::cout << "Non-authoritative answer: " << primary_ns.first << "\t" << admin_mb.first
                              << std::endl;
                }

                found_answer = true;
            }
        }

        if (!found_answer) {
            // Read authority section if it exists
            std::vector<std::pair<std::string, std::string>> nameservers_or_cnames;
            if (ntohs(header->nscount > 0)) {
                if (debug)
                    std::cout << "Reading authoritative records" << std::endl;

                size_t answer_offset = domain_offset;
                for (int i = 0; i < ntohs(header->nscount); ++i) {

                    auto name_and_offset = read_name(ans_buf, answer_offset);
                    if (debug)
                        std::cout << "Domain: " << name_and_offset.first << std::endl;

                    answer_offset += name_and_offset.second;

                    DNSAnswerSegment *answer = (DNSAnswerSegment *)&ans_buf[answer_offset];

                    if (debug)
                        std::cout << "Type: " << decode_answer_type(ntohs(answer->type)) << std::endl;

                    auto rdata_and_offset = read_name(ans_buf, answer_offset + sizeof(DNSAnswerSegment));
                    answer_offset += sizeof(DNSAnswerSegment) + rdata_and_offset.second;

                    if (debug)
                        std::cout << "Checking with: " << rdata_and_offset.first << std::endl;

                    nameservers_or_cnames.emplace_back(
                        std::make_pair(rdata_and_offset.first, decode_answer_type(ntohs(answer->type))));
                }

                domain_offset = answer_offset;
            }

            // Read additional section if exists
            if (ntohs(header->arcount > 0)) {
                if (debug)
                    std::cout << "Reading additional records" << std::endl;

                size_t answer_offset = domain_offset;
                for (int i = 0; i < ntohs(header->arcount); ++i) {
                    auto name_and_offset = read_name(ans_buf, answer_offset);

                    if (debug)
                        std::cout << "Domain: " << name_and_offset.first << std::endl;

                    answer_offset += name_and_offset.second;

                    DNSAnswerSegment *answer = (DNSAnswerSegment *)&ans_buf[answer_offset];
                    answer_offset += sizeof(DNSAnswerSegment);

                    if (debug)
                        std::cout << "Type: " << decode_answer_type(ntohs(answer->type)) << std::endl;

                    if (decode_answer_type(ntohs(answer->type)) == "A") {
                        uint32_t ip_addr;
                        // Don't worry about manually flipping byte order, just use ntohl()
                        ip_addr = ans_buf[answer_offset] << 24 | ans_buf[answer_offset + 1] << 16 |
                                  ans_buf[answer_offset + 2] << 8 | ans_buf[answer_offset + 3];
                        auto ip = decode_ip(ntohl(ip_addr));

                        answer_offset += 4;

                        if (std::find(nameservers_or_cnames.begin(), nameservers_or_cnames.end(),
                                      std::make_pair(name_and_offset.first, std::string("NS"))) !=
                                nameservers_or_cnames.end() ||
                            std::find(nameservers_or_cnames.begin(), nameservers_or_cnames.end(),
                                      std::make_pair(name_and_offset.first, std::string("CNAME"))) !=
                                nameservers_or_cnames.end()) {
                            nameserver_to_query = ip;
                            if (debug)
                                std::cout << "Following up with: " << nameserver_to_query << std::endl;

                            break;
                        }
                    }
                    else {
                        answer_offset += ntohs(answer->rdlength);
                    }
                }
            }
        }

        memset(ans_buf, 0, 512);
    }

    delete[] ans_buf;

    return 0;
}