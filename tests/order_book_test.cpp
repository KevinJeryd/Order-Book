#include "../include/order_book.hpp"
#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test {
protected:
  std::vector<ob::Trade> trades;
  ob::OrderBook book{
      [this](const ob::Trade &trade) { trades.push_back(trade); }};
};

TEST_F(OrderBookTest, BasicMatch) {
  ob::Order ask_order{102, 200, 1, ob::Side::ask};
  ob::Order bid_order{105, 150, 2, ob::Side::bid};

  book.add_order(ask_order);
  book.add_order(bid_order);

  EXPECT_EQ(trades.size(), 1);
  EXPECT_EQ(trades[0].price, 102);
  EXPECT_EQ(trades[0].quantity, 150);
  EXPECT_EQ(trades[0].buyer, 2);
  EXPECT_EQ(trades[0].seller, 1);
}

TEST_F(OrderBookTest, PartialFillTest) {
  ob::Order ask_order{85, 50, 3, ob::Side::ask};
  ob::Order buy_order{90, 100, 4, ob::Side::bid};

  book.add_order(ask_order);
  book.add_order(buy_order);

  EXPECT_EQ(trades.size(), 1);
  EXPECT_EQ(trades[0].price, 85);
  EXPECT_EQ(trades[0].quantity, 50);
  EXPECT_EQ(trades[0].buyer, 4);
  EXPECT_EQ(trades[0].seller, 3);
}

TEST_F(OrderBookTest, TwoPriceLevelsTest) {
  ob::Order ask_order_one{60, 10, 5, ob::Side::ask};
  ob::Order ask_order_two{55, 20, 6, ob::Side::ask};
  ob::Order buy_order{60, 30, 7, ob::Side::bid};

  book.add_order(ask_order_one);
  book.add_order(ask_order_two);
  book.add_order(buy_order);

  EXPECT_EQ(trades.size(), 2);

  EXPECT_EQ(trades[0].price, 55);
  EXPECT_EQ(trades[0].quantity, 20);
  EXPECT_EQ(trades[0].buyer, 7);
  EXPECT_EQ(trades[0].seller, 6);

  EXPECT_EQ(trades[1].price, 60);
  EXPECT_EQ(trades[1].quantity, 10);
  EXPECT_EQ(trades[1].buyer, 7);
  EXPECT_EQ(trades[1].seller, 5);
}

TEST_F(OrderBookTest, CancellationTest) {
  ob::Order ask_order{60, 10, 8, ob::Side::ask};
  book.add_order(ask_order);

  bool result = book.cancel_order(8);
  EXPECT_EQ(result, true);

  ob::Order bid_order{100, 10, 9, ob::Side::bid};
  book.add_order(bid_order);

  bool result2 = book.cancel_order(999);
  EXPECT_EQ(result2, false);

  EXPECT_TRUE(trades.empty());
}

TEST_F(OrderBookTest, TestBestBid) {
  ob::Order buy_order1{60, 10, 7, ob::Side::bid};
  ob::Order buy_order2{40, 30, 8, ob::Side::bid};
  ob::Order buy_order3{80, 15, 9, ob::Side::bid};
  ob::Order buy_order4{90, 30, 10, ob::Side::bid};

  book.add_order(buy_order1);
  book.add_order(buy_order2);
  book.add_order(buy_order3);
  book.add_order(buy_order4);

  ob::Price best_bid = book.best_bid();

  EXPECT_EQ(best_bid, 90);
}

TEST_F(OrderBookTest, TestNoBidsBestBid) {
  ob::Price best_bid = book.best_bid();
  EXPECT_EQ(best_bid, -1);
}

TEST_F(OrderBookTest, TestBestAsk) {
  ob::Order ask_order1{60, 10, 7, ob::Side::ask};
  ob::Order ask_order2{40, 30, 8, ob::Side::ask};
  ob::Order ask_order3{80, 15, 9, ob::Side::ask};
  ob::Order ask_order4{90, 30, 10, ob::Side::ask};

  book.add_order(ask_order1);
  book.add_order(ask_order2);
  book.add_order(ask_order3);
  book.add_order(ask_order4);

  ob::Price best_ask = book.best_ask();

  EXPECT_EQ(best_ask, 40);
}

TEST_F(OrderBookTest, TestNoAsksBestAsk) {
  ob::Price best_ask = book.best_ask();
  EXPECT_EQ(best_ask, -1);
}

TEST_F(OrderBookTest, TestSpread) {
  ob::Order ask_order1{60, 5, 7, ob::Side::ask};

  ob::Order buy_order1{30, 10, 1, ob::Side::bid};
  ob::Order buy_order2{40, 10, 2, ob::Side::bid};

  book.add_order(ask_order1);
  book.add_order(buy_order1);
  book.add_order(buy_order2);

  ob::Price spread = book.spread();

  EXPECT_EQ(spread, 20);
}

TEST_F(OrderBookTest, TestNoAsksOrBidsSpread) {
  ob::Price spread = book.spread();
  EXPECT_EQ(spread, -1);
}

TEST_F(OrderBookTest, TestNoAsksButBidsSpread) {
  ob::Order buy_order1{30, 10, 1, ob::Side::bid};
  ob::Order buy_order2{40, 10, 2, ob::Side::bid};

  book.add_order(buy_order1);
  book.add_order(buy_order2);

  ob::Price spread = book.spread();
  EXPECT_EQ(spread, -1);
}

TEST_F(OrderBookTest, TestNoBidsButAsksSpread) {
  ob::Order ask_order1{30, 10, 1, ob::Side::ask};
  ob::Order ask_order2{40, 10, 2, ob::Side::ask};

  book.add_order(ask_order1);
  book.add_order(ask_order2);

  ob::Price spread = book.spread();
  EXPECT_EQ(spread, -1);
}
