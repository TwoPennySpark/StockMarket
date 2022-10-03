#include "server.h"

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

void SMServer::on_message(pConnection netClient, tps::net::message<packet_type> &msg)
{
    auto newPkt = sm_packet::create(msg);
    if (!newPkt)
        return;

    auto type = newPkt->type;
    if (type == SM_CONNECT)
    {
        std::cout << "\n\t{CONNECT}\n";
        m_core.add_client(netClient);
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
            std::cout << "\n\t{ACTIVE OFFS REQUEST}\n";
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
            std::cout << "\n\t{ERROR}\n";
            m_core.delete_client(netClient);
            break;
        case SM_REQ_BALANCE_ACK:
        case SM_OFFS_ACK:
        default:
            std::cout << "\n\t{UNEXPECTED TYPE}: "<< type << "\n";
            break;
    }
}

void SMServer::handle_publish_off(pClient &client, sm_publish &pkt)
{
    offer_t newOffer(client, pkt);
    m_core.add_offer(newOffer);
}

void SMServer::handle_balance_req(pClient &client, sm_req_balance &pkt)
{
    sm_req_balance_ack ack;

    for (auto cur: pkt.vCur)
    {
        auto it = client->hBalance.find(cur);
        if (it != client->hBalance.end())
            ack.vBalance.emplace_back(cur, it->second);
    }

    tps::net::message<packet_type> reply;
    ack.pack(reply);
    client->netClient->send(reply);
}

void SMServer::handle_active_offs_req(pClient &client, sm_req_offs &pkt)
{

}

void SMServer::handle_all_offs_req(pClient &client, sm_req_offs &pkt)
{
    sm_req_offs_ack ack;
    ack.volumeCur = pkt.volumeCur;
    ack.priceCur = pkt.priceCur;

    auto res = m_core.get_all_offers(pkt.volumeCur, pkt.priceCur);
    for (auto& offer: res)
        ack.vOffs.emplace_back(offer.type, offer.volume, offer.price);

    tps::net::message<packet_type> reply;
    ack.pack(reply);
    client->netClient->send(reply);
}

void SMServer::handle_past_offs_req(pClient &client, sm_req_offs &pkt)
{
    sm_req_offs_ack ack;
    ack.volumeCur = pkt.volumeCur;
    ack.priceCur = pkt.priceCur;

    auto res = m_core.get_client_past_offers(pkt.volumeCur, pkt.priceCur, client);
    for (auto& offer: res)
        ack.vOffs.emplace_back(offer.type, offer.volume, offer.price);

    tps::net::message<packet_type> reply;
    ack.pack(reply);
    client->netClient->send(reply);
}

void SMServer::handle_error(pClient &client)
{
    m_core.delete_client(client->netClient);
}
