//
// Created by Eric Dattore on 10/6/16.
//

#include "../headers/includes.h"

void make_query_header(DNSQueryHeader *header)
{
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
}

void make_query_question(DNSQueryQuestion *question)
{
    question->qtype = ntohs(1);
    question->qclass = ntohs(1);
}

uint8_t *domain_to_dns_format(std::string domain)
{
    std::vector<std::string> segments;

    unsigned long loc;
    while (domain.find(".") != std::string::npos) {
        loc = domain.find(".");
        segments.emplace_back(domain.substr(0, loc));
        domain.erase(0, loc + 1);
    }

    segments.emplace_back(domain);

    int seg_bytes = 0;
    for (auto &seg : segments) {
        seg_bytes += seg.size();
    }

    uint8_t *buffer = new uint8_t[segments.size() + seg_bytes + 1];

    int counter = 0;
    for (auto &seg : segments) {
        buffer[counter] = (uint8_t)seg.size();
        counter += 1;
        for (auto &c : seg) {
            buffer[counter] = (uint8_t)c;
            counter += 1;
        }
    }

    buffer[segments.size() + seg_bytes] = '\0';

    return buffer;
}

void close_socket(int fd) { close(fd); }

std::string read_name(uint8_t *buffer, size_t name_start)
{
    std::string domain;
    // read until null terminator
    // ex: 3www6google3com
    size_t ptr = name_start;
    while (buffer[ptr] != 0) {
        size_t num = buffer[ptr];
        for (int i = 0; i < num; ++i) {
            domain += buffer[ptr + i + 1];
        }

        domain += ".";
        ptr += num + 1;
    }

    return domain;
}

bool is_pointer(uint8_t first_word) { return first_word == 192; }

std::string decode_answer_type(uint16_t answer_type)
{
    std::string str_answer_type;
    switch (ntohs(answer_type)) {
    case 1:
        str_answer_type = "A";
        break;
    case 2:
        str_answer_type = "NS";
        break;
    case 5:
        str_answer_type = "CNAME";
        break;
    case 6:
        str_answer_type = "SOA";
        break;
    case 12:
        str_answer_type = "MX";
        break;
    case 15:
        str_answer_type = "PTR";
        break;
    default:
        break;
    }

    return str_answer_type;
}

std::string decode_ip(uint32_t ip_addr)
{
    std::string ip_str;

    struct in_addr ip;
    ip.s_addr = ip_addr;

    ip_str = inet_ntoa(ip);

    return ip_str;
}

void decode_header(DNSQueryHeader *header, uint16_t value)
{
    header->qr = (uint16_t)(value >> 15);
    header->opcode = (uint16_t)((value >> 11) & ~((uint16_t)~0 << 4));
    header->aa = (uint16_t)((value >> 10) & ~((uint16_t)~0 << 1));
    header->tc = (uint16_t)((value >> 9) & ~((uint16_t)~0 << 1));
    header->rd = (uint16_t)((value >> 8) & ~((uint16_t)~0 << 1));
    header->ra = (uint16_t)((value >> 7) & ~((uint16_t)~0 << 1));
    header->z = (uint16_t)((value >> 6) & ~((uint16_t)~0 << 3));
    header->rcode = (uint16_t)((value >> 3) & ~((uint16_t)~0 << 4));
}

std::string get_dns_error(uint16_t error_code)
{
    std::string error;
    if (error_code == 1) {
        error = "Malformed packet";
    }
    else if (error_code == 2) {
        error = "Server failure";
    }
    else if (error_code == 3) {
        error = "The server has no record of the requested domain";
    }
    else if (error_code == 4) {
        error = "The nameserver does not support the type of query requested";
    }
    else {
        error = "The nameserver refused the request";
    }

    return error;
}
