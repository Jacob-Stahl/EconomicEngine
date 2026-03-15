#pragma once

#include "matcher.h"
#include "match.h"
#include <string>
#include <functional>
#include <map>
#include "tick.h"
#include <memory>
#include <vector>

struct Observation{
    tick time;

    /// @brief asset - Spread
    std::map<std::string, Spread> assetSpreads;
    std::map<std::string, Depth> assetOrderDepths;
};

struct Action{
    std::vector<Order> ordersToPlace;
    std::vector<long> orderIdsToCancel;

    Action() = default;

    explicit Action(const Order& order_){
        ordersToPlace.push_back(order_);
    }

    Action(const Order& order_, long doomedOrderId_){
        ordersToPlace.push_back(order_);
        orderIdsToCancel.push_back(doomedOrderId_);
    }

    explicit Action(long doomedOrderId_){
        orderIdsToCancel.push_back(doomedOrderId_);
    }

    explicit Action(std::vector<Order> ordersToPlace_)
        : ordersToPlace(std::move(ordersToPlace_))
    {}

    explicit Action(std::vector<long> orderIdsToCancel_)
        : orderIdsToCancel(std::move(orderIdsToCancel_))
    {}

    Action(std::vector<Order> ordersToPlace_, std::vector<long> orderIdsToCancel_)
        : ordersToPlace(std::move(ordersToPlace_)),
          orderIdsToCancel(std::move(orderIdsToCancel_))
    {}

    void addOrder(const Order& order_){
        ordersToPlace.push_back(order_);
    }

    void addCancellation(long doomedOrderId_){
        orderIdsToCancel.push_back(doomedOrderId_);
    }

    bool empty() const {
        return ordersToPlace.empty() && orderIdsToCancel.empty();
    }
};

class Agent{
    public:
        long traderId;
        Agent(long);
        virtual ~Agent() = default;

        virtual Action policy(const Observation& observation);

        virtual void matchFound(const Match& match, const tick now){};
        virtual void orderPlaced(long orderId, const tick now){};
        virtual void orderCanceled(long orderId, const tick now){};

        /// @brief Final action before agent is removed from ABM
        virtual Action lastWill(const Observation& observation){return Action();};
};


struct ConsumerState{
    std::string asset;
    tick sinceLastFill = tick(0);
    long orderOnBookId = 0;
    unsigned short maxPrice;
    tick hungerDelay = tick(0);
};

class Consumer : public Agent{

    private:
        std::shared_ptr<ConsumerState> state;

    public:
        Consumer(long traderId_, std::shared_ptr<ConsumerState> state_);
        Consumer(long traderId_, std::string asset_, unsigned short maxPrice_,
            tick hungerDelay_);
        Action policy(const Observation& observation) override;
        void orderPlaced(long orderId, const tick now) override;
        void matchFound(const Match& match, const tick now) override;
        void orderCanceled(long orderId, const tick now) override;
        Action lastWill(const Observation& observation) override;
};

struct ProducerState {
    std::string asset;
    unsigned short preferedPrice;
    unsigned int qtyPerTick = 1;
};

class Producer : public Agent{
    private:
        std::shared_ptr<ProducerState> state;

    public:
        Producer(long traderId_, std::shared_ptr<ProducerState> state_);
        Producer(long traderId_, std::string asset, 
            unsigned short preferedPrice);
        Action policy(const Observation& observation) override;
};

inline double fast_sigmoid(double x) {
    return x / (1 + std::abs(x));
};

struct Recipe {
    // Asset - Amount
    std::map<std::string, int> inputs;
    std::map<std::string, int> outputs;
};

class Inventory {
    long traderId;
    std::map<std::string, long> assets{};
    long cashBalance = 0;

    public:
        Inventory(long traderId_) : traderId(traderId_){};
        void update(const Match& match);
        void update(const std::string& asset, int qtyChange, long cashChange);

        int qty(const std::string& asset) const {
            auto it = assets.find(asset);
            return it == assets.end() ? 0 : it->second;
        }

        long cash() const {
            return cashBalance;
        }
};

struct ManufacturerState {
    Recipe recipe;
    unsigned short maxCost = 0;
    Inventory inventory{0};

    // Asset - OrderId
    std::map<std::string, long> placedOrders;
};

class Manufacturer : public Agent{
    private:
        std::shared_ptr<ManufacturerState> state;

        unsigned short costPerCraft(const Observation& observation);

    public:
        Manufacturer(long traderId_, std::shared_ptr<ManufacturerState> state);
        Action policy(const Observation& observation) override;
        void orderPlaced(long orderId, const tick now) override;
        void matchFound(const Match& match, const tick now) override;
        void orderCanceled(long orderId, const tick now) override;
        Action lastWill(const Observation& observation) override;
};