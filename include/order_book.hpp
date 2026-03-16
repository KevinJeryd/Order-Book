#ifndef INCLUDE_ORDER_BOOK_HPP
#define INCLUDE_ORDER_BOOK_HPP

#include "types.hpp"
#include <deque>
#include <functional>
#include <map>
#include <unordered_map>

namespace ob {

class OrderBook {
public:
  OrderBook(std::function<void(const Trade &)> trade_callback);

  void add_order(const Order &order);
  bool cancel_order(const OrderId order_id);

private:
  bool prices_cross(const Price bid, const Price ask) const;
  void emit_trade(const Price price, const Quantity quantity,
                  const OrderId buyer, const OrderId seller);
  template <typename Map>
  Quantity match_against(Quantity remaining, const Order &order,
                         Map &opposing_map);

  struct OrderLocation {
    Side side;
    Price price;
  };

  std::unordered_map<OrderId, OrderLocation> order_index;

  std::map<Price, std::deque<Order>> asks;
  std::map<Price, std::deque<Order>, std::greater<Price>> bids;
  std::function<void(const Trade &)> trade_callback;
};

template <typename Map>
Quantity OrderBook::match_against(Quantity remaining, const Order &order,
                                  Map &opposing_map) {
  while (remaining > 0 && !opposing_map.empty()) {
    auto &[best_price, best_level] = *opposing_map.begin();

    if (order.side == Side::bid) {
      // order is bid, resting is ask
      if (!prices_cross(order.price, best_price))
        break;
    } else {
      // resting is bid, order is ask
      if (!prices_cross(best_price, order.price))
        break;
    }

    while (remaining > 0 && !best_level.empty()) {
      Order &current_order = best_level.front();
      Quantity fill = std::min(remaining, current_order.quantity);

      if (order.side == Side::bid) {
        emit_trade(current_order.price, fill, order.ID, current_order.ID);
      } else {
        emit_trade(current_order.price, fill, current_order.ID, order.ID);
      }

      remaining -= fill;

      current_order.quantity -= fill;

      // Remove resting order if fully consumed
      if (current_order.quantity == 0) {
        order_index.erase(current_order.ID);
        best_level.pop_front();
      }
    }

    if (best_level.empty()) {
      opposing_map.erase(opposing_map.begin());
    }
  }

  return remaining;
}

} // namespace ob

#endif
