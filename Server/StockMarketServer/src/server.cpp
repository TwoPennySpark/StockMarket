#include "server.h"
#include "offer_filter.h"

bool SMServer::on_client_connect(pConnection client)
{
    tps::net::message<packet_type> msg;
    msg.hdr.id = SM_CONNECT;

    m_qMessagesIn.push_back(tps::net::owned_message<packet_type>(
                            {std::move(client), std::move(msg)}));
    return true;
}

void SMServer::on_client_disconnect(pConnection client)
{
    tps::net::message<packet_type> msg;
    msg.hdr.id = SM_ERROR;

    m_qMessagesIn.push_back(tps::net::owned_message<packet_type>(
                            {std::move(client), std::move(msg)}));
}

void SMServer::on_message(pConnection netClient, tps::net::message<packet_type>& msg)
{
    auto newPkt = sm_packet::create(msg);
    if (!newPkt)
        return;

    auto type = newPkt->type;
    if (type == SM_CONNECT)
    {
        std::cout << "\n\t{CONNECT}\n";
        handle_connect(netClient);
        return;
    }

    auto res = m_core.find_client(netClient);
    if (!res)
        return;
    auto& client = res.value().get();

    switch (type)
    {
        case SM_PUBLISH:
            std::cout << "\n\t{PUBLISH OFFS REQUEST}\n";
            handle_publish_off(client, dynamic_cast<sm_publish&>(*newPkt));
            break;
        case SM_REQ_BALANCE:
            std::cout << "\n\t{BALANCE REQUEST}\n";
            handle_balance_req(client, dynamic_cast<sm_req_balance&>(*newPkt));
            break;
        case SM_REQ_CLIENT_ACTIVE_OFFS:
            std::cout << "\n\t{CLIENT's ACTIVE OFFS REQUEST}\n";
            handle_active_offs_req(client, dynamic_cast<sm_req_offs&>(*newPkt));
            break;
        case SM_REQ_ALL_ACTIVE_OFFS:
            std::cout << "\n\t{ALL ACTIVE OFFS REQUEST}\n";
            handle_all_offs_req(client, dynamic_cast<sm_req_offs&>(*newPkt));
            break;
        case SM_REQ_PAST_OFFS:
            std::cout << "\n\t{PAST OFFS REQUEST}\n";
            handle_past_offs_req(client, dynamic_cast<sm_req_offs&>(*newPkt));
            break;
        case SM_ERROR:
            std::cout << "\n\t{CLIENT DISCONNECT}\n";
            handle_error(client);
            break;
        default:
            std::cout << "\n\t{UNEXPECTED TYPE}: "<< type << "\n";
            break;
    }
}

void SMServer::send_reply(pConnection& netClient, sm_packet& reply)
{
    tps::net::message<packet_type> msg;
    reply.pack(msg);
    netClient->send(std::move(msg));
}

void SMServer::handle_connect(pConnection& netClient)
{
    m_core.add_client(netClient);

    sm_packet ack(SM_CONNACK);
    send_reply(netClient, ack);
}

void SMServer::handle_publish_off(pClient& client, sm_publish& pkt)
{
    offer_t newOffer(pkt.offerSide, pkt.volumeCur, pkt.priceCur,
                     pkt.volume, pkt.price, client);

    auto replyType = m_core.add_offer(newOffer) ? SM_PUBLISH_ACK : SM_PUBLISH_REJECTED_ACK;

    sm_packet ack(replyType);
    send_reply(client->netClient, ack);
}

void SMServer::handle_balance_req(pClient& client, sm_req_balance& pkt)
{
    sm_req_balance_ack ack;

    for (auto cur: pkt.vCur)
    {
        auto it = client->hBalance.find(cur);
        if (it != client->hBalance.end())
            ack.vBalance.emplace_back(cur, it->second);
    }

    send_reply(client->netClient, ack);
}

void SMServer::handle_active_offs_req(pClient& client, sm_req_offs& pkt)
{
    sm_req_offs_ack ack(SM_OFFS_ACK, pkt.volumeCur, pkt.priceCur);

    OfferCurrencyFilter curFilter(pkt.volumeCur, pkt.priceCur);
    OfferClientFilter clientFilter(*client);
    curFilter.SetNext(&clientFilter);

    auto offers = m_core.get_offers(curFilter);
    for (auto& offer: offers)
        ack.vOffs.emplace_back(offer->side, offer->volume, offer->price);

    send_reply(client->netClient, ack);
}

void SMServer::handle_all_offs_req(pClient& client, sm_req_offs& pkt)
{
    sm_req_offs_ack ack(SM_OFFS_ACK, pkt.volumeCur, pkt.priceCur);

    OfferCurrencyFilter curFilter(pkt.volumeCur, pkt.priceCur);
    auto offers = m_core.get_offers(curFilter);
    for (auto& offer: offers)
        ack.vOffs.emplace_back(offer->side, offer->volume, offer->price);

    send_reply(client->netClient, ack);
}

void SMServer::handle_past_offs_req(pClient& client, sm_req_offs& pkt)
{
    sm_req_offs_ack ack(SM_OFFS_ACK, pkt.volumeCur, pkt.priceCur);

    for (auto& offer: client->vPastOffers)
        ack.vOffs.emplace_back(offer.side, offer.volume, offer.price);

    send_reply(client->netClient, ack);
}

void SMServer::handle_error(pClient& client)
{
    m_core.del_client(client);
}
