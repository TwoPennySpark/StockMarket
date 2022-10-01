#include "client.h"

void SMClient::publish_new_offer(offer_type type, currency_type volumeCur, currency_type priceCur, uint16_t volume, uint16_t price)
{
    sm_publish pub;
    pub.offerType = type;
    pub.volumeCur = volumeCur;
    pub.priceCur = priceCur;
    pub.volume = volume;
    pub.price = price;

    tps::net::message<packet_type> msg;
    pub.pack(msg);
    send(std::move(msg));
}

void SMClient::request_offers(packet_type type, currency_type volumeCur, currency_type priceCur)
{
    sm_req_offs req(type);
    req.volumeCur = volumeCur;
    req.priceCur = priceCur;

    tps::net::message<packet_type> msg;
    req.pack(msg);
    send(std::move(msg));
}

