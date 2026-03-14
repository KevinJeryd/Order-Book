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
      asks[order.price].push(updated_order);
    }
  } else {
    remaining_quantity = match_against(remaining_quantity, order, asks);
    if (remaining_quantity > 0) {
      Order updated_order = order;
      updated_order.quantity = remaining_quantity;
      bids[order.price].push(updated_order);
    }
  }
}

void OrderBook::cancel_order(const OrderId order_id) {}

bool OrderBook::prices_cross(const Price bid, const Price ask) const {
  return bid >= ask;
}

void OrderBook::emit_trade(const Price price, const Quantity quantity,
                           const OrderId buyer, const OrderId seller) {
  Trade trade{price, quantity, buyer, seller};
  trade_callback(trade);
}
} // namespace ob
