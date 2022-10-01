#ifndef CLIENT_H
#define CLIENT_H

#include "net_client.h"
#include "ms_types.h"

class SMClient: public tps::net::client_interface<packet_type>
{
public:
    void publish_new_offer(offer_type type, currency_type volumeCur, currency_type priceCur, uint16_t volume, uint16_t price);
    void request_offers(packet_type type, currency_type volumeCur, currency_type priceCur);
};

#endif // CLIENT_H
