#include "matcher2.h"

void Matcher2::placeOrder(const Order2& order){
    notifier->registerOrder(order);

    if(order.type == LIMIT){
        placeLimit(order);
    }
    else if(order.type == MARKET){
        placeMarket(order);
    }
    else if(order.type == STOPLIMIT){
        placeStopLimit(order);
    }
    else if(order.type == STOP){
        placeStopLimit(order);
    }
}

void Matcher2::placeLimit(const Order2& order){
    BookEntry entry{order};
    if(order.side == BUY){

        // try to match if it crosses the spread
        if(!spread.asksMissing && spread.lowestAsk <= order.price){
            takeSells(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        // place on book
        auto& buyBin = getLimitsBin(order.price, buyLimitBins);
        buyBin.make(entry);

        // update spread
        if(order.price > spread.highestBid || spread.bidsMissing){
            spread.highestBid = order.price;
        }
        spread.bidsMissing = false;
    }
    else{ // SELL
        if(!spread.bidsMissing && spread.highestBid >= order.price){
            takeBuys(entry, order.price);
            if(entry.qty == 0){ return;}
        }

        auto& sellBin = getLimitsBin(order.price, sellLimitBins);
        sellBin.make(entry);

        if(order.price < spread.lowestAsk || spread.asksMissing){
            spread.lowestAsk = order.price;
        }
        spread.asksMissing = false;
    }
    // TODO notify placement?
}

void Matcher2::placeMarket(const Order2& order){
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

void Matcher2::placeStopLimit(const Order2& order){

}

void Matcher2::placeStop(const Order2& order){
    
}

inline LimitsBin& Matcher2::getLimitsBin(int price, std::flat_map<int, LimitsBin>& bins){
    auto [it, _] = bins.try_emplace(price, notifier.get());
    return it->second;
}

void Matcher2::takeSells(BookEntry& buyOrder, int maxPrice){
    // Iterating flat maps is a bit different than regular maps:
    // https://stackoverflow.com/questions/79847808/how-can-i-iterate-a-flat-map-in-a-range-based-for-loop-updating-values
    // https://stackoverflow.com/questions/13230480/what-is-the-meaning-of-a-variable-with-type-auto
    for(auto&& [price, bin] : sellLimitBins){
        if(buyOrder.qty > 0 && price <= maxPrice){
            bin.take(buyOrder); // first fill the order
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

void Matcher2::takeBuys(BookEntry& sellOrder, int minPrice){
    for(auto bin = buyLimitBins.rbegin(); bin != buyLimitBins.rend(); bin++){
        auto&& [price, limitsBin] = *bin;
        if(sellOrder.qty > 0 && price >= minPrice){
            limitsBin.take(sellOrder);
        }
        if(limitsBin.totalQty() == 0){
            continue;
        }
        
        spread.bidsMissing = false;
        spread.highestBid = price;
        return;
    }

    spread.bidsMissing = true;
}

void Matcher2::cancelOrder(long ordId){
    Order2 doomedOrder;
    bool orderOnBook = notifier->getOrder(ordId, doomedOrder);
    if(!orderOnBook){
        return;
    }

    unsigned int remainingQty = 0;
    if(doomedOrder.side == BUY){
        auto& bin = buyLimitBins.at(doomedOrder.price);
        bin.cancel(ordId, remainingQty);
    }
    else{
        auto& bin = sellLimitBins.at(doomedOrder.price);
        bin.cancel(ordId, remainingQty);
    }

    notifier->cancelled(ordId, remainingQty);
}