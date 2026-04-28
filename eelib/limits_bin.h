#pragma once

#include "order.h"
#include "notifier2.h"
#include <queue>

struct BookEntry{
    long ordId = -1;
    unsigned int qty = 0;
    bool isCancelled = false;
    BookEntry(const Order2& order) : 
        ordId(order.ordId), 
        qty(order.qty){}
};

struct StopEntry{
    BookEntry entry;
    TimeInForce timeInForce;
    OrdType type;
    int limitPrice;

    StopEntry(const BookEntry& entry_, TimeInForce timeInForce_, OrdType type_, int limitPrice_) :
        entry(entry_),
        timeInForce(timeInForce_),
        type(type_),
        limitPrice(limitPrice_){}
};

// TODO: Store all stops at this price, on this side. 
//      matcher will place all stops when stop price is hit

class LimitsBin{
    private:
        std::vector<StopEntry> dormantStops;
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
        bool cancel(long ordId, unsigned int& remainingQty);
        void addDormantStop(const StopEntry& dormantStop);
        const std::vector<StopEntry>& getDormantStops() const;
        void clearDormantStops();
};
