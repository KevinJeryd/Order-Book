#ifndef INCLUDE_ORDER_BOOK_HPP
#define INCLUDE_ORDER_BOOK_HPP

#include "types.hpp"
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace ob {

class OrderBook {
public:
  OrderBook(std::function<void(const Trade &)> trade_callback);

  void add_order(const Order &order);
  bool cancel_order(const OrderId order_id);

  Price best_bid() const;
  Price best_ask() const;
  Price spread() const;

private:
  bool prices_cross(const Price bid, const Price ask) const;
  void emit_trade(const Price price, const Quantity quantity,
                  const OrderId buyer, const OrderId seller);
  template <typename Container>
  Quantity match_against(Quantity remaining, const Order &order,
                         Container &opposing_side);

  struct OrderLocation {
    Side side;
    Price price;
  };

  struct PriceLevel {
    Price price;
    std::deque<Order> orders;
  };

  std::unordered_map<OrderId, OrderLocation> order_index;

  std::vector<PriceLevel> asks;
  std::vector<PriceLevel> bids;
  std::function<void(const Trade &)> trade_callback;
};

template <typename Container>
Quantity OrderBook::match_against(Quantity remaining, const Order &order,
                                  Container &opposing_side) {
  while (remaining > 0 && !opposing_side.empty()) {
    auto &best_level_entry = opposing_side.front();
    Price &best_price = best_level_entry.price;
    auto &best_level = best_level_entry.orders;

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
      opposing_side.erase(opposing_side.begin());
    }
  }

  return remaining;
}

} // namespace ob

#endif
