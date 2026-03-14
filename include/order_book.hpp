#ifndef INCLUDE_ORDER_BOOK_HPP
#define INCLUDE_ORDER_BOOK_HPP

#include "types.hpp"
#include <functional>
#include <map>
#include <queue>

namespace ob {

class OrderBook {
public:
  OrderBook(std::function<void(const Trade &)> trade_callback);

  void add_order(const Order &order);
  void cancel_order(const OrderId order_id);

private:
  bool prices_cross(const Price ask, const Price bid) const;
  void emit_trade(const Price price, const Quantity quantity,
                  const OrderId buyer, const OrderId seller);

  std::map<Price, std::queue<Order>> asks;
  std::map<Price, std::queue<Order>, std::greater<Price>> bids;
  std::function<void(const Trade &)> trade_callback;
};

} // namespace ob

#endif
