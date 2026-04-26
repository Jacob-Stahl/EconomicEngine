#include <gtest/gtest.h>
#include "../matcher2.h"
#include "../notifier2.h"
#include "../order.h"

class Matcher2Test : public ::testing::Test {
protected:
    Matcher2 matcher;

    void SetUp() override {
        matcher.notifier = std::make_unique<Notifier2>();
    }

    Order makeOrder(long ordId, Side side, unsigned short price, unsigned int qty) {
        Order o;
        o.ordId = ordId;
        o.traderId = 1;
        o.side = side;
        o.price = price;
        o.qty = qty;
        o.type = LIMIT;
        o.asset = "TEST";
        return o;
    }
};

TEST_F(Matcher2Test, PlaceBuyAndSellNoMatch_SpreadIsCorrect) {
    // BUY LIMIT 100 1
    matcher.placeOrder(makeOrder(1, BUY, 100, 1));
    // SELL LIMIT 110 1
    matcher.placeOrder(makeOrder(2, SELL, 110, 1));

    const Spread& spread = matcher.getSpread();

    EXPECT_FALSE(spread.bidsMissing);
    EXPECT_FALSE(spread.asksMissing);
    EXPECT_EQ(spread.highestBid, 100);
    EXPECT_EQ(spread.lowestAsk, 110);
}
