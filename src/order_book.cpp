#include "order_book.hpp"

namespace ob {

OrderBook::OrderBook(std::function<void(const Trade &)> trade_callback)
    : trade_callback(trade_callback) {}

void OrderBook::add_order(const Order &order) {}

void OrderBook::cancel_order(const OrderId order_id) {}

bool OrderBook::prices_cross(const Price ask, const Price bid) const {
  return false;
}

void OrderBook::emit_trade(const Price price, const Quantity quantity,
                           const OrderId buyer, const OrderId seller) {}
} // namespace ob
