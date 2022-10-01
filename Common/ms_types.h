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

//enum connack_rc: uint8_t
//{
//    SM_RC_OK,
//    SM_RC_WRONG_PASSWORD,
//    SM_RC_USER_ALREADY_LOGINED
//};

enum currency_type: uint8_t
{
    SM_CUR_USD,
    SM_CUR_RUB
};

enum offer_type { SM_BUY, SM_SELL };

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

//struct sm_connect: public sm_packet
//{
//    sm_connect(packet_type type = SM_CONNECT): sm_packet(type) {}

//    std::string ID;
//    std::string password;

//    void pack(tps::net::message<packet_type>&) const override;
//    void unpack(tps::net::message<packet_type>&) override;
//};

//struct sm_connack: public sm_packet
//{
//    sm_connack(packet_type type = SM_CONNACK): sm_packet(type){}

//    connack_rc rc;

//    void pack(tps::net::message<packet_type>&) const;
//    void unpack(tps::net::message<packet_type>&);
//};

struct sm_publish: public sm_packet
{
    sm_publish(packet_type type = SM_PUBLISH): sm_packet(type){}

    currency_type volumeCur, priceCur;
    offer_type offerType;
    uint16_t volume, price;

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

    std::vector<uint16_t> vBalance;

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

    currency_type volumeCur, priceCur;
    std::vector<std::tuple<offer_type, uint16_t, uint16_t>> vOffs;

    void pack(tps::net::message<packet_type>&) const;
    void unpack(tps::net::message<packet_type>&);
};

#endif
