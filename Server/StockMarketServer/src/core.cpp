#include "core.h"
#include "offer_filter.h"

bool operator< (const std::list<offer>::iterator& lhs,
                const std::list<offer>::iterator& rhs)
{
    return lhs->price < rhs->price;
}

std::optional<std::reference_wrapper<pClient>> Core::find_client(pConnection &netClient)
{
    auto it = m_hClients.find(netClient);
    if (it != m_hClients.end())
        return it->second;

    return std::nullopt;
}

void Core::add_client(pConnection &netClient)
{
    m_hClients.emplace(netClient, std::make_shared<client_t>(netClient));
}

void Core::del_client(pClient &client)
{
    OfferClientFilter filter(*client);
    auto offers = get_offers(filter);
    for (auto& offer: offers)
        m_lActiveOffers.erase(offer);

    m_hClients.erase(client->netClient);
}

void Core::add_offer(offer_t &newOffer)
{
    // get all offers with certain currency pair and offer side
    OfferCurrencyFilter curFilter(newOffer.volumeCur, newOffer.priceCur);
    OfferSideFilter sideFilter((newOffer.side == SM_BUY) ? SM_SELL : SM_BUY);
    curFilter.SetNext(&sideFilter);
    auto offers = get_offers_sorted_by_price(curFilter);

    bool bExectuted = false;
    if (newOffer.side == SM_SELL)
    {
        auto& sellOffer = newOffer;

        auto it = offers.rbegin();
        while (it != offers.rend() && (*it)->price >= newOffer.price)
        {
            auto& buyOffer = **it;
            auto isGreater = buyOffer.volume < sellOffer.volume;
            auto isLess = buyOffer.volume > sellOffer.volume;

            match_offers(sellOffer, buyOffer);

            if (isGreater)
            {
                m_lActiveOffers.erase(*it);
            }
            else if (isLess)
            {
                bExectuted = true;
                break;
            }
            else
            {
                m_lActiveOffers.erase(*it);
                bExectuted = true;
                break;
            }
            it++;
        }
    }
    else
    {
        auto& buyOffer = newOffer;

        auto it = offers.begin();
        while (it != offers.end() && (*it)->price <= newOffer.price)
        {
            auto& sellOffer = **it;
            auto isGreater = buyOffer.volume > sellOffer.volume;
            auto isLess = buyOffer.volume < sellOffer.volume;

            match_offers(sellOffer, buyOffer);

            if (isGreater)
            {
                m_lActiveOffers.erase(*it);
            }
            else if (isLess)
            {
                bExectuted = true;
                break;
            }
            else
            {
                m_lActiveOffers.erase(*it);
                bExectuted = true;
                break;
            }
            it++;
        }
    }
    if (!bExectuted)
        m_lActiveOffers.emplace_back(std::move(newOffer));
}

std::vector<std::list<offer_t>::iterator> Core::get_offers(OfferFilter &filter)
{
    std::vector<std::list<offer_t>::iterator> res;
    for (auto it = m_lActiveOffers.begin(); it != m_lActiveOffers.end(); it++)
        if (filter.handle(*it))
            res.emplace_back(it);
    return res;
}

std::multiset<std::list<offer_t>::iterator> Core::get_offers_sorted_by_price(OfferFilter &filter)
{
    std::multiset<std::list<offer_t>::iterator> res;
    for (auto it = m_lActiveOffers.begin(); it != m_lActiveOffers.end(); it++)
        if (filter.handle(*it))
            res.emplace(it);
    return res;
}

void Core::match_offers(offer_t& lhs, offer_t& rhs)
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

    auto exchangeVolume = (sellOffer.volume >= buyOffer.volume) ? buyOffer.volume : sellOffer.volume;
    seller.hBalance[volumeCur] -= exchangeVolume;
    seller.hBalance[priceCur]  += exchangeVolume * buyOffer.price;
    buyer.hBalance [volumeCur] += exchangeVolume;
    buyer.hBalance [priceCur]  -= exchangeVolume * buyOffer.price;

    if (sellOffer.volume > buyOffer.volume)
    {
        offer_t partialDeal(sellOffer);
        partialDeal.volume = exchangeVolume;
        partialDeal.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(partialDeal));

        sellOffer.volume -= buyOffer.volume;

        buyer.vPastOffers.emplace_back(std::move(buyOffer));
    }
    else if (sellOffer.volume < buyOffer.volume)
    {
        offer_t partialDeal(buyOffer);
        partialDeal.volume = exchangeVolume;
        buyer.vPastOffers.emplace_back(std::move(partialDeal));

        buyOffer.volume -= sellOffer.volume;

        sellOffer.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(sellOffer));
    }
    else
    {
        sellOffer.price = buyOffer.price;
        seller.vPastOffers.emplace_back(std::move(sellOffer));
        buyer.vPastOffers.emplace_back(std::move(buyOffer));
    }
}
