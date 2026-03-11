#pragma once

#include "agent.h"
#include "abm.h"
#include <vector>
#include <memory>





class Manager{
    
    std::shared_ptr<ABM> abm;



    public:
        virtual void create(){};
        
        ~Manager(){

        };

};