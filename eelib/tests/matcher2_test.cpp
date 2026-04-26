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

    Order makeOrder(long ordId, Side side, OrdType type, unsigned short price, unsigned int qty) {
        Order o;
        o.ordId = ordId;
        o.traderId = 1;
        o.side = side;
        o.price = price;
        o.qty = qty;
        o.type = type;
        o.asset = "TEST";
        return o;
    }
};

TEST_F(Matcher2Test, PlaceBuyAndSellLimits_NoMatch_StateIsCorrect) {
    // Arrange & Act

    // BUY LIMIT 100 1
    matcher.placeOrder(makeOrder(1, BUY, LIMIT, 100, 1));
    // SELL LIMIT 110 1
    matcher.placeOrder(makeOrder(2, SELL, LIMIT, 110, 1));
    // BUY LIMIT 90 1
    matcher.placeOrder(makeOrder(3, BUY, LIMIT, 90, 1));
    // SELL LIMIT 120 1
    matcher.placeOrder(makeOrder(4, SELL, LIMIT, 120, 1));


    const Spread& spread = matcher.getSpread();

    // Assert

    // No matches or cancellations expected
    EXPECT_EQ(0, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // 4 orders have been registered
    EXPECT_EQ(4, matcher.notifier->orderRegistery.size());

    // check spread
    EXPECT_FALSE(spread.bidsMissing);
    EXPECT_FALSE(spread.asksMissing);
    EXPECT_EQ(spread.highestBid, 100);
    EXPECT_EQ(spread.lowestAsk, 110);
}

TEST_F(Matcher2Test, PlaceBuyAndSellLimits_SpreadCrossed_StateIsCorrect){
        // Arrange & Act

    // BUY LIMIT 100 1
    matcher.placeOrder(makeOrder(1, BUY, LIMIT, 100, 1));
    // SELL LIMIT 110 1
    matcher.placeOrder(makeOrder(2, SELL, LIMIT, 110, 1));
    // BUY LIMIT 111 1
    matcher.placeOrder(makeOrder(3, BUY, LIMIT, 111, 1));
    // SELL LIMIT 99 1
    matcher.placeOrder(makeOrder(4, SELL, LIMIT, 99, 1));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_TRUE(spread.bidsMissing);
    EXPECT_TRUE(spread.asksMissing);

    // Check matchs

    EXPECT_EQ(3, matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[0].qty);
    EXPECT_EQ(110, matcher.notifier->matches[0].price);

    EXPECT_EQ(1, matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(4, matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[1].qty);
    EXPECT_EQ(100, matcher.notifier->matches[1].price);
}

TEST_F(Matcher2Test, PlaceBuyAndSellLimitsAndMarkets_SpreadCrossed_StateIsCorrect){
        // Arrange & Act

    // BUY LIMIT 100 1
    matcher.placeOrder(makeOrder(1, BUY, LIMIT, 100, 1));
    // SELL LIMIT 110 1
    matcher.placeOrder(makeOrder(2, SELL, LIMIT, 110, 1));
    // BUY LIMIT 111 1
    matcher.placeOrder(makeOrder(3, BUY, MARKET, 0, 1));
    // SELL LIMIT 99 1
    matcher.placeOrder(makeOrder(4, SELL, MARKET, 0, 1));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_TRUE(spread.bidsMissing);
    EXPECT_TRUE(spread.asksMissing);

    // Check matchs
    EXPECT_EQ(3, matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[0].qty);
    EXPECT_EQ(110, matcher.notifier->matches[0].price);

    EXPECT_EQ(1, matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(4, matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[1].qty);
    EXPECT_EQ(100, matcher.notifier->matches[1].price);
}

TEST_F(Matcher2Test, CancelOrder_NotMatched_StateIsCorrect){
    // BUY LIMIT 100 1
    matcher.placeOrder(makeOrder(1, BUY, LIMIT, 100, 1));
    // SELL LIMIT 110 1
    matcher.placeOrder(makeOrder(2, SELL, LIMIT, 110, 1));

    // Cancel 2 then 1
    matcher.cancelOrder(2);
    matcher.cancelOrder(1);

    // BUY LIMIT 111 1
    matcher.placeOrder(makeOrder(3, BUY, MARKET, 0, 1));
    // SELL LIMIT 99 1
    matcher.placeOrder(makeOrder(4, SELL, MARKET, 0, 1));

    const Spread& spread = matcher.getSpread();

    // Expect no matches or cancellations expected
    EXPECT_EQ(0, matcher.notifier->matches.size());
    EXPECT_EQ(4, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_TRUE(spread.bidsMissing);
    EXPECT_TRUE(spread.asksMissing);

    // Check cancellations. 
    // First 2 are cancelled manually, in correct order
    EXPECT_EQ(2, matcher.notifier->cancellations[0].ordId);
    EXPECT_EQ(1, matcher.notifier->cancellations[1].ordId);

    // Next 2 market orders are cancelled because there is no liquidity
    EXPECT_EQ(3, matcher.notifier->cancellations[2].ordId);
    EXPECT_EQ(4, matcher.notifier->cancellations[3].ordId);
    
}