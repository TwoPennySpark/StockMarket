#ifndef CORE_H
#define CORE_H

#include <iostream>
#include "ms_types.h"
#include <unordered_map>

typedef struct client client_t;

namespace tps::net {template <typename T> class connection;}

using pClient = std::shared_ptr<client_t>;
using pConnection = std::shared_ptr<tps::net::connection<packet_type>>;

typedef struct offer
{
    offer_type type;
    currency_type volumeCur, priceCur;
    uint16_t volume, price;

    client_t& client;
}offer_t;

typedef struct client
{
    client(pConnection& _netClient): netClient(_netClient){}

    std::unordered_map<currency_type, uint16_t> balance;
    std::vector<offer> vActiveOffers;
    std::vector<offer> vPastOffers;

    pConnection netClient;
}client_t;

typedef struct core
{
public:
    std::optional<std::reference_wrapper<pClient> > find_client(pConnection &netClient);
    pClient& add_new_client(pConnection &netClient);
    void delete_client(pClient& client);

private:
    std::unordered_map<pConnection, pClient> clients;
}core_t;

#endif // CORE_H
