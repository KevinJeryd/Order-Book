#include "../include/order_book.hpp"
#include <algorithm>

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

      // Find insert position for current price
      auto it = std::lower_bound(
          asks.begin(), asks.end(), order.price,
          [](const PriceLevel &level, Price p) { return level.price < p; });

      if (it != asks.end() && it->price == order.price) {
        // Price level already exists, just push the order
        it->orders.push_back(updated_order);
      } else {
        // Price level doesn't exist, insert a new one at this position
        asks.insert(it, PriceLevel{order.price, {updated_order}});
      }

      order_index.emplace(order.ID, OrderLocation{order.side, order.price});
    }
  } else {
    remaining_quantity = match_against(remaining_quantity, order, asks);
    if (remaining_quantity > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_quantity;

      // Find insert position for current price
      auto it = std::lower_bound(
          bids.begin(), bids.end(), order.price,
          [](const PriceLevel &level, Price p) { return level.price > p; });

      if (it != bids.end() && it->price == order.price) {
        // Price level already exists, just push the order
        it->orders.push_back(updated_order);
      } else {
        // Price level doesn't exist, insert a new one at this position
        bids.insert(it, PriceLevel{order.price, {updated_order}});
      }

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
    auto level_it = std::lower_bound(
        asks.begin(), asks.end(), order_location.price,
        [](const PriceLevel &level, Price p) { return level.price < p; });

    auto &orders = level_it->orders;

    for (auto order_it = orders.begin(); order_it != orders.end(); ++order_it) {
      if (order_it->ID == order_id) {
        orders.erase(order_it);
        break;
      }
    }

    // Remove price point if all orders are consumed
    if (orders.empty()) {
      asks.erase(level_it);
    }
  } else {
    auto level_it = std::lower_bound(
        bids.begin(), bids.end(), order_location.price,
        [](const PriceLevel &level, Price p) { return level.price > p; });

    auto &orders = level_it->orders;

    for (auto order_it = orders.begin(); order_it != orders.end(); ++order_it) {
      if (order_it->ID == order_id) {
        orders.erase(order_it);
        break;
      }
    }

    // Remove price point if all orders are consumed
    if (orders.empty()) {
      bids.erase(level_it);
    }
  }

  order_index.erase(order_id);

  return true;
}

Price OrderBook::best_bid() const {
  if (bids.empty()) {
    return -1;
  }

  return bids.front().price;
}

Price OrderBook::best_ask() const {
  if (asks.empty()) {
    return -1;
  }

  return asks.front().price;
}

Price OrderBook::spread() const {
  if (asks.empty() || bids.empty()) {
    return -1;
  }

  return asks.front().price - bids.front().price;
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
