#include "agent.h"
#include <string>
#include "tick.h"
#include <algorithm>

Agent::Agent(long id) : traderId(id) {}

Action Agent::policy(const Observation& observation) {
    return Action();
}

// Consumer Implementation

Consumer::Consumer(long traderId_, std::string asset_, unsigned short maxPrice_, 
    tick hungerDelay_): 
    Agent(traderId_), 
    sinceLastFill(tick(0)),
    maxPrice(maxPrice_), 
    hungerDelay(hungerDelay_),
    asset(asset_),
    orderOnBookId(0)
{}

Action Consumer::policy(const Observation& observation){
    
    // Get new limit price based on current hunger
    auto price = std::min(
        std::max(0l, (long)sinceLastFill.raw() - (long)hungerDelay.raw()
    ), (long)maxPrice);
    ++sinceLastFill;

    // qty always set to 1 to avoid partial fills
    Order order(asset, BUY, LIMIT, price, 1);
    
    if (orderOnBookId > 0) {
        return Action{order, orderOnBookId};
    } else {
        return Action{order};
    }
}

void Consumer::orderPlaced(long orderId, const tick now) {
    orderOnBookId = orderId;
}

void Consumer::orderCanceled(long orderId, const tick now){
    orderOnBookId = 0;
}

void Consumer::matchFound(const Match& match, const tick now) {
    sinceLastFill = tick{0};
}

Action Consumer::lastWill(const Observation& observation){
    return Action(orderOnBookId); // Cancel order before death
}


// Producer Implementation

Producer::Producer(long traderId_, std::string asset_, unsigned short preferedPrice_):
    Agent(traderId_),
    asset(asset_),
    preferedPrice(preferedPrice_)
{}

Action Producer::policy(const Observation& observation) {
    auto it = observation.assetSpreads.find(asset);

    // If asset spread is missing, trust a new orderbook is created for new asset
    Spread assetSpread = Spread();
    if (it != observation.assetSpreads.end()) {
        assetSpread = it->second;
    }

    // Reduce production if bids are missing
    if(assetSpread.bidsMissing){
        if (qtyPerTick > 0)
            --qtyPerTick;
    }
    else if(assetSpread.highestBid > preferedPrice){
        ++qtyPerTick;
    }
    else if(assetSpread.highestBid < preferedPrice){
        if (qtyPerTick > 0)
            --qtyPerTick;
    }

    Order order(asset, SELL, MARKET, 0, qtyPerTick);
    return Action{order};
}