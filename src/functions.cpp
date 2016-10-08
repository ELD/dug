//
// Created by Eric Dattore on 10/6/16.
//

#include "../headers/includes.h"

DNSQueryHeader *make_query_header()
{
    DNSQueryHeader *q = new DNSQueryHeader;
    q->id = htons(getpid());
    q->qr = 0;
    q->opcode = 0;
    q->aa = 0;
    q->tc = 0;
    q->rd = 0;
    q->ra = 0;
    q->z = 0;
    q->rcode = 0;
    q->qdcount = ntohs(1);
    q->ancount = 0;
    q->nscount = 0;
    q->arcount = 0;

    return q;
}

DNSQueryQuestion *make_query_question()
{
    DNSQueryQuestion *q = new DNSQueryQuestion;
    q->qtype = ntohs(1);
    q->qclass = ntohs(1);

    return q;
}

std::string domain_to_dns_format(std::string domain)
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
    for (auto& seg : segments) {
        seg_bytes += seg.size();
    }

    char *buffer = new char[segments.size() + seg_bytes];

    int counter = 0;
    for (auto& seg : segments) {
        buffer[counter] = (char) seg.size();
        counter += 1;
        for (auto& c : seg) {
            buffer[counter] = c;
            counter += 1;
        }
    }

    return std::string(buffer);
}
