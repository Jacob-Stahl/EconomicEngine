#include "agent_manager.h"
#include "agent.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

namespace {

tick clampTickSample(double value) {
    return tick(static_cast<tick::rep>(std::clamp(
        value,
        0.0,
        static_cast<double>(std::numeric_limits<tick::rep>::max()))));
}

unsigned short clampUnsignedShortSample(double value) {
    return static_cast<unsigned short>(std::clamp(
        value,
        0.0,
        static_cast<double>(std::numeric_limits<unsigned short>::max())));
}

}

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
    resampleAgentHungerDelay();
};

void ConsumerManager::changeMaxPrice(unsigned short mean, unsigned short std){
    maxPriceDist = std::normal_distribution<double>(mean, std);
    reampleAgentMaxPrice();
};

void ConsumerManager::resampleAgentHungerDelay(){
    for(auto state : states){
        state->hungerDelay = clampTickSample(hungerDelayDist(gen));
    }
};

void ConsumerManager::reampleAgentMaxPrice(){
    for(auto state : states){
        state->maxPrice = clampUnsignedShortSample(maxPriceDist(gen));
    }
};

std::unique_ptr<Agent> ConsumerManager::factory(){
    auto state = std::make_shared<ConsumerState>();
    state->hungerDelay = clampTickSample(hungerDelayDist(gen));
    state->maxPrice = clampUnsignedShortSample(maxPriceDist(gen));
    state->asset = asset;
    states.push_back(state);

    return std::make_unique<Consumer>(0, state);
};


void ConsumerManager::changeNumAgents(unsigned int numAgents){
    long diff = (long)numAgents - (long)states.size();
    std::vector<long> doomedIds{};

    // Create new agents
    while(diff > 0){ 
        create();
        --diff;
    }

    // Destroy Agents
    while(diff < 0){ 
        long doomedId = traderIdsUnderMgmt.back();
        doomedIds.push_back(doomedId);

        traderIdsUnderMgmt.pop_back();
        states.pop_back();
        ++diff;
    }

    if (doomedIds.size() > 0)
        abm->removeAgents(doomedIds);       
}

// Producer Manager
ProducerManager::ProducerManager(
    std::shared_ptr<ABM> abm_,
    std::string name_,
    std::string asset_): AgentManager(abm_, name_){
        asset = asset_;
        preferedPriceDist = std::normal_distribution<double>(0, 0);
    };

void ProducerManager::changePreferedPrice(unsigned short mean, unsigned short std){
    preferedPriceDist = std::normal_distribution<double>(mean, std);
    reampleAgentPreferedPrice();
};

void ProducerManager::reampleAgentPreferedPrice(){
    for(auto state : states){
        state->preferedPrice = clampUnsignedShortSample(preferedPriceDist(gen));
    }
};

std::unique_ptr<Agent> ProducerManager::factory(){
    auto state = std::make_shared<ProducerState>();
    state->preferedPrice = clampUnsignedShortSample(preferedPriceDist(gen));
    state->asset = asset;
    states.push_back(state);

    return std::make_unique<Producer>(0, state);
};

void ProducerManager::changeNumAgents(unsigned int numAgents){
    long diff = static_cast<long>(numAgents) - static_cast<long>(states.size());
    std::vector<long> doomedIds{};

    while(diff > 0){
        create();
        --diff;
    }

    while(diff < 0){
        long doomedId = traderIdsUnderMgmt.back();
        doomedIds.push_back(doomedId);

        traderIdsUnderMgmt.pop_back();
        states.pop_back();
        ++diff;
    }

    if (!doomedIds.empty()) {
        abm->removeAgents(doomedIds);
    }
}