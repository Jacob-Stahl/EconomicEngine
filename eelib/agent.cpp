#include "agent.h"
#include <string>
#include "tick.h"
#include <algorithm>

Agent::Agent(long id) : traderId(id) {}

Action Agent::policy(const Observation& observation) {
    return Action();
}

// Consumer Implementation

Consumer::Consumer(long traderId_, std::shared_ptr<ConsumerState> state_)
    : Agent(traderId_), state(std::move(state_))
{}

Consumer::Consumer(long traderId_, std::string asset_, unsigned short maxPrice_,
    tick hungerDelay_)
    : Agent(traderId_), state(std::make_shared<ConsumerState>())
{
    state->asset = std::move(asset_);
    state->maxPrice = maxPrice_;
    state->hungerDelay = hungerDelay_;
}

Action Consumer::policy(const Observation& observation){
    
    // Get new limit price based on current hunger
    auto price = std::min(
        std::max(0l, (long)state->sinceLastFill.raw() - (long)state->hungerDelay.raw()
    ), (long)state->maxPrice);
    ++state->sinceLastFill;

    // qty always set to 1 to avoid partial fills
    Order order(state->asset, BUY, LIMIT, price, 1);
    
    if (state->orderOnBookId > 0) {
        return Action{order, state->orderOnBookId};
    } else {
        return Action{order};
    }
}

void Consumer::orderPlaced(long orderId, const tick now) {
    state->orderOnBookId = orderId;
}

void Consumer::orderCanceled(long orderId, const tick now){
    state->orderOnBookId = 0;
}

void Consumer::matchFound(const Match& match, const tick now) {
    state->sinceLastFill = tick{0};
    state->orderOnBookId = 0;
}

Action Consumer::lastWill(const Observation& observation){
    return Action(state->orderOnBookId); // Cancel order before death
}


// Producer Implementation

Producer::Producer(long traderId_, std::shared_ptr<ProducerState> state_)
    : Agent(traderId_), state(std::move(state_))
{}

Producer::Producer(long traderId_, std::string asset_, unsigned short preferedPrice_)
    : Agent(traderId_), state(std::make_shared<ProducerState>())
{
    state->asset = std::move(asset_);
    state->preferedPrice = preferedPrice_;
}

Action Producer::policy(const Observation& observation) {
    auto it = observation.assetSpreads.find(state->asset);

    // If asset spread is missing, trust a new orderbook is created for new asset
    Spread assetSpread = Spread();
    if (it != observation.assetSpreads.end()) {
        assetSpread = it->second;
    }

    // Reduce production if bids are missing
    if(assetSpread.bidsMissing){
        if (state->qtyPerTick > 0)
            --state->qtyPerTick;
    }
    else if(assetSpread.highestBid > state->preferedPrice){
        ++state->qtyPerTick;
    }
    else if(assetSpread.highestBid < state->preferedPrice){
        if (state->qtyPerTick > 0)
            --state->qtyPerTick;
    }

    Order order(state->asset, SELL, MARKET, 0, state->qtyPerTick);
    return Action{order};
}
