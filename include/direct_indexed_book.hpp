#ifndef INCLUDE_DIRECT_INDEXED_BOOK_HPP
#define INCLUDE_DIRECT_INDEXED_BOOK_HPP

#include "types.hpp"
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace ob {

class DirectIndexedBook {
public:
  DirectIndexedBook(std::function<void(const Trade &)> trade_callback);

  void add_order(const Order &order);
  bool cancel_order(OrderId order_id);
  Price best_bid() const;
  Price best_ask() const;
  Price spread() const;

private:
  bool prices_cross(const Price bid, const Price ask) const;
  void emit_trade(const Price price, const Quantity quantity,
                  const OrderId buyer, const OrderId seller);
  void resize();
  template <typename Container>
  Quantity match_against(Quantity remaining, const Order &order,
                         Container &opposing_side, int32_t &best_idx);

  struct OrderLocation {
    Side side;
    Price price;
  };

  struct PriceLevel {
    Price price;
    std::deque<Order> orders;
  };

  Price base_price = 0;
  int32_t best_ask_idx = -1;
  int32_t best_bid_idx = -1;

  uint32_t array_size = 500;

  std::function<void(const Trade &)> trade_callback;

  std::vector<PriceLevel> asks;
  std::vector<PriceLevel> bids;
  std::unordered_map<OrderId, OrderLocation> order_index;
};

template <typename Container>
Quantity
DirectIndexedBook::match_against(Quantity remaining, const Order &order,
                                 Container &opposing_side, int32_t &best_idx) {
  int step = (order.side == Side::bid) ? 1 : -1;
  while (remaining > 0 && best_idx != -1) {
    auto &best_level = opposing_side[best_idx];

    if (order.side == Side::bid) {
      // order is bid, resting is ask
      if (!prices_cross(order.price, best_level.price))
        break;
    } else {
      // resting is bid, order is ask
      if (!prices_cross(best_level.price, order.price))
        break;
    }

    while (remaining > 0 && !best_level.orders.empty()) {
      Order &current_order = best_level.orders.front();

      Quantity fill = std::min(remaining, current_order.quantity);

      if (order.side == Side::bid) {
        emit_trade(current_order.price, fill, order.ID, current_order.ID);
      } else {
        emit_trade(current_order.price, fill, current_order.ID, order.ID);
      }

      remaining -= fill;

      current_order.quantity -= fill;

      if (current_order.quantity == 0) {
        order_index.erase(current_order.ID);
        best_level.orders.pop_front();
      }
    }

    if (best_level.orders.empty()) {
      best_idx += step;
      while (best_idx >= 0 && best_idx < (int)array_size &&
             opposing_side[best_idx].orders.empty()) {
        best_idx += step;
      }
      if (best_idx < 0 || best_idx >= (int)array_size) {
        best_idx = -1;
      }
    }
  }
  return remaining;
}

} // namespace ob

#endif
