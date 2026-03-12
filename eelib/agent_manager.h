#pragma once

#include "agent.h"
#include "abm.h"
#include <vector>
#include <memory>

class AgentManager{
    std::vector<long> traderIdsUnderMgmt;
    std::shared_ptr<ABM> abm; // make sealed?

    public:
        std::string name;

        AgentManager(std::shared_ptr<ABM> abm_){
            abm = abm_;
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


// TODO consumer and producer managers