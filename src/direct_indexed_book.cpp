#include "../include/direct_indexed_book.hpp"

namespace ob {

DirectIndexedBook::DirectIndexedBook(
    std::function<void(const Trade &)> trade_callback)
    : trade_callback(trade_callback), asks(array_size), bids(array_size) {}

void DirectIndexedBook::add_order(const Order &order) {
  if (base_price == 0) {
    base_price = order.price - (array_size / 2);
  };

  int insert_idx = order.price - base_price;

  if (insert_idx >= (int)array_size || insert_idx < 0) {
    resize();
    insert_idx = order.price - base_price; // recalculate after resize
  }

  Quantity remaining_qty = order.quantity;

  if (order.side == Side::ask) {
    remaining_qty = match_against(remaining_qty, order, bids, best_bid_idx);
    if (remaining_qty > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_qty;

      asks[insert_idx].price = order.price;
      asks[insert_idx].orders.push_back(updated_order);

      if (best_ask_idx == -1 ||
          asks[insert_idx].price < asks[best_ask_idx].price) {
        best_ask_idx = insert_idx;
      }

      order_index.emplace(order.ID, OrderLocation{order.side, order.price});
    }
  } else {
    remaining_qty = match_against(remaining_qty, order, asks, best_ask_idx);
    if (remaining_qty > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_qty;

      bids[insert_idx].price = order.price;
      bids[insert_idx].orders.push_back(updated_order);

      if (best_bid_idx == -1 ||
          bids[insert_idx].price > bids[best_bid_idx].price) {
        best_bid_idx = insert_idx;
      }

      order_index.emplace(order.ID, OrderLocation{order.side, order.price});
    }
  }
}

void DirectIndexedBook::resize() {
  uint32_t new_size = array_size * 2;

  uint32_t shift = (new_size - array_size) / 2;

  base_price = base_price - shift;

  std::vector<PriceLevel> new_asks(new_size);
  std::vector<PriceLevel> new_bids(new_size);

  for (int i = 0; i < asks.size(); ++i) {
    new_asks[i + shift] = std::move(asks[i]);
    new_bids[i + shift] = std::move(bids[i]);
  }

  if (best_bid_idx != -1)
    best_bid_idx += shift;
  if (best_ask_idx != -1)
    best_ask_idx += shift;

  asks = std::move(new_asks);
  bids = std::move(new_bids);

  array_size = new_size;
}

bool DirectIndexedBook::cancel_order(OrderId order_id) {
  auto it = order_index.find(order_id);

  if (it == order_index.end()) {
    return false; // Order doesn't exist
  }

  OrderLocation order_location = it->second;

  int order_idx = order_location.price - base_price;

  if (order_location.side == Side::ask) {
    std::deque<Order> &orders = asks[order_idx].orders;

    for (auto it = orders.begin(); it != orders.end(); ++it) {
      if (it->ID == order_id) {
        orders.erase(it);
        break;
      }
    }

    if (orders.empty() && order_idx == best_ask_idx) {
      best_ask_idx += 1;
      while (best_ask_idx >= 0 && best_ask_idx < (int)array_size &&
             asks[best_ask_idx].orders.empty()) {
        best_ask_idx += 1;
      }
      if (best_ask_idx < 0 || best_ask_idx >= (int)array_size) {
        best_ask_idx = -1;
      }
    }
  } else {
    std::deque<Order> &orders = bids[order_idx].orders;

    for (auto it = orders.begin(); it != orders.end(); ++it) {
      if (it->ID == order_id) {
        orders.erase(it);
        break;
      }
    }

    if (orders.empty() && order_idx == best_bid_idx) {
      best_bid_idx -= 1;
      while (best_bid_idx >= 0 && best_bid_idx < (int)array_size &&
             bids[best_bid_idx].orders.empty()) {
        best_bid_idx -= 1;
      }
      if (best_bid_idx < 0 || best_bid_idx >= (int)array_size) {
        best_bid_idx = -1;
      }
    }
  }

  order_index.erase(order_id);

  return true;
}

Price DirectIndexedBook::best_bid() const {
  if (best_bid_idx == -1)
    return -1;
  return bids[best_bid_idx].price;
}
Price DirectIndexedBook::best_ask() const {
  if (best_ask_idx == -1)
    return -1;
  return asks[best_ask_idx].price;
}
Price DirectIndexedBook::spread() const {
  if (best_ask_idx == -1 || best_bid_idx == -1)
    return -1;
  return asks[best_ask_idx].price - bids[best_bid_idx].price;
}

bool DirectIndexedBook::prices_cross(const Price bid, const Price ask) const {
  return bid >= ask;
}
void DirectIndexedBook::emit_trade(const Price price, const Quantity quantity,
                                   const OrderId buyer, const OrderId seller) {
  Trade trade{price, quantity, buyer, seller};
  trade_callback(trade);
}
} // namespace ob
