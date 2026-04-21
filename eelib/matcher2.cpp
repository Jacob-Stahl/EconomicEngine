#include "matcher2.h"


void Matcher::placeOrder(const Order& order){
    if(order.type == LIMIT){
        placeLimit(order);
    }

}

void Matcher::placeLimit(const Order& order){

    LimitEntry entry{order};
    if(order.side == BUY){
        if(!spread.asksMissing && spread.lowestAsk >= order.price){
            // TODO Take what we can. return remaining entry qty
        }

        // Make
        auto& buyBin = getLimitsBin(order.price, buyLimitBins);
        buyBin.insert(entry);
        updateDepthAndSpread(BUY, order.price, entry.qty);
    }
    else{
        if(!spread.bidsMissing && spread.highestBid >= order.price){
            // TODO Take what we can. return remaining entry qty
        }

        // Make
        auto& sellBin = getLimitsBin(order.price, sellLimitBins);
        sellBin.insert(entry);   
        updateDepthAndSpread(SELL, order.price, entry.qty);
    }
}

inline LimitsBin& Matcher::getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins){
    if(bins.find(price) == bins.end()){
        LimitsBin newBin{};
        bins.emplace(price, newBin);
    }
    return bins.at(price);
}

void Matcher::updateDepthAndSpread(Side side, int price, long qtyChange){
    // TODO
};

