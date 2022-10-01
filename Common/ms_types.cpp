#include "ms_types.h"
#include "net_message.h"

std::unique_ptr<sm_packet> sm_packet::create(tps::net::message<packet_type>& msg)
{
    std::unique_ptr<sm_packet> ret;
    packet_type type = msg.hdr.id;

    switch (packet_type(type))
    {
//        case SM_CONNECT:
//            ret = create<sm_connect>(type);
//            break;
//        case SM_CONNACK:
//            ret = create<sm_connack>(type);
//            break;
        case SM_PUBLISH:
            ret = create<sm_publish>(type);
            break;
        case SM_REQ_BALANCE:
            ret = create<sm_req_balance>(type);
            break;
        case SM_REQ_BALANCE_ACK:
            ret = create<sm_req_balance_ack>(type);
            break;
        case SM_REQ_PAST_OFFS:
        case SM_REQ_ALL_ACTIVE_OFFS:
        case SM_REQ_CLIENT_ACTIVE_OFFS:
            ret = create<sm_req_offs>(type);
            break;
        case SM_OFFS_ACK:
            ret = create<sm_req_offs>(type);
            break;
        default:
            ret = create<sm_packet>(type);
            break;
    }

    try {
        ret->unpack(msg);
    } catch (...) {
        ret = nullptr;
    }

    if (msg.data_left_to_pop())
        ret = nullptr;

    return ret;
}

void sm_packet::pack(tps::net::message<packet_type> &msg) const
{
    msg.hdr.id = type;
    msg.hdr.size = 0;
}

void sm_packet::unpack(tps::net::message<packet_type> &)
{

}

//void sm_connect::pack(tps::net::message<packet_type> &msg) const
//{
//    uint16_t clientNameLen = ID.size();
//    uint16_t clientPasswordLen = password.size();

//    msg.hdr.id = type;
//    msg.hdr.size = clientNameLen + clientPasswordLen;

//    msg << clientNameLen;
//    msg << ID;

//    msg << clientPasswordLen;
//    msg << password;
//}

//void sm_connect::unpack(tps::net::message<packet_type>& msg)
//{
//    uint16_t clientNameLen = 0;
//    msg >> clientNameLen;
//    ID.resize(clientNameLen);
//    msg >> ID;

//    uint16_t clientPasswordLen = 0;
//    msg >> clientPasswordLen;
//    password.resize(clientPasswordLen);
//    msg >> password;
//}

//void sm_connack::pack(tps::net::message<packet_type>& msg) const
//{
//    msg.hdr.id = type;
//    msg.hdr.size = sizeof(rc);
//    msg << rc;
//}

//void sm_connack::unpack(tps::net::message<packet_type>& msg)
//{
//    msg >> rc;
//}

void sm_publish::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
//    msg.hdr.size = sizeof(currency_type)*2 + sizeof(offerType) + sizeof(volume) + sizeof(price);
    msg << volumeCur << priceCur;
    msg << offerType << volume << price;
}

void sm_publish::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;
    msg >> offerType >> volume >> price;
}

void sm_req_balance::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
//    msg.hdr.size = sizeof(currency_type) * vCur.size();

    for (auto cur: vCur)
        msg << cur;
}

void sm_req_balance::unpack(tps::net::message<packet_type>& msg)
{
    auto nCur = msg.hdr.size / sizeof(currency_type);
    vCur.resize(nCur);

    for (auto& cur: vCur)
        msg >> cur;
}

void sm_req_balance_ack::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
//    msg.hdr.size = sizeof(uint16_t) * vBalance.size();
    for (auto amount: vBalance)
        msg << amount;
}

void sm_req_balance_ack::unpack(tps::net::message<packet_type>& msg)
{
    vBalance.resize(msg.hdr.size / sizeof(uint16_t));
    for (auto& amount: vBalance)
        msg >> amount;
}

void sm_req_offs::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
//    msg.hdr.size = sizeof(currency_type)*2;

    msg << volumeCur << priceCur;
}

void sm_req_offs::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;
}

void sm_req_offs_ack::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
//    msg.hdr.size = sizeof(currency_type)*2 + vOffs.size() *
//                   (sizeof(offer_type) + sizeof(uint16_t) * 2);
    for (auto [type, volume, price]: vOffs)
        msg << type << volume << price;
}

void sm_req_offs_ack::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;

    uint32_t nOffs = (msg.hdr.size - sizeof(currency_type) * 2) /
                     (sizeof(uint8_t) + sizeof(uint16_t) * 2);
    vOffs.resize(nOffs);
    for (auto& [type, volume, price]: vOffs)
        msg >> type >> volume >> price;
}
