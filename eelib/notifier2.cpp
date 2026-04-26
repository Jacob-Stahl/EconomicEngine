#include "notifier2.h"

bool Notifier2::getOrder(long ordId, Order2& order) const{
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
    Order2 make, take;
    getOrder(makeId, make);
    getOrder(takeId, take);

    if(take.side == BUY){
        Match2 match{
            take, make, transferQty, make.price
        };
        matches.push_back(match);
    }
    else{
        Match2 match{
            make, take, transferQty, make.price
        };
        matches.push_back(match);
    }
}

void Notifier2::cancelled(long ordId, unsigned int remainingQty){
    Order2 cancelledOrder;  
    getOrder(ordId, cancelledOrder);
    cancellations.push_back(cancelledOrder);
    orderRegistery.erase(ordId);
}