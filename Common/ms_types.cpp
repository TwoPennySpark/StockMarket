#include "ms_types.h"
#include "net_message.h"

std::unique_ptr<sm_packet> sm_packet::create(tps::net::message<packet_type>& msg)
{
    std::unique_ptr<sm_packet> ret;
    packet_type type = msg.hdr.id;

    switch (packet_type(type))
    {
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

void sm_publish::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
    msg << volumeCur << priceCur;
    msg << offerSide << volume << price;
}

void sm_publish::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;
    msg >> offerSide >> volume >> price;
}

void sm_req_balance::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
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
    for (auto [cur, amount]: vBalance)
        msg << cur << amount;
}

void sm_req_balance_ack::unpack(tps::net::message<packet_type>& msg)
{
    vBalance.resize(msg.hdr.size / (sizeof(currency_type) + sizeof(int64_t)));
    for (auto& [cur, amount]: vBalance)
        msg >> cur >> amount;
}

void sm_req_offs::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;
    msg << volumeCur << priceCur;
}

void sm_req_offs::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;
}

void sm_req_offs_ack::pack(tps::net::message<packet_type>& msg) const
{
    msg.hdr.id = type;

    msg << volumeCur << priceCur;
    for (auto [side, volume, price]: vOffs)
        msg << side << volume << price;
}

void sm_req_offs_ack::unpack(tps::net::message<packet_type>& msg)
{
    msg >> volumeCur >> priceCur;

    uint32_t nOffs = msg.hdr.size / (sizeof(uint8_t) + sizeof(uint32_t) * 2);
    if (nOffs)
        vOffs.resize(nOffs);
    for (auto& [side, volume, price]: vOffs)
        msg >> side >> volume >> price;
}
