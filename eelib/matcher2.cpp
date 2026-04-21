#include "matcher2.h"

void Matcher::placeOrder(const Order& order){

    // Register Order
    orderRegistery.insert({order.ordId, order});

    if(order.type == LIMIT){
        placeLimit(order);
    }

}

void Matcher::placeLimit(const Order& order){
    BookEntry entry{order};
    if(order.side == BUY){
        if(!spread.asksMissing && spread.lowestAsk >= order.price){
            takeSells(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        auto& buyBin = getLimitsBin(order.price, buyLimitBins);
        buyBin.insert(entry);
    }
    else{
        if(!spread.bidsMissing && spread.highestBid >= order.price){
            takeBuys(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        auto& sellBin = getLimitsBin(order.price, sellLimitBins);
        sellBin.insert(entry);   
    }

    // TODO notify placement?
}

inline LimitsBin& Matcher::getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins){
    auto [it, _] = bins.try_emplace(price);
    return it->second;
}


// consider using generators here: https://en.cppreference.com/cpp/coroutine/generator

// TODO iterate through all price bins, 
// take until entry is filled, or limit price is reached. 
// update spread before return
// market orders have unbounded limits
void Matcher::takeSells(BookEntry& entry, int maxPrice = INT_MAX){

}

void Matcher::takeBuys(BookEntry& entry, int minPrice = INT_MIN){

}