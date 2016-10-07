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

DNSQueryQuestion *make_query_question(std::string const &domain)
{
    DNSQueryQuestion *q = new DNSQueryQuestion;
    q->qtype = ntohs(1);
    q->qclass = ntohs(1);

    return q;
}
