#pragma once

#include "agent.h"
#include "abm.h"
#include <random>
#include <vector>
#include <memory>

class AgentManager{
    protected:
        std::vector<long> traderIdsUnderMgmt;
        std::shared_ptr<ABM> abm;

    public:
        std::string name;

        AgentManager(std::shared_ptr<ABM> abm_, std::string name_){
            abm = abm_;
            name = name_;
        }

        /// @brief produces the desired agent
        /// @return 
        virtual std::unique_ptr<Agent> factory();

        void create(){
            long traderId = abm->addAgent(std::move(factory()));
            traderIdsUnderMgmt.push_back(traderId);
        };

        virtual ~AgentManager(){
            // Clean up agents before destruction
            abm->removeAgents(traderIdsUnderMgmt);
        };
};

class ConsumerManager : public AgentManager{
    std::mt19937 gen;
    std::vector<std::shared_ptr<ConsumerState>> states;
    std::string asset;

    // Underflow issue?
    std::normal_distribution<double> hungerDelayDist;

    // Would log distribution make more sense? 
    // Mark Zuckerberg has a higher max price than the normal person
    std::normal_distribution<double> maxPriceDist;

    void resampleAgentHungerDelay();
    void reampleAgentMaxPrice();

    public:
        ConsumerManager(
            std::shared_ptr<ABM> abm_,
            std::string name_,
            std::string asset_);

        void changeHungerDelay(unsigned long mean, unsigned long std);
        void changeMaxPrice(unsigned short mean, unsigned short std);

        std::unique_ptr<Agent> factory() override;
};

class ProducerManager : public AgentManager{
    std::mt19937 gen;
    std::vector<std::shared_ptr<ProducerState>> states;
    std::string asset;

    std::normal_distribution<double> preferedPriceDist;

    public:
        void reampleAgentPreferedPrice();

        ProducerManager(
            std::shared_ptr<ABM> abm_,
            std::string name_,
            std::string asset_);

        void changePreferedPrice(unsigned short mean, unsigned short std);

        std::unique_ptr<Agent> factory() override;
};

