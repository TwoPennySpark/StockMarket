#ifndef CLIENT_H
#define CLIENT_H

#include "net_client.h"
#include "ms_types.h"

class SMClient: public tps::net::client_interface<packet_type>
{
public:
    void send_pkt(sm_packet& pkt);

    void publish_new_offer(offer_type type, currency_type volumeCur, currency_type priceCur,
                           uint32_t volume, uint32_t price);
    void req_balance(const std::vector<currency_type> &curns);
    void req_offers(packet_type type, currency_type volumeCur, currency_type priceCur);
};

#endif // CLIENT_H
