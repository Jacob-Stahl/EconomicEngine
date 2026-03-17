# pragma once

#include <memory>
#include <unordered_set>
#include <vector>
#include "matcher.h"
#include "agent.h"


class TickCallback{
    public:
        /// @brief Called after the end of every tick
    virtual ~TickCallback() = default;
    virtual void callBackAction() = 0;
};

/// @brief Agent Based Model. Framework for multi agent trading simulations.
class ABM{

    const int cleanupCancelledEvery = 16;

    // Agents
    std::vector<std::unique_ptr<Agent>> agents;
    tick tickCounter{0};
    long nextTraderId = 1;
    long nextOrderId = 1;

    /// @brief Asset - Matcher
    std::unordered_map<std::string, Matcher> orderMatchers;
    InMemoryNotifier notifier{};

    Observation latestObservation;

    void cancelOrderWithAllMatchers(long doomedOrderId);
    void cleanupCanceledOrdersWithAllMatchers();
    void addMatcherIfNeeded(const std::string& asset);
    void routeMatches(std::vector<Match>& matches);
    void observe();
    void removeAgents(const std::vector<size_t>& agentsToRemove);
    void runTickCallbacks();

    public:
        TickCallback* addTickCallback(std::unique_ptr<TickCallback> callback);
        void removeTickCallback(TickCallback* callback);

        ABM() = default;
        void simStep();
        long addAgent(std::unique_ptr<Agent> newAgent);
        void removeAgents(std::vector<long>& traderIdsToRemove);
        
        size_t getNumAgents() const { return agents.size(); }
        const Observation& getLatestObservation() {return latestObservation; };

    private:
        std::vector<std::unique_ptr<TickCallback>> tickCallbacks{};
};