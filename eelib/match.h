#pragma once

#include <vector>
#include "order.h"
#include <stdexcept>

// TODO time of match might be important for message ordering down stream
struct Match{
    Order buyer;
    Order seller;
    long qty;

    public:
        Match(const Order& ord1, const Order& ord2, long qty_)
        {
            if (ord1.side == ord2.side) { std::logic_error("Can't match orders on the same side!"); }
            if (ord1.side == BUY){
                buyer = ord1;
                seller = ord2;
            }
            else{
                buyer = ord2;
                seller = ord1;
            }

            qty = qty_;
        }
};
