#ifndef CORE_H
#define CORE_H

#include "ms_types.h"
#include <map>
#include <set>
#include <unordered_map>

typedef struct client client_t;

namespace tps::net { template <typename T> class connection; }

using currency_pair_key = uint32_t;
using pClient = std::shared_ptr<client_t>;
using pConnection = std::shared_ptr<tps::net::connection<packet_type>>;

typedef struct offer
{
    offer(pClient& client, sm_publish& pkt): rClient(*client)
    {
        type = pkt.offerType;
        volumeCur = pkt.volumeCur;
        priceCur = pkt.priceCur;
        volume = pkt.volume;
        price = pkt.price;
    }

    bool operator< (const offer& off) const
    { return price < off.price; }

    offer_type type;
    currency_type volumeCur, priceCur;
    mutable uint32_t volume;
    uint32_t price;

    client_t& rClient;
}offer_t;

typedef struct currency_pair
{
    // Cantor pairing function
    static currency_pair_key get_key(const currency_type c1,
                                     const currency_type c2)
    { return ((c1+c2)*(c1+c2+1))/2+c2; }

    std::multiset<offer_t> buy;
    std::multiset<offer_t> sell;
}currency_pair_t;

typedef struct client
{
    client(pConnection& _netClient): netClient(_netClient)
    {
        hBalance.emplace(SM_CUR_USD, 0);
        hBalance.emplace(SM_CUR_RUB, 0);
    }

    std::unordered_map<currency_type, int64_t> hBalance;
    std::unordered_map<currency_pair_key,
                       std::vector<std::reference_wrapper<const offer>>> hvrActiveOffers;
    std::unordered_map<currency_pair_key, std::vector<offer>> hvPastOffers;

    pConnection netClient;
}client_t;

typedef struct core
{
public:
    core() { offers[currency_pair::get_key(SM_CUR_USD, SM_CUR_RUB)]; }

    void add_client(pConnection &netClient);
    void delete_client(pConnection &netClient);
    std::optional<std::reference_wrapper<pClient>> find_client(pConnection& netClient);

    void add_offer(offer_t& newOffer);
    std::vector<offer> get_all_offers(currency_type volumeCur, currency_type priceCur);
    std::vector<offer> get_client_past_offers(currency_type volumeCur, currency_type priceCur, pClient& client);

private:
    std::unordered_map<pConnection, pClient> clients;
    std::unordered_map<currency_pair_key, currency_pair_t> offers;
}core_t;

#endif // CORE_H
