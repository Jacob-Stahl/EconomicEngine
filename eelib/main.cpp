#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <unordered_map>
#include <type_traits>
#include <string>

#include "order.h"
#include "matcher.h"
#include "agent.h"
#include "abm.h"
#include "agent_manager.h"

void benchmarkMatcher();
void tinkerWithABM();

int main() {
    tinkerWithABM();
}


template <typename Enum, int maxValue>
Enum random_enum()
{
    static_assert(std::is_enum_v<Enum>);

    static std::mt19937 rng{std::random_device{}()};

    using U = std::underlying_type_t<Enum>;

    constexpr U min = static_cast<U>(1);
    constexpr U max = static_cast<U>(maxValue);

    std::uniform_int_distribution<U> dist(min, max);
    return static_cast<Enum>(dist(rng));
}

// weighted picker: weights.size() == number of enum values (ordered by underlying value starting at 1)
template<typename Enum>
Enum weighted_random_enum(const std::vector<double>& weights) {
    static_assert(std::is_enum_v<Enum>);
    static std::mt19937 rng{std::random_device{}()};
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    size_t idx = dist(rng);           // idx in [0, weights.size()-1]
    using U = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<U>(idx + 1)); // +1 because enums are assumed to start at 1
}

class OrderFactory{
    long int currentIdTimestamp = 1;
    std::mt19937 gen;
    std::uniform_int_distribution<long> qtyDistrib;
    std::normal_distribution<double> priceDistrib;
    std::normal_distribution<double> stopPriceFactorDistrib;
    double spreadFactor = 10;

    public:
        OrderFactory(){
            qtyDistrib = std::uniform_int_distribution<long>(1, 100);
            priceDistrib = std::normal_distribution<double>(1000, 100);
            stopPriceFactorDistrib = std::normal_distribution<double>(30, 10);
            
            std::random_device device;
            gen = std::mt19937(device());
        }

        Order newOrder(Side side, OrdType type, long qty, long price = 0, long stopPrice = 0){
            Order o{};
            o.traderId = currentIdTimestamp;
            o.ordId = currentIdTimestamp;
            o.side = side;
            o.qty = qty;
            o.price = price;
            o.stopPrice = stopPrice;
            o.asset = "TEST";
            o.type = type;
            o.ordNum = currentIdTimestamp;

            ++currentIdTimestamp;
            return o;
        }

        Order randomOrder(){
            Side side = random_enum<Side, 2>();
            OrdType ordType = weighted_random_enum<OrdType>(
                {
                    1.0, // MARKET
                    1.01, // LIMIT
                    0.0, // STOP
                    0.0, // STOPLIMIT
                }
            );
            long qty = qtyDistrib(gen);
            double price = priceDistrib(gen);
            double stopPriceFactor = stopPriceFactorDistrib(gen);
            double stopPrice;


            switch(side){
                case BUY:
                    price = price - spreadFactor;
                    stopPrice = price + stopPriceFactor;
                    break;
                case SELL:
                    price = price + spreadFactor; 
                    stopPrice = price - stopPriceFactor;
                    break;
            }
            
            return newOrder(side, ordType, qty, price, stopPrice);
        }
};
    
void benchmarkMatcher(){

    InMemoryNotifier notifier;
    Matcher matcher{&notifier};
    OrderFactory ordFactory{};

    int numOrders = 5000000;
    std::vector<Order> orders{};

    for(int i = 0; i <= numOrders; i++){
        orders.push_back(ordFactory.randomOrder());
    }

    std::cout << "Generated orders. Running benchmark..." << std::endl;
    size_t processed = 0;
    auto last_print = std::chrono::steady_clock::now();

    for (auto &order : orders) {
        matcher.addOrder(order);
        ++processed;

        auto now = std::chrono::steady_clock::now();
        if (now - last_print >= std::chrono::seconds(1)) {
            auto counts = matcher.getOrderCounts();
            auto spread = matcher.getSpread();
            std::cout << processed << " orders processed | "
                      << "MARKET:" << counts[MARKET]
                      << " LIMIT:" << counts[LIMIT]
                      << " STOP:" << counts[STOP]
                      << " STOPLIMIT:" << counts[STOPLIMIT]

                      << " | "
                      << "Matches found:" << notifier.matches.size()
                      << " | Spread:";

            if (spread.bidsMissing) {
                std::cout << " bidsMissing";
            } else {
                std::cout << " highestBid:" << spread.highestBid;
            }

            if (spread.asksMissing) {
                std::cout << " asksMissing";
            } else {
                std::cout << " lowestAsk:" << spread.lowestAsk;
            }

            std::cout << "\n";
            last_print = now;
        }
    }
    std::cout << "Done!" << std::endl;
    std::cout << "Matches Found: " << notifier.matches.size() << std::endl;
    std::cout << "Orders Rejected: " << notifier.placementFailedOrders.size() << std::endl;

};


void showObservations(const Observation& observations){
    constexpr int assetWidth = 16;
    constexpr int priceWidth = 10;

    auto printPrice = [](bool missing, unsigned short price) {
        if (missing) {
            std::cout << std::setw(priceWidth) << "-";
            return;
        }

        std::cout << std::setw(priceWidth) << price;
    };

    std::cout << "\x1B[2J\x1B[H";
    std::cout << "Tick: " << observations.time << "\n\n";
    std::cout << std::left
              << std::setw(assetWidth) << "Asset"
              << std::setw(priceWidth) << "Bid"
              << std::setw(priceWidth) << "Ask"
              << std::setw(priceWidth) << "Spread"
              << "Market"
              << "\n";
    std::cout << std::string(assetWidth + (priceWidth * 3) + 6, '-') << "\n";

    if (observations.assetSpreads.empty()) {
        std::cout << "No spreads available.\n";
        std::cout.flush();
        return;
    }

    for (const auto& [asset, spread] : observations.assetSpreads) {
        std::cout << std::left << std::setw(assetWidth) << asset;
        std::cout << std::right;
        printPrice(spread.bidsMissing, spread.highestBid);
        printPrice(spread.asksMissing, spread.lowestAsk);

        if (spread.bidsMissing || spread.asksMissing) {
            std::cout << std::setw(priceWidth) << "-";
        } else {
            std::cout << std::setw(priceWidth)
                      << (spread.lowestAsk - spread.highestBid);
        }

        std::cout << "  ";
        if (spread.bidsMissing && spread.asksMissing) {
            std::cout << "No bids or asks";
        } else if (spread.bidsMissing) {
            std::cout << "No bids";
        } else if (spread.asksMissing) {
            std::cout << "No asks";
        } else {
            std::cout << "Two-sided";
        }

        std::cout << "\n";
    }

    std::cout.flush();
}

void showManufacturerManagerState(const ManufacturerManager& manager){
    constexpr int agentWidth = 8;
    constexpr int ageWidth = 8;
    constexpr int cashWidth = 10;
    constexpr int assetWidth = 12;

    const auto& states = manager.getStates();
    const auto& recipe = manager.getRecipe();

    std::vector<std::string> trackedAssets;
    trackedAssets.reserve(recipe.inputs.size() + recipe.outputs.size());

    for (const auto& [asset, qty] : recipe.inputs) {
        (void)qty;
        trackedAssets.push_back(asset);
    }

    for (const auto& [asset, qty] : recipe.outputs) {
        (void)qty;
        if (std::find(trackedAssets.begin(), trackedAssets.end(), asset) == trackedAssets.end()) {
            trackedAssets.push_back(asset);
        }
    }

    std::cout << "\nRefinery: " << manager.name << "\n\n";
    std::cout << std::left
              << std::setw(agentWidth) << "Agent"
              << std::setw(ageWidth) << "Age"
              << std::setw(cashWidth) << "Cash";

    for (const auto& asset : trackedAssets) {
        std::cout << std::setw(assetWidth) << asset;
    }
    std::cout << "\n";

    std::cout << std::string(
        agentWidth + ageWidth + cashWidth + static_cast<int>(trackedAssets.size()) * assetWidth,
        '-') << "\n";

    if (states.empty()) {
        std::cout << "No refinery agents.\n";
        return;
    }

    for (size_t index = 0; index < states.size(); ++index) {
        const auto& state = states[index];
        std::cout << std::left
                  << std::setw(agentWidth) << index
                  << std::setw(ageWidth) << state->timeSinceLastSale
                  << std::setw(cashWidth) << state->inventory.cash();

        for (const auto& asset : trackedAssets) {
            std::cout << std::setw(assetWidth) << state->inventory.qty(asset);
        }

        std::cout << "\n";
    }

    std::cout << "\nRecipe: ";
    bool firstTerm = true;

    for (const auto& [asset, qty] : recipe.inputs) {
        if (!firstTerm) {
            std::cout << " + ";
        }
        std::cout << qty << ' ' << asset;
        firstTerm = false;
    }

    std::cout << " -> ";
    firstTerm = true;

    for (const auto& [asset, qty] : recipe.outputs) {
        if (!firstTerm) {
            std::cout << " + ";
        }
        std::cout << qty << ' ' << asset;
        firstTerm = false;
    }

    std::cout << " | Cost: " << recipe.cost << "\n";
}

void tinkerWithABM(){

    // Setup
    int numSteps = 10000;
    auto abm = std::make_shared<ABM>();

    Recipe refineOil({{"OIL", 2}}, {{"FUEL", 1}}, 5);

    auto driller = ProducerManager(abm, "OIL Producer", "OIL");
    driller.changeNumAgents(1);
    driller.changePreferedPrice(50, 0);
    auto refinery = ManufacturerManager(abm, "Refinery", refineOil);
    refinery.neutralAge = tick(10);
    refinery.staleAge = tick(100);
    refinery.changeNumAgents(10000);
    refinery.numAgentsFixed = true;
    
    auto consumers = ConsumerManager(abm, "FUEL Consumers", "FUEL");
    consumers.changeNumAgents(1000);
    consumers.changeHungerDelay(100, 40);
    consumers.changeMaxPrice(200, 0);

    
    // Show ABM initial state
    std::cout << "Num Agents: " << abm->getNumAgents() << std::endl;

    // Run
    for(int i = 0; i < numSteps; i++){
        abm->simStep();
        showObservations(abm->getLatestObservation());
        //showManufacturerManagerState(refinery);
    }
};