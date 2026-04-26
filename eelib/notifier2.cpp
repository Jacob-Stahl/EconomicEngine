#include "notifier2.h"

bool Notifier2::getOrder(long ordId, Order& order) const{
    auto registryItem = orderRegistery.find(ordId);
    if(registryItem == orderRegistery.end()){
        return false;
    }
    else{
        order = registryItem->second;
        return true; 
    }
};

void Notifier2::matchFound(long makeId, long takeId, unsigned int transferQty){   
    Order make, take;
    getOrder(makeId, make);
    getOrder(takeId, take);

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

void Notifier2::cancelled(long ordId, unsigned int remainingQty){
    Order cancelledOrder;  
    getOrder(ordId, cancelledOrder);
    cancellations.push_back(cancelledOrder);
    orderRegistery.erase(ordId);
}