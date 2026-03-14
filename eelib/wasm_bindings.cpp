#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "order.h"
#include "matcher.h"
#include "agent.h" 
#include "abm.h"

using namespace emscripten;

// Helper to manage unique_ptr transfer from JS
long abm_add_agent(ABM& abm, Agent* agent) {
    return abm.addAgent(std::unique_ptr<Agent>(agent));
}

val action_get_orders_to_place(const Action& action) {
    val array = val::array();
    for (std::size_t i = 0; i < action.ordersToPlace.size(); ++i) {
        array.set(i, val(action.ordersToPlace[i]));
    }
    return array;
}

val action_get_order_ids_to_cancel(const Action& action) {
    val array = val::array();
    for (std::size_t i = 0; i < action.orderIdsToCancel.size(); ++i) {
        array.set(i, val(action.orderIdsToCancel[i]));
    }
    return array;
}

std::size_t action_num_orders_to_place(const Action& action) {
    return action.ordersToPlace.size();
}

std::size_t action_num_order_ids_to_cancel(const Action& action) {
    return action.orderIdsToCancel.size();
}

Order action_get_order_to_place(const Action& action, std::size_t index) {
    return action.ordersToPlace.at(index);
}

long action_get_order_id_to_cancel(const Action& action, std::size_t index) {
    return action.orderIdsToCancel.at(index);
}

void action_clear(Action& action) {
    action.ordersToPlace.clear();
    action.orderIdsToCancel.clear();
}

EMSCRIPTEN_BINDINGS(eelib_module) {
    enum_<OrdType>("OrdType")
        .value("MARKET", OrdType::MARKET)
        .value("LIMIT", OrdType::LIMIT)
        .value("STOP", OrdType::STOP)
        .value("STOPLIMIT", OrdType::STOPLIMIT);

    enum_<Side>("Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL);

    // Bind tick so we can pass it to Consumer
    class_<tick>("tick")
        .constructor<unsigned long>()
        .function("raw", &tick::raw);

    value_object<Order>("Order")
        .field("traderId", &Order::traderId)
        .field("ordId", &Order::ordId)
        .field("side", &Order::side)
        .field("qty", &Order::qty)
        .field("price", &Order::price)
        .field("stopPrice", &Order::stopPrice)
        .field("asset", &Order::asset)
        .field("type", &Order::type);

    value_object<Spread>("Spread")
        .field("bidsMissing", &Spread::bidsMissing)
        .field("asksMissing", &Spread::asksMissing)
        .field("highestBid", &Spread::highestBid)
        .field("lowestAsk", &Spread::lowestAsk);

    value_object<PriceBin>("PriceBin")
        .field("price", &PriceBin::price)
        .field("totalQty", &PriceBin::totalQty);

    // Register std::vector types used in Depth
    register_vector<Order>("VectorOrder");
    register_vector<PriceBin>("VectorPriceBin");
    register_vector<long>("VectorLong");

    value_object<Depth>("Depth")
        .field("bidBins", &Depth::bidBins)
        .field("askBins", &Depth::askBins);

    // Bindings for Observation and Action
    register_map<std::string, Spread>("MapStringSpread");
    register_map<std::string, Depth>("MapStringDepth");

    value_object<Observation>("Observation")
        .field("time", &Observation::time)
        .field("assetSpreads", &Observation::assetSpreads)
        .field("assetOrderDepths", &Observation::assetOrderDepths);

    class_<Action>("Action")
        .constructor<>()
        .function("addOrder", &Action::addOrder)
        .function("addCancellation", &Action::addCancellation)
        .function("empty", &Action::empty)
        .function("clear", &action_clear)
        .function("numOrdersToPlace", &action_num_orders_to_place)
        .function("numOrderIdsToCancel", &action_num_order_ids_to_cancel)
        .function("getOrderToPlace", &action_get_order_to_place)
        .function("getOrderIdToCancel", &action_get_order_id_to_cancel)
        .function("getOrdersToPlace", &action_get_orders_to_place)
        .function("getOrderIdsToCancel", &action_get_order_ids_to_cancel);


    // Agents
    class_<Agent>("Agent")
        .property("traderId", &Agent::traderId);

    class_<Producer, base<Agent>>("Producer")
        .constructor<long, std::string, unsigned short>();

    class_<Consumer, base<Agent>>("Consumer")
        .constructor<long, std::string, unsigned short, tick>();

    class_<ABM>("ABM")
        .constructor<>()
        .function("simStep", &ABM::simStep)
        .function("addAgent", &abm_add_agent, allow_raw_pointers())
        .function("getNumAgents", &ABM::getNumAgents)
        .function("getLatestObservation", &ABM::getLatestObservation);
}
