#ifndef CORE_H
#define CORE_H

#include "ms_types.h"
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <algorithm>

class OfferFilter;
typedef struct offer offer_t;
typedef struct client client_t;

namespace tps::net { template <typename T> class connection; }

using pClient = std::shared_ptr<client_t>;
using pConnection = std::shared_ptr<tps::net::connection<packet_type>>;

typedef struct client
{
    client(pConnection& _netClient): netClient(_netClient)
    {
        hBalance[SM_CUR_USD] = 0;
        hBalance[SM_CUR_RUB] = 0;
    }

    std::unordered_map<currency_type, int64_t> hBalance;
    std::vector<offer> vPastOffers;

    pConnection netClient;
}client_t;

typedef struct offer
{
    offer(client_t& _client): rClient(_client) {}
    offer(const offer_t& off) = default;

    offer(pClient& client, sm_publish& pkt): rClient(*client) //
    {
        side = pkt.offerSide;
        volumeCur = pkt.volumeCur;
        priceCur = pkt.priceCur;
        volume = pkt.volume;
        price = pkt.price;
    }

    friend bool operator< (const std::list<offer>::iterator& lhs,
                           const std::list<offer>::iterator& rhs);

    offer_side side;
    currency_type volumeCur, priceCur;
    mutable uint32_t volume;
    uint32_t price;

    client_t& rClient;
}offer_t;

class Core
{
public:
    // ========CLIENTS========
    void add_client(pConnection &netClient);
    void del_client(pClient &client);
    std::optional<std::reference_wrapper<pClient>> find_client(pConnection& netClient);

    // ========OFFERS=========
    void add_offer(offer_t &newOffer);
    std::vector<std::list<offer_t>::iterator> get_offers(OfferFilter& filter);
    std::multiset<std::list<offer_t>::iterator> get_offers_sorted_by_price(OfferFilter &filter);
    void match_offers(offer_t &lhs, offer_t &rhs);

private:
    std::unordered_map<pConnection, pClient> m_hClients;
    std::list<offer_t> m_lActiveOffers;
};

#endif // CORE_H
