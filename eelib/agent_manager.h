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
        virtual std::unique_ptr<Agent> factory() = 0;

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

        void changeNumAgents(unsigned int numAgents);
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

        void changeNumAgents(unsigned int numAgents);
};

class ManufacturerManager : public AgentManager{
    std::mt19937 gen;
    std::vector<std::shared_ptr<ManufacturerState>> states;
    Recipe recipe;

    public:
        bool numAgentsFixed = true;

        /// @brief Increase/Decrease the number of agents by this proportion
        double numAgentsScaleFactor = 0.05;
        tick neutralAge = tick(5);
        tick staleAge = tick(10);

        ManufacturerManager(
            std::shared_ptr<ABM> abm_,
            std::string name_,
            Recipe recipe);

        std::unique_ptr<Agent> factory() override;
        
        void changeNumAgents(unsigned int numAgents);
        

        // TODO How do we incorperate this into the game loop? Ideally the ABM doesn't know about these managers...

        /// @brief Find the new number of agents. 
        /// @return 
        unsigned int newNumAgents();

        // agent_diff = (recent_sale_count - stale_count) * scale_factor
        // new_num_agents = num_agents + agent_diff
        
};
