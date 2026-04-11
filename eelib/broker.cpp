#include "broker.h"

bool Broker::canPlaceOrder(const Order& order) const{
    auto& inventory = observeInventory(order.traderId);

    // TODO: BUY MARKETS are tricky, because the price is undefined.
    // Apparently this is not a trivial problem to solve
    
    if(order.side == BUY && order.type == LIMIT){
        if(inventory.netCash() >= (long)(order.qty * order.price)){
            return true;
        }
    }

    if(order.side == BUY && order.type == STOPLIMIT){
        if(inventory.netCash() >= (long)(order.qty * order.stopPrice)){
            return true;
        }
    }

    if(order.side == SELL){
        if(inventory.netQty(order.asset) >= order.qty){
            return true;
        }
    }

    return false;
}