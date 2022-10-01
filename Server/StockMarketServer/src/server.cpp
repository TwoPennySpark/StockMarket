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

//bool SMServer::on_first_message(pConnection netClient, tps::net::message<packet_type> &msg)
//{
//    if (msg.hdr.id == SM_CONNECT)
//        return true;
//    return false;
//}

void SMServer::on_message(pConnection netClient, tps::net::message<packet_type> &msg)
{
    auto newPkt = sm_packet::create(msg);
    if (!newPkt)
        return;

    auto type = newPkt->type;
    if (type == SM_CONNECT)
    {
        std::cout << "\n\t{CONNECT}\n";
//        handle_connect(netClient, dynamic_cast<sm_connect&>(*newPkt));
        m_core.add_new_client(netClient);
        return;
    }

    auto res = m_core.find_client(netClient);
    if (!res)
        return;
    auto& client = res.value().get();

    switch (type)
    {
        case SM_PUBLISH:
        {

            break;
        }
    }
}

//void SMServer::handle_connect(pConnection &netClient, sm_connect &pkt)
//{
//    pClient client;

//    sm_connack connack;
//    connack.rc = SM_RC_OK;

//    // if client with such ID already exists
//    if (auto res = m_core.find_client(pkt.ID))
//    {
//        auto& existingClient = res.value().get();

//        // if client is currently connected
//        if (existingClient->netClient)
//            connack.rc = SM_RC_USER_ALREADY_LOGINED;
//        else
//        {
//            if (existingClient->password != pkt.password)
//                connack.rc = SM_RC_WRONG_PASSWORD;
//            else
//                existingClient->netClient = netClient;
//        }
//    }
//    else
//        client = m_core.add_new_client(pkt.ID, netClient);

//    tps::net::message<packet_type> reply;
//    connack.pack(reply);
//    netClient->send(reply);

//    if (connack.rc)
//        netClient->disconnect();
//}
