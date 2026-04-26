#include "matcher2.h"

void Matcher::placeOrder(const Order& order){
    notifier->registerOrder(order);

    if(order.type == LIMIT){
        placeLimit(order);
    }
}

void Matcher::placeLimit(const Order& order){
    BookEntry entry{order};
    if(order.side == BUY){
        if(!spread.asksMissing && spread.lowestAsk <= order.price){
            takeSells(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        auto& buyBin = getLimitsBin(order.price, buyLimitBins);
        buyBin.make(entry);
    }
    else{ // SELL
        if(!spread.bidsMissing && spread.highestBid >= order.price){
            takeBuys(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        auto& sellBin = getLimitsBin(order.price, sellLimitBins);
        sellBin.make(entry);
    }
    // TODO notify placement?
}

void Matcher::placeMarket(const Order& order){
    BookEntry entry{order};
    if(order.side == BUY){
        if(!spread.asksMissing){
            takeSells(entry);
            if(entry.qty == 0){ return;}
        }
    }
    else{ // SELL
        if(!spread.bidsMissing){
            takeBuys(entry);
            if(entry.qty == 0){ return;}
        }
    }

    // cancel what remains of this market order
    notifier->cancelled(entry.ordId, entry.qty);
}

inline LimitsBin& Matcher::getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins){
    auto [it, _] = bins.try_emplace(price, notifier.get());
    return it->second;
}

void Matcher::takeSells(BookEntry& buyOrder, int maxPrice = INT_MAX){
    // Iterating flat maps is a bit different than regular maps:
    // https://stackoverflow.com/questions/79847808/how-can-i-iterate-a-flat-map-in-a-range-based-for-loop-updating-values
    // https://stackoverflow.com/questions/13230480/what-is-the-meaning-of-a-variable-with-type-auto
    for(auto&& [price, bin] : sellLimitBins){
        if(buyOrder.qty > 0 && price <= maxPrice){
            bin.take(buyOrder); // first fill the order
            continue;
        }
        if(bin.totalQty() == 0){
            continue; // then find a non-empty bin with the best asks
        }
        spread.asksMissing = false;
        spread.lowestAsk = price;
        return; // return after updating the spread
    }

    // If we reach this point, all ask liquidity has been drained
    spread.asksMissing = true;
}

void Matcher::takeBuys(BookEntry& sellOrder, int minPrice = INT_MIN){
    for(auto bin = buyLimitBins.rbegin(); bin != buyLimitBins.rend(); bin++){
        auto&& [price, limitsBin] = *bin;
        if(sellOrder.qty > 0 && price >= minPrice){
            limitsBin.take(sellOrder);
        }
        if(limitsBin.totalQty() == 0){
            continue;
        }
        
        spread.bidsMissing = true;
        spread.highestBid = price;
        return;
    }

    spread.bidsMissing = true;
}