#pragma once

#include "agent.h"
#include "abm.h"
#include <vector>
#include <memory>

class Manager{
    std::vector<long> traderIds;
    std::shared_ptr<ABM> abm;

    public:
        Manager(std::shared_ptr<ABM> abm_){
            abm = abm_;
        }

        virtual void create(){};

        virtual ~Manager(){
            // Clean up agents before destruction
            abm->removeAgents(traderIds);
        };

};