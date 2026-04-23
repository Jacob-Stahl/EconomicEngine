#pragma once

#include "match.h"
#include "notifier.h"
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

    std::unordered_map<long, Order> orderRegistery;

    // Limit orders by price
    std::flat_map<int, LimitsBin> buyLimitBins;
    std::flat_map<int, LimitsBin> sellLimitBins;

    Spread spread;
    // depth?

    LimitsBin& getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins);
    void placeLimit(const Order& order);
    void takeSells(BookEntry& entry, int maxPrice);
    void takeBuys(BookEntry& entry, int minPrice);

    public:
        // Try to fill
        // Place remainder
        // Dispatch notifications and spread/depth changes
        void placeOrder(const Order& order);
};