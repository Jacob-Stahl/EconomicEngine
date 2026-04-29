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

    Order2 makeLimit(long ordId, Side side, int price, unsigned int qty) {
        return OrderBuilder()
            .limit(side, price, qty)
            .withAsset("TEST")
            .withTraderId(1)
            .withOrdId(ordId)
            .build();
    }

    Order2 makeMarket(long ordId, Side side, unsigned int qty) {
        return OrderBuilder()
            .market(side, qty)
            .withAsset("TEST")
            .withTraderId(1)
            .withOrdId(ordId)
            .build();
    }

    Order2 makeStop(long ordId, Side side, int stopPrice, unsigned int qty) {
        return OrderBuilder()
            .stop(side, stopPrice, qty)
            .withAsset("TEST")
            .withTraderId(1)
            .withOrdId(ordId)
            .build();
    }

    Order2 makeStopLimit(long ordId, Side side, int limitPrice, int stopPrice, unsigned int qty) {
        return OrderBuilder()
            .stopLimit(side, limitPrice, stopPrice, qty)
            .withAsset("TEST")
            .withTraderId(1)
            .withOrdId(ordId)
            .build();
    }
};

TEST_F(Matcher2Test, PlaceBuyAndSellLimits_NoMatch_StateIsCorrect) {
    // Arrange & Act

    matcher.placeOrder(makeLimit(1, BUY, 100, 1));
    matcher.placeOrder(makeLimit(2, SELL, 110, 1));
    matcher.placeOrder(makeLimit(3, BUY, 90, 1));
    matcher.placeOrder(makeLimit(4, SELL, 120, 1));


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
    matcher.placeOrder(makeLimit(1, BUY, 100, 1));
    matcher.placeOrder(makeLimit(2, SELL, 110, 1));
    matcher.placeOrder(makeLimit(3, BUY, 111, 1));
    matcher.placeOrder(makeLimit(4, SELL, 99, 1));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_TRUE(spread.bidsMissing);
    EXPECT_TRUE(spread.asksMissing);

    // Check matches

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
    matcher.placeOrder(makeLimit(1, BUY, 100, 1));
    matcher.placeOrder(makeLimit(2, SELL, 110, 1));
    matcher.placeOrder(makeMarket(3, BUY, 1));
    matcher.placeOrder(makeMarket(4, SELL, 1));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_TRUE(spread.bidsMissing);
    EXPECT_TRUE(spread.asksMissing);

    // Check matches
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
    matcher.placeOrder(makeLimit(1, BUY, 100, 1));
    matcher.placeOrder(makeLimit(2, SELL, 110, 1));

    // Cancel 2 then 1
    matcher.cancelOrder(2);
    matcher.cancelOrder(1);

    matcher.placeOrder(makeMarket(3, BUY, 1));
    matcher.placeOrder(makeMarket(4, SELL, 1));

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

TEST_F(Matcher2Test, PlaceBuyAndSellLimits_SpreadCrossed_LiquidityNotDrained_StateIsCorrect){
    // Arrange & Act
    matcher.placeOrder(makeLimit(1, BUY, 100, 2));
    matcher.placeOrder(makeLimit(2, SELL, 110, 2));
    matcher.placeOrder(makeMarket(3, BUY, 1));
    matcher.placeOrder(makeMarket(4, SELL, 1));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_FALSE(spread.bidsMissing);
    EXPECT_FALSE(spread.asksMissing);

    EXPECT_EQ(100, spread.highestBid);
    EXPECT_EQ(110, spread.lowestAsk);

    // Check matches
    EXPECT_EQ(3, matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[0].qty);
    EXPECT_EQ(110, matcher.notifier->matches[0].price);

    EXPECT_EQ(1, matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(4, matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(1, matcher.notifier->matches[1].qty);
    EXPECT_EQ(100, matcher.notifier->matches[1].price);
}

TEST_F(Matcher2Test, SpreadCrossed_PartialFill_BUY_LIMIT_PlacedOnBook_StateIsCorrect){
    // Arrange & Act
    matcher.placeOrder(makeLimit(1, SELL, 110, 1));
    matcher.placeOrder(makeLimit(2, SELL, 101, 2));
    matcher.placeOrder(makeLimit(3, SELL, 100, 2));

    matcher.placeOrder(makeLimit(4, BUY, 95, 2));
    matcher.placeOrder(makeLimit(5, BUY, 94, 2));
    matcher.placeOrder(makeLimit(6, BUY, 90, 1));

    matcher.placeOrder(makeLimit(7, BUY, 105, 5));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_FALSE(spread.bidsMissing);
    EXPECT_FALSE(spread.asksMissing);

    EXPECT_EQ(105, spread.highestBid);
    EXPECT_EQ(110, spread.lowestAsk);

    // Check matches
    EXPECT_EQ(7, matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(3, matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[0].qty);
    EXPECT_EQ(100, matcher.notifier->matches[0].price);

    EXPECT_EQ(7, matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[1].qty);
    EXPECT_EQ(101, matcher.notifier->matches[1].price);
}

TEST_F(Matcher2Test, SpreadCrossed_PartialFill_SELL_LIMIT_PlacedOnBook_StateIsCorrect){
    // Arrange & Act
    matcher.placeOrder(makeLimit(4, SELL, 105, 2));
    matcher.placeOrder(makeLimit(5, SELL, 106, 2));
    matcher.placeOrder(makeLimit(6, SELL, 110, 1));

    matcher.placeOrder(makeLimit(1, BUY, 90, 1));
    matcher.placeOrder(makeLimit(2, BUY, 99, 2));
    matcher.placeOrder(makeLimit(3, BUY, 100, 2));

    matcher.placeOrder(makeLimit(7, SELL, 95, 5));

    const Spread& spread = matcher.getSpread();

    // Expect 2 matches. No cancellations expected
    EXPECT_EQ(2, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());

    // check spread
    EXPECT_FALSE(spread.bidsMissing);
    EXPECT_FALSE(spread.asksMissing);

    EXPECT_EQ(90, spread.highestBid);
    EXPECT_EQ(95, spread.lowestAsk);

    // Check matches
    EXPECT_EQ(3, matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(7, matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[0].qty);
    EXPECT_EQ(100, matcher.notifier->matches[0].price);

    EXPECT_EQ(2, matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(7, matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(2, matcher.notifier->matches[1].qty);
    EXPECT_EQ(99, matcher.notifier->matches[1].price);
}

TEST_F(Matcher2Test, StopLimitsActivateOnPriceSignal){

    // Place BUY LIMITS and a SELL STOP with a trigger price in the middle
    matcher.placeOrder(makeLimit(1, BUY, 115, 1));
    matcher.placeOrder(makeLimit(2, BUY, 111, 1));
    matcher.placeOrder(makeLimit(3, BUY, 110, 1));
    matcher.placeOrder(makeLimit(4, BUY, 105, 1));
    matcher.placeOrder(makeStop(5, SELL, 110, 1));

    // No matches expected initially
    EXPECT_EQ(0, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());
    EXPECT_FALSE(matcher.getSpread().bidsMissing);
    EXPECT_TRUE(matcher.getSpread().asksMissing);
    EXPECT_EQ(115, matcher.getSpread().highestBid);

    // Take the BUY limit @115. 1 match expected, trigger price IS NOT reached
    matcher.placeOrder(makeMarket(6, SELL, 1));
    EXPECT_EQ(1, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());
    EXPECT_FALSE(matcher.getSpread().bidsMissing);
    EXPECT_TRUE(matcher.getSpread().asksMissing);
    EXPECT_EQ(111, matcher.getSpread().highestBid);

    // Take the BUY limit @111 AND @110. another match expected, trigger price IS reached
    matcher.placeOrder(makeMarket(7, SELL, 1));
    EXPECT_EQ(3, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());
    EXPECT_FALSE(matcher.getSpread().bidsMissing);
    EXPECT_TRUE(matcher.getSpread().asksMissing);
    EXPECT_EQ(105, matcher.getSpread().highestBid);

    // Take the BUY limit @105. Book should be empty afterwards
    matcher.placeOrder(makeMarket(8, SELL, 1));
    EXPECT_EQ(4, matcher.notifier->matches.size());
    EXPECT_EQ(0, matcher.notifier->cancellations.size());
    EXPECT_TRUE(matcher.getSpread().bidsMissing);
    EXPECT_TRUE(matcher.getSpread().asksMissing);

    // Match 0: first market sell takes the best bid @115
    EXPECT_EQ(1,   matcher.notifier->matches[0].buyer.ordId);
    EXPECT_EQ(6,   matcher.notifier->matches[0].seller.ordId);
    EXPECT_EQ(1,   matcher.notifier->matches[0].qty);
    EXPECT_EQ(115, matcher.notifier->matches[0].price);

    // Match 1: second market sell takes bid @111, activating the dormant stop at 110
    EXPECT_EQ(2,   matcher.notifier->matches[1].buyer.ordId);
    EXPECT_EQ(7,   matcher.notifier->matches[1].seller.ordId);
    EXPECT_EQ(1,   matcher.notifier->matches[1].qty);
    EXPECT_EQ(111, matcher.notifier->matches[1].price);

    // Match 2: the now-active stop market-sells into the bid @110
    EXPECT_EQ(3,   matcher.notifier->matches[2].buyer.ordId);
    EXPECT_EQ(5,   matcher.notifier->matches[2].seller.ordId);
    EXPECT_EQ(1,   matcher.notifier->matches[2].qty);
    EXPECT_EQ(110, matcher.notifier->matches[2].price);

    // Match 3: third market sell takes the remaining bid @105
    EXPECT_EQ(4,   matcher.notifier->matches[3].buyer.ordId);
    EXPECT_EQ(8,   matcher.notifier->matches[3].seller.ordId);
    EXPECT_EQ(1,   matcher.notifier->matches[3].qty);
    EXPECT_EQ(105, matcher.notifier->matches[3].price);
}