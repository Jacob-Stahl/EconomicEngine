#pragma once

#include "notifier2.h"
#include "limits_bin.h"
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <flat_map>

struct PriceBin{
    int price = 0;
    unsigned int totalQty = 0;
};

struct Depth{
    std::vector<PriceBin> bidBins;
    std::vector<PriceBin> askBins;
};

class Matcher{
    private:
        // Limit orders by price
        std::flat_map<int, LimitsBin> buyLimitBins;
        std::flat_map<int, LimitsBin> sellLimitBins;

        Spread spread;
        // depth?

        LimitsBin& getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins);
        void placeLimit(const Order& order);
        void placeMarket(const Order& order);
        void takeSells(BookEntry& takeEntry, int maxPrice);
        void takeBuys(BookEntry& takeEntry, int minPrice);

    public:
        void placeOrder(const Order& order);
        std::unique_ptr<Notifier> notifier;

};