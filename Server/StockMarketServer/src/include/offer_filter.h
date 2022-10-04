#ifndef OFFER_FILTER_H
#define OFFER_FILTER_H

#include "core.h"

class OfferFilter
{
public:
    OfferFilter* SetNext(OfferFilter *handler)
    {
        this->nextHandler = handler;
        return handler;
    }

    virtual bool handle(offer_t& offer)
    {
        if (nextHandler)
            return nextHandler->handle(offer);

        return true;
    }
    OfferFilter* nextHandler = nullptr;
};

class OfferCurrencyFilter: public OfferFilter
{
public:
    OfferCurrencyFilter(currency_type _volume, currency_type _price): volume(_volume), price(_price) {}

    bool handle(offer_t& offer)
    {
        if (offer.volumeCur == volume && offer.priceCur == price)
            return OfferFilter::handle(offer);
        else
            return false;
    }
private:
    currency_type volume, price;
};

class OfferClientFilter: public OfferFilter
{
public:
    OfferClientFilter(client_t& _client): rClient(_client){}

    bool handle(offer_t& offer)
    {
        if (&offer.rClient == &rClient)
            return OfferFilter::handle(offer);
        else
            return false;
    }
private:
    client_t& rClient;
};

class OfferSideFilter: public OfferFilter
{
public:
    OfferSideFilter(offer_side _side): side(_side){}

    bool handle(offer_t& offer)
    {
        if (offer.side == side)
            return OfferFilter::handle(offer);
        else
            return false;
    }
private:
    offer_side side;
};

#endif // OFFER_FILTER_H
