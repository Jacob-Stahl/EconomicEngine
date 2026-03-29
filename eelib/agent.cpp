#include "agent.h"
#include <string>
#include "tick.h"
#include <algorithm>
#include <limits>

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

// Manufacturer Implementation
void Inventory::update(const Match& match){
    if(match.buyer.traderId == traderId){
        auto asset = match.buyer.asset;
        update(asset, 
            (match.qty), 
            match.buyer.price * match.qty
        );
    }
    if(match.seller.traderId == traderId){
        auto asset = match.seller.asset;
        update(asset, 
            -(match.qty),
            -match.seller.price * match.qty
        );
    }
};

void Inventory::update(const std::string& asset, int qtyChange, long cashChange){
    if(assets.find(asset) == assets.end()){
        assets.emplace(asset, qtyChange);
    }
    else{
        int newQty = assets.at(asset) + qtyChange;
        assets[asset] = newQty;
    }
    cashBalance += cashChange;
}

Manufacturer::Manufacturer(long traderId_, std::shared_ptr<ManufacturerState> state_)
    : Agent(traderId_), state(std::move(state_))
{
    state->inventory = Inventory(traderId_);
}

long Manufacturer::costOfProd(
    const Recipe& recipe,
    const Observation& observation) {
    long totalCost = recipe.cost;

    for (const auto& [asset, qty] : recipe.inputs) {
        auto spreadIt = observation.assetSpreads.find(asset);
        unsigned short bidPrice = 1;

        if (spreadIt != observation.assetSpreads.end() && !spreadIt->second.bidsMissing) {
            bidPrice = static_cast<unsigned short>(std::min<long>(

                // Add +1 tp bid price. We want to out bid the other buy limits.
                // This puts natural upward pressure on the price.
                static_cast<long>(spreadIt->second.highestBid) + 1,
                std::numeric_limits<unsigned short>::max()));
        }

        totalCost += static_cast<long>(qty) * bidPrice;
    }

    return totalCost;
}

long Manufacturer::saleRevenue(
    const Recipe& recipe,
    const Observation& observation) {
    long totalRevenue = 0;

    for (const auto& [asset, qty] : recipe.outputs) {
        auto spreadIt = observation.assetSpreads.find(asset);
        unsigned short salePrice = 0;

        if (spreadIt != observation.assetSpreads.end() && !spreadIt->second.bidsMissing) {
            salePrice = spreadIt->second.highestBid;
        }

        totalRevenue += static_cast<long>(qty) * salePrice;
    }

    return totalRevenue;
}

std::vector<Order> Manufacturer::procurementOrders(
    const Recipe& recipe,
    const Observation& observation) {
    std::vector<Order> orders;
    orders.reserve(recipe.inputs.size());

    for (const auto& [asset, requiredQty] : recipe.inputs) {
        long deficit = static_cast<long>(requiredQty) - state->inventory.qty(asset);
        if (deficit <= 0) {
            continue;
        }

        unsigned short bidPrice = 1;
        auto spreadIt = observation.assetSpreads.find(asset);
        if (spreadIt != observation.assetSpreads.end() && !spreadIt->second.bidsMissing) {
            bidPrice = static_cast<unsigned short>(std::min<long>(
                static_cast<long>(spreadIt->second.highestBid) + 1,
                std::numeric_limits<unsigned short>::max()));
        }

        orders.emplace_back(
            asset,
            BUY,
            LIMIT,
            bidPrice,
            static_cast<unsigned int>(std::min<long>(
                deficit,
                std::numeric_limits<unsigned int>::max())));
    }

    return orders;
}

void Manufacturer::craft() {
    if (state->recipe.inputs.empty()) {
        return;
    }

    long craftCount = std::numeric_limits<long>::max();
    bool hasPositiveInput = false;

    for (const auto& [asset, requiredQty] : state->recipe.inputs) {
        if (requiredQty <= 0) {
            continue;
        }

        hasPositiveInput = true;
        craftCount = std::min(
            craftCount,
            state->inventory.qty(asset) / static_cast<long>(requiredQty));
    }

    if (!hasPositiveInput || craftCount <= 0) {
        return;
    }

    for (const auto& [asset, requiredQty] : state->recipe.inputs) {
        if (requiredQty <= 0) {
            continue;
        }
        state->inventory.update(asset, -requiredQty * craftCount, 0);
    }

    for (const auto& [asset, producedQty] : state->recipe.outputs) {
        if (producedQty <= 0) {
            continue;
        }
        state->inventory.update(asset, producedQty * craftCount, 0);
    }
}

std::vector<Order> Manufacturer::sellOrders() {
    std::vector<Order> orders;
    orders.reserve(state->recipe.outputs.size());

    for (const auto& [asset, producedQty] : state->recipe.outputs) {
        (void)producedQty;
        long inventoryQty = state->inventory.qty(asset);
        if (inventoryQty <= 0) {
            continue;
        }

        orders.emplace_back(
            asset,
            SELL,
            MARKET,
            0,
            static_cast<unsigned int>(std::min<long>(
                inventoryQty,
                std::numeric_limits<unsigned int>::max())));
    }

    return orders;
}

Action Manufacturer::policy(const Observation& observation){

    // ensure inventory traderId is consistant with state traderId
    state->inventory.traderId = traderId;
    
    long prodCost = costOfProd(state->recipe, observation);
    long expectedSaleRevenue = saleRevenue(state->recipe, observation);
    std::vector<Order> orders;

    // If sale revenue > cost, place bids for remaining required feedstock
    if (expectedSaleRevenue > prodCost) {
        orders = procurementOrders(state->recipe, observation);
    }

    craft();

    auto productOrders = sellOrders();
    orders.insert(orders.end(), productOrders.begin(), productOrders.end());

    ++state->timeSinceLastSale;

    return Action(std::move(orders));
}

void Manufacturer::orderPlaced(long orderId, const tick now) {
}

void Manufacturer::matchFound(const Match& match, const tick now) {
    if(match.seller.traderId == traderId){
        state->timeSinceLastSale = tick(0);
    }
    state->inventory.update(match);
}

void Manufacturer::orderCanceled(long orderId, const tick now) {
}

Action Manufacturer::lastWill(const Observation& observation) {
    std::vector<long> doomedOrderIds;
    doomedOrderIds.reserve(state->placedOrders.size());

    for (const auto& [asset, orderId] : state->placedOrders) {
        if (orderId > 0) {
            doomedOrderIds.push_back(orderId);
        }
    }

    return Action(std::move(doomedOrderIds));
}