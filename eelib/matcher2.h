#pragma once

#include "order.h"
#include "match.h"
#include "notifier.h"
#include <vector>
#include <set>
#include <queue>
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

struct LimitEntry{
    long ordId = -1;
    unsigned int qty = 0;

    
    LimitEntry(const Order& order) : ordId(order.ordId), qty(order.qty){}
};

struct LimitsBin{
    std::vector<LimitEntry> orders;
};

class Matcher{

    // Limit orders by price
    std::flat_map<int, LimitsBin> buyLimitBins;
    std::flat_map<int, LimitsBin> sellLimitBins;

    Spread spread;
    Depth depth;

    LimitsBin& getLimitsBin(int price, const std::flat_map<int, LimitsBin>& bins);
    void placeLimit(const Order& order);

    public:
        // Try to fill
        // Place remainder
        // Dispatch notifications and spread/depth changes
        void placeOrder(const Order& order);
};