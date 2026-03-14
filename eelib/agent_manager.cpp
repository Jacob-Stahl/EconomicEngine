#include "agent_manager.h"
#include "agent.h"

#include <memory>
#include <vector>

// Consumer Manager
ConsumerManager::ConsumerManager(
    std::shared_ptr<ABM> abm_,
    std::string name_,
    std::string asset_): AgentManager(abm_, name_){
        asset = asset_;
        hungerDelayDist = std::normal_distribution<double>(0, 0);
        maxPriceDist = std::normal_distribution<double>(0, 0);
    };

void ConsumerManager::changeHungerDelay(unsigned long mean, unsigned long std){
    hungerDelayDist = std::normal_distribution<double>(mean, std);
};

void ConsumerManager::changeMaxPrice(unsigned short mean, unsigned short std){
    maxPriceDist = std::normal_distribution<double>(mean, std);
};

void ConsumerManager::resampleAgentHungerDelay(){
    for(auto state : states){
        state->hungerDelay = tick(hungerDelayDist(gen));
    }
};

void ConsumerManager::reampleAgentMaxPrice(){
    for(auto state : states){
        state->maxPrice = maxPriceDist(gen);
    }
};

std::unique_ptr<Agent> ConsumerManager::factory(){

    auto state = std::make_shared<ConsumerState>();
    state->hungerDelay = tick(hungerDelayDist(gen));
    state->maxPrice = maxPriceDist(gen);
    states.push_back(std::move(state));

    auto agent = std::make_unique<Consumer>(0, std::move(state));
    long traderId = abm->addAgent(std::move(agent));
    traderIdsUnderMgmt.push_back(traderId);
};



// Producer Manager