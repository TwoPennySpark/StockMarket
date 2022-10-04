#include "client.h"

void SMClient::send_pkt(sm_packet &pkt)
{
    tps::net::message<packet_type> msg;
    pkt.pack(msg);
    send(std::move(msg));
}

void SMClient::publish_new_offer(offer_side side, currency_type volumeCur, currency_type priceCur,
                                 uint32_t volume, uint32_t price)
{
    sm_publish pub;
    pub.offerSide = side;
    pub.volumeCur = volumeCur;
    pub.priceCur = priceCur;
    pub.volume = volume;
    pub.price = price;

    send_pkt(pub);
}

void SMClient::req_balance(const std::vector<currency_type> &curns)
{
    sm_req_balance req;
    for (auto cur: curns)
        req.vCur.emplace_back(cur);

    send_pkt(req);
}

void SMClient::req_offers(packet_type type, currency_type volumeCur, currency_type priceCur)
{
    sm_req_offs req(type);
    req.volumeCur = volumeCur;
    req.priceCur = priceCur;

    send_pkt(req);
}

