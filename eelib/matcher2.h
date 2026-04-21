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

struct BookEntry{
    long ordId = -1;
    unsigned int qty = 0;

    BookEntry(const Order& order) : ordId(order.ordId), qty(order.qty){}
};

class LimitsBin{
    std::vector<BookEntry> bookEntries;
    unsigned long totalQty = 0;
    // Notifier* notifier

    inline void notifyMatch(long giveId, long takeId);

    public:
        void insert(const BookEntry entry){
            bookEntries.push_back(entry);
            totalQty += entry.qty;
        }

        void take(BookEntry entry);

        // void take(BookEntry, Notifier*)
            // match until provided book entry is full, or bin is empty
            // notify all matches
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