#include "../include/order_book.hpp"

namespace ob {

OrderBook::OrderBook(std::function<void(const Trade &)> trade_callback)
    : trade_callback(trade_callback) {}

void OrderBook::add_order(const Order &order) {
  Quantity remaining_quantity = order.quantity;

  if (order.side == Side::ask) {
    remaining_quantity = match_against(remaining_quantity, order, bids);
    if (remaining_quantity > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_quantity;
      asks[order.price].push_back(updated_order);
      order_index.emplace(order.ID, OrderLocation{order.side, order.price});
    }
  } else {
    remaining_quantity = match_against(remaining_quantity, order, asks);
    if (remaining_quantity > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_quantity;
      bids[order.price].push_back(updated_order);
      order_index.emplace(order.ID, OrderLocation{order.side, order.price});
    }
  }
}

bool OrderBook::cancel_order(const OrderId order_id) {
  auto it = order_index.find(order_id);

  if (it == order_index.end()) {
    return false; // Order doesn't exist
  }

  OrderLocation order_location = it->second;

  if (order_location.side == Side::ask) {
    std::deque<Order> &orders = asks[order_location.price];

    for (auto it = orders.begin(); it != orders.end(); ++it) {
      if (it->ID == order_id) {
        orders.erase(it);
        break;
      }
    }

    // Remove price point if all orders are consumed
    if (orders.empty()) {
      asks.erase(order_location.price);
    }
  } else {
    std::deque<Order> &orders = bids[order_location.price];

    for (auto it = orders.begin(); it != orders.end(); ++it) {
      if (it->ID == order_id) {
        orders.erase(it);
        break;
      }
    }

    // Remove price point if all orders are consumed
    if (orders.empty()) {
      bids.erase(order_location.price);
    }
  }

  order_index.erase(order_id);

  return true;
}

bool OrderBook::prices_cross(const Price bid, const Price ask) const {
  return bid >= ask;
}

void OrderBook::emit_trade(const Price price, const Quantity quantity,
                           const OrderId buyer, const OrderId seller) {
  Trade trade{price, quantity, buyer, seller};
  trade_callback(trade);
}
} // namespace ob
