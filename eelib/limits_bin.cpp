#include "limits_bin.h"
#include <algorithm>

void LimitsBin::make(const BookEntry& makeEntry){
    entries.push(makeEntry);
    _totalQty += makeEntry.qty;
}

void LimitsBin::take(BookEntry& takeEntry){
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

void LimitsBin::notifyMatch(long makeId, long takeId, int transferQty){
    notifier->matchFound(makeId, takeId, transferQty);
}
