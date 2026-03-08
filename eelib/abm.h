# pragma once

#include <memory>
#include <unordered_map>
#include "matcher.h"
#include "agent.h"


class AgentSelector{
    public:
        virtual bool isSelected(const std::unique_ptr<Agent>& agent){return true; };
};

/// @brief Agent Based Model. Framework for multi agent trading simulations.
class ABM{

    const int cleanupCancelledEvery = 16;

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

    public:
        ABM() = default;
        void simStep();
        long addAgent(std::unique_ptr<Agent> newAgent);
        void removeAgents(AgentSelector& agentSelector);
        
        size_t getNumAgents() const { return agents.size(); }
        const Observation& getLatestObservation() {return latestObservation; };

};