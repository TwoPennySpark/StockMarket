#include "core.h"

std::optional<std::reference_wrapper<pClient>> core::find_client(pConnection &netClient)
{
    auto it = clients.find(netClient);
    if (it != clients.end())
        return it->second;

    return std::nullopt;
}

void core::add_client(pConnection &netClient)
{
//    auto newClient = std::make_shared<client_t>(netClient);
    clients.emplace(netClient, std::make_shared<client_t>(netClient));
}

void core::delete_client(pConnection &netClient)
{
    clients.erase(netClient);
    // erase active offers
}

void core::add_offer(offer_t &newOffer)
{
    auto volumeCur = newOffer.volumeCur;
    auto priceCur = newOffer.priceCur;

    auto key = currency_pair::get_key(newOffer.volumeCur, newOffer.priceCur);
    auto it = offers.find(key);
    if (it == offers.end())
        return;

    auto& hBuyingOffers = it->second.buy;
    auto& hSellingOffers = it->second.sell;
    if (newOffer.type == SM_SELL)
    {
        auto& sellOffer = newOffer;
        auto& seller = sellOffer.rClient;

        std::reverse_iterator revIt = hBuyingOffers.rbegin();
        while (revIt != hBuyingOffers.rend() && revIt->price >= newOffer.price && newOffer.volume)
        {
            auto& buyOffer = *revIt;
            auto& buyer = buyOffer.rClient;

            if (buyOffer.volume >= sellOffer.volume)
            {
                buyer.hBalance[volumeCur] += sellOffer.volume;
                buyer.hBalance[priceCur] -= sellOffer.volume * buyOffer.price;
                seller.hBalance[volumeCur] -= sellOffer.volume;
                seller.hBalance[priceCur] += sellOffer.volume * buyOffer.price;

                revIt->volume -= newOffer.volume;
                sellOffer.volume = 0;
                // todo: move newOffer to past

                break;
            }
            else
            {
                seller.hBalance[volumeCur] -= buyOffer.volume;
                seller.hBalance[priceCur] += buyOffer.volume * buyOffer.price;
                buyer.hBalance[volumeCur] += buyOffer.volume;
                buyer.hBalance[priceCur] -= buyOffer.volume * buyOffer.price;

                sellOffer.volume -= buyOffer.volume;
                buyOffer.volume = 0;
                // todo: move offer to past
                hBuyingOffers.erase(std::next(revIt).base()); // erase finished offer
            }

            revIt++;
        }

        if (newOffer.volume)
        {
            auto res = hSellingOffers.emplace(std::move(newOffer));
            auto res2 = seller.hvrActiveOffers.find(key);
            res2->second.emplace_back(*res);
        }
//        else {

//        }
    }
    else
    {
        auto& buyOffer = newOffer;
        auto& buyer = buyOffer.rClient;

        auto it2 = hSellingOffers.begin();
        while (it2 != hSellingOffers.end() && it2->price <= newOffer.price && newOffer.volume)
        {
            auto& sellOffer = *it2;
            auto& seller = sellOffer.rClient;

            if (sellOffer.volume >= buyOffer.volume)
            {
                sellOffer.volume -= buyOffer.volume;
                buyOffer.volume = 0; // move new offer to past
                // todo: move newOffer to past

                buyer.hBalance[volumeCur] += buyOffer.volume;
                buyer.hBalance[priceCur] -= buyOffer.volume * sellOffer.price;
                seller.hBalance[volumeCur] -= buyOffer.volume;
                seller.hBalance[priceCur] += buyOffer.volume * sellOffer.price;

                break;
            }
            else
            {
                buyOffer.volume -= sellOffer.volume;
                // todo: move offer to past
                hSellingOffers.erase(it2); // erase finished offer

                seller.hBalance[volumeCur] -= sellOffer.volume;
                seller.hBalance[priceCur] += sellOffer.volume * sellOffer.price;
                buyer.hBalance[volumeCur] += sellOffer.volume;
                buyer.hBalance[priceCur] -= sellOffer.volume * sellOffer.price;
            }

            it2++;
        }

        if (newOffer.volume)
            hBuyingOffers.emplace(std::move(newOffer));
//        else
//            buyer.hvPastOffers.emplace(key, std::move(buyOffer));
    }

}

std::vector<offer> core::get_all_offers(currency_type volumeCur, currency_type priceCur)
{
    std::vector<offer> res;

    auto key = currency_pair::get_key(volumeCur, priceCur);
    if (auto it = offers.find(key); it != offers.end())
    {
        for (auto& offer: it->second.buy)
            res.emplace_back(offer);
        for (auto& offer: it->second.sell)
            res.emplace_back(offer);
    }
    return res;
}

std::vector<offer> core::get_client_past_offers(currency_type volumeCur, currency_type priceCur, pClient &client)
{
    std::vector<offer> res;

    auto key = currency_pair::get_key(volumeCur, priceCur);
    if (auto it = client->hvPastOffers.find(key); it != client->hvPastOffers.end())
        for (auto& offer: it->second)
            res.emplace_back(offer);

    return res;
}
