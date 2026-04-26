#pragma once

#include "order.h"
#include "notifier2.h"
#include <queue>

struct BookEntry{
    long ordId = -1;
    unsigned int qty = 0;
    bool isCancelled = false;
    BookEntry(const Order& order) : ordId(order.ordId), qty(order.qty){}
};

// TODO: Store all stops at this price, on this side. 
//      matcher will place all stops when stop price is hit

class LimitsBin{
    private:
        std::deque<BookEntry> entries;
        unsigned int _totalQty = 0;
        Notifier2* notifier;
        void notifyMatch(long makeId, long takeId, unsigned int transferQty);

    public:

        /// @brief Takes raw pointer to notifier in Matcher2. LimitsBin has no effect on the nofifier lifetime
        /// @param _notifier 
        LimitsBin(Notifier2* _notifier): notifier(_notifier){};
        const unsigned int totalQty() const {return _totalQty; }
        void make(const BookEntry& makeEntry);
        void take(BookEntry& takeEntry);
        bool cancel(long ordId);
};
