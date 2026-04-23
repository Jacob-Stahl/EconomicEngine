#pragma once

#include "order.h"
#include <queue>

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

    void notifyMatch(long makeId, long takeId, int transferQty);

    public:

        unsigned long totalQty() const {return _totalQty; }

        void make(const BookEntry& makeEntry);
        void take(BookEntry& takeEntry);
};
