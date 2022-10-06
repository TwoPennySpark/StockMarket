#include "core.h"
#include "offer_filter.h"

bool operator< (const listIt& lhs, const listIt& rhs)
{
    return lhs->price < rhs->price;
}

bool operator> (const listIt& lhs, const listIt& rhs)
{
    return lhs->price > rhs->price;
}

std::vector<listIt> Core::get_offers(const OfferFilter& filter)
{
    std::vector<listIt> res;
    for (auto it = m_lActiveOffers.begin(); it != m_lActiveOffers.end(); it++)
        if (filter.handle(*it))
            res.emplace_back(it);
    return res;
}

auto Core::get_offers_sorted_by_price(const OfferFilter& filter,
                                      const std::function<bool(listIt, listIt)> cmp)
{
    std::multiset<listIt, decltype(cmp)> res(cmp);
    for (auto it = m_lActiveOffers.begin(); it != m_lActiveOffers.end(); it++)
        if (filter.handle(*it))
            res.emplace(it);
    return res;
}

std::optional<std::reference_wrapper<pClient>> Core::find_client(pConnection& netClient)
{
    auto it = m_hClients.find(netClient);
    if (it != m_hClients.end())
        return it->second;

    return std::nullopt;
}

void Core::add_client(pConnection& netClient)
{
    m_hClients.emplace(netClient, std::make_shared<client_t>(netClient));
}

void Core::del_client(pClient& client)
{
    // find offers that belong to client and delete them
    OfferClientFilter filter(*client);
    auto offers = get_offers(filter);
    for (auto& offer: offers)
        m_lActiveOffers.erase(offer);

    m_hClients.erase(client->netClient);
}

bool Core::add_offer(offer_t& newOffer)
{
    if (!newOffer.volume || !newOffer.price)
        return false;

    auto isSelling = (newOffer.side == SM_SELL);

    // make sure that new offer's completion won't exceed client's balance cap
    auto priceCurBalance = newOffer.rClient.hBalance[newOffer.priceCur];
    auto volumeCurBalance = newOffer.rClient.hBalance[newOffer.volumeCur];
    if (isSelling)
    {
        if (volumeCurBalance - int64_t(newOffer.volume) < MIN_BALANCE ||
            priceCurBalance  + int64_t(newOffer.volume) * int64_t(newOffer.price) > MAX_BALANCE)
            return false;
    }
    else
    {
        if (volumeCurBalance + int64_t(newOffer.volume) > MAX_BALANCE ||
            priceCurBalance  - int64_t(newOffer.volume) * int64_t(newOffer.price) < MIN_BALANCE)
            return false;
    }

    // get all offers with certain currency pair and offer side
    OfferCurrencyFilter curFilter(newOffer.volumeCur, newOffer.priceCur);
    OfferSideFilter sideFilter((newOffer.side == SM_BUY) ? SM_SELL : SM_BUY);
    curFilter.SetNext(&sideFilter);
    // sort buying  offers by prices in decreasing order when selling and
    // sort selling offers by prices in increasing order when buying
    auto offers = isSelling ? get_offers_sorted_by_price(curFilter, std::greater<listIt>()) :
                              get_offers_sorted_by_price(curFilter, std::less<listIt>());

    bool bCompleted = false;
    auto it = offers.begin();
    // iterate through offers with opposite side
    while (it != offers.end())
    {
        // if prices do not cross each other
        if ((isSelling && (*it)->price < newOffer.price) ||
           (!isSelling && (*it)->price > newOffer.price))
            break;

        auto& offer = **it;
        auto isGreater = offer.volume < newOffer.volume;
        auto isLess = offer.volume > newOffer.volume;

        // match offers with overlapping prices
        match_offers(newOffer, offer);

        if (isGreater)
        {
            // erase offer from active list if it's fully completed
            m_lActiveOffers.erase(*it);
        }
        else if (isLess)
        {
            bCompleted = true;
            break;
        }
        else
        {
            // erase offer from active list if it's fully completed
            m_lActiveOffers.erase(*it);
            bCompleted = true;
            break;
        }

        it++;
    }
    // if there is still volume to sell/buy - add new offer to active offers list
    if (!bCompleted)
        m_lActiveOffers.emplace_back(std::move(newOffer));

    return true;
}

void Core::match_offers(offer_t& lhs, offer_t& rhs) const
{
    if (lhs.side == rhs.side)
        return;

    if (lhs.volumeCur != rhs.volumeCur || lhs.priceCur != rhs.priceCur)
        return;

    auto& volumeCur = lhs.volumeCur;
    auto& priceCur = lhs.priceCur;

    auto& sellOffer = (lhs.side == SM_SELL) ? lhs : rhs;
    auto& buyOffer = (&sellOffer == &lhs) ? rhs : lhs;
    auto& seller = sellOffer.rClient;
    auto& buyer = buyOffer.rClient;

    // modify each client's balance
    auto exchangeVolume = (sellOffer.volume >= buyOffer.volume) ? buyOffer.volume : sellOffer.volume;
    seller.hBalance[volumeCur] -= exchangeVolume;
    seller.hBalance[priceCur]  += exchangeVolume * buyOffer.price;
    buyer.hBalance [volumeCur] += exchangeVolume;
    buyer.hBalance [priceCur]  -= exchangeVolume * buyOffer.price;

    if (sellOffer.volume > buyOffer.volume)
    {
        // add partial deal to client's history
        offer_t partialDeal(sellOffer);
        partialDeal.volume = exchangeVolume;
        partialDeal.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(partialDeal));

        sellOffer.volume -= buyOffer.volume;

        // add fully completed deal to client's history
        buyer.vPastOffers.emplace_back(std::move(buyOffer));
    }
    else if (sellOffer.volume < buyOffer.volume)
    {
        // add partial deal to client's history
        offer_t partialDeal(buyOffer);
        partialDeal.volume = exchangeVolume;
        buyer.vPastOffers.emplace_back(std::move(partialDeal));

        buyOffer.volume -= sellOffer.volume;

        // add fully completed deal to client's history
        sellOffer.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(sellOffer));
    }
    else
    {
        // two offers complete each other fully - add both to history
        sellOffer.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(sellOffer));
        buyer.vPastOffers.emplace_back(std::move(buyOffer));
    }
}
