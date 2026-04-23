#pragma once

#include "order.h"
#include "match.h"
#include "notifier.h"
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <flat_map>
#include <queue>

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
    std::queue<BookEntry> entries;

    // TODO: Store all stops at this price, on this side. 
    //      matcher will place all stops when stop price is hit

    unsigned long _totalQty = 0;
    // Notifier* notifier

    inline void notifyMatch(long makeId, long takeId, int transferQty);

    public:

        unsigned long totalQty() const {return _totalQty; }

        void make(const BookEntry& makeEntry){
            entries.push(makeEntry);
            _totalQty += makeEntry.qty;
        }

        void take(BookEntry& takeEntry){
            while (takeEntry.qty > 0) {
                auto& makeEntry = entries.front();
                int transferQty = std::min(takeEntry.qty, makeEntry.qty);
                
                takeEntry.qty -= transferQty;
                makeEntry.qty -= transferQty;
                _totalQty -= transferQty;

                notifyMatch(makeEntry.ordId, takeEntry.ordId, transferQty);

                if(makeEntry.qty == 0){
                    entries.pop();
                }
            };
        }
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