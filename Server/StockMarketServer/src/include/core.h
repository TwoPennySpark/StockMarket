#ifndef CORE_H
#define CORE_H

#include "ms_types.h"
#include <set>
#include <list>
#include <unordered_map>
#include <functional>

class OfferFilter;
typedef struct offer offer_t;
typedef struct client client_t;

namespace tps::net { template <typename T> class connection; }

using listIt = std::list<offer>::iterator;
using pClient = std::shared_ptr<client_t>;
using pConnection = std::shared_ptr<tps::net::connection<packet_type>>;

typedef struct client
{
    client(pConnection& _netClient): netClient(_netClient)
    {
        hBalance[SM_CUR_USD] = 0;
        hBalance[SM_CUR_RUB] = 0;
    }

    // first - currency, second - balance
    std::unordered_map<currency_type, int64_t> hBalance;
    // completed transactions are stored here
    std::vector<offer> vPastOffers;

    pConnection netClient;
}client_t;

typedef struct offer
{
    offer (offer_side _side, currency_type _volumeCur, currency_type _priceCur,
           uint32_t _volume, uint32_t _price, pClient client):
           side(_side), volumeCur(_volumeCur), priceCur(_priceCur),
           volume(_volume), price(_price), rClient(*client) {}

    friend bool operator< (const listIt& lhs, const listIt& rhs);
    friend bool operator> (const listIt& lhs, const listIt& rhs);

    offer_side side;
    uint32_t volume, price;
    currency_type volumeCur, priceCur;

    client_t& rClient;
}offer_t;

class Core
{
public:
    // ========CLIENTS========
    // create new client_t struct and bind it with connection object
    void add_client(pConnection& netClient);
    // delete client and all associated offers
    void del_client(pClient& client);
    // get reference to client
    std::optional<std::reference_wrapper<pClient>> find_client(pConnection& netClient);

    // ========OFFERS=========
    // match new offer to the rest, if it's not fully completed add it to active offers list
    bool add_offer(offer_t& newOffer);
    // return iterators to offers that satisfy the conditions of the filter chain
    std::vector<listIt> get_offers(const OfferFilter& filter);
    // same as before but sorted by offer's price using cmp func
    auto get_offers_sorted_by_price(const OfferFilter& filter,
                                    const std::function<bool(listIt, listIt)> cmp);
private:
    void match_offers(offer_t& lhs, offer_t& rhs) const;

    std::unordered_map<pConnection, pClient> m_hClients;
    std::list<offer_t> m_lActiveOffers;
};

#endif // CORE_H
