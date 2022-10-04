#ifndef SM_TYPES_H
#define SM_TYPES_H

#include <memory>
#include <vector>

enum packet_type: uint8_t
{
    SM_CONNECT, //
    SM_CONNACK,
    SM_PUBLISH, //
    SM_REQ_BALANCE, //
    SM_REQ_BALANCE_ACK,
    SM_REQ_CLIENT_ACTIVE_OFFS, //
    SM_REQ_ALL_ACTIVE_OFFS,
    SM_REQ_PAST_OFFS,
    SM_OFFS_ACK,
    SM_ERROR
};

enum currency_type: uint8_t
{
    SM_CUR_USD,
    SM_CUR_RUB
};

enum offer_side: uint8_t { SM_BUY, SM_SELL };

namespace tps::net {template <typename T> struct message;}

struct sm_packet
{
    sm_packet() = default;
    sm_packet(packet_type _type): type(_type){}

    virtual ~sm_packet() = default;

    packet_type type;

    template <typename T>
    static std::unique_ptr<sm_packet> create(packet_type type)
    { return std::unique_ptr<T>(new T(type)); }

    static std::unique_ptr<sm_packet> create(tps::net::message<packet_type>&);

    virtual void pack(tps::net::message<packet_type>&) const;
    virtual void unpack(tps::net::message<packet_type>&);
};

struct sm_publish: public sm_packet
{
    sm_publish(packet_type type = SM_PUBLISH): sm_packet(type){}

    currency_type volumeCur, priceCur;
    offer_side offerSide;
    uint32_t volume, price;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

struct sm_req_balance: public sm_packet
{
    sm_req_balance(packet_type type = SM_REQ_BALANCE): sm_packet(type){}

    std::vector<currency_type> vCur;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

struct sm_req_balance_ack: public sm_packet
{
    sm_req_balance_ack(packet_type type = SM_REQ_BALANCE_ACK): sm_packet(type){}

    std::vector<std::pair<currency_type, int64_t>> vBalance;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

// SM_REQ_ACTIVE_OFFS, SM_REQ_PAST_OFFS, SM_REQ_ALL_OFFS
struct sm_req_offs: public sm_packet
{
    sm_req_offs(packet_type type): sm_packet(type){}

    currency_type volumeCur, priceCur;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

struct sm_req_offs_ack: public sm_packet
{
    sm_req_offs_ack(packet_type type = SM_OFFS_ACK): sm_packet(type){}
    sm_req_offs_ack(packet_type type, currency_type _volumeCur, currency_type _priceCur):
        sm_packet(type), volumeCur(_volumeCur), priceCur(_priceCur) {}

    currency_type volumeCur, priceCur;
    std::vector<std::tuple<offer_side, uint32_t, uint32_t>> vOffs;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

#endif
