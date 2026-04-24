#include "notifier2.h"
#include "match.h"

void Notifier::matchFound(long makeId, long takeId, int transferQty){   
    const Order& make = getOrder(makeId);
    const Order& take = getOrder(takeId);

    if(take.side == BUY){
        Match match{
            take, make, transferQty
        };
        matches.push_back(match);
    }
    else{
        Match match{
            make, take, transferQty
        };
        matches.push_back(match);
    }
}