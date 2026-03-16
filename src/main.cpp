#include "../include/order_book.hpp"
#include <iostream>

void print_trades(const ob::Trade &trade) {
  std::cout << "---------- TRADE --------" << std::endl;
  std::cout << "Price: " << trade.price << std::endl;
  std::cout << "Quantity: " << trade.quantity << std::endl;
  std::cout << "BuyerID: " << trade.buyer << std::endl;
  std::cout << "SellerID: " << trade.seller << std::endl;
}

int main() {
  ob::OrderBook order_book(print_trades);

  // Basic test
  // ob::Order ask_order{102, 200, 1, ob::Side::ask};
  // ob::Order buy_order{105, 150, 2, ob::Side::bid};
  //
  // order_book.add_order(ask_order);
  // order_book.add_order(buy_order);

  // Partial fill test
  // ob::Order ask_order{85, 50, 3, ob::Side::ask};
  // ob::Order buy_order{90, 100, 4, ob::Side::bid};
  //
  // order_book.add_order(ask_order);
  // order_book.add_order(buy_order);

  // Two price levels test
  // ob::Order ask_order_one{60, 10, 5, ob::Side::ask};
  // ob::Order ask_order_two{55, 20, 6, ob::Side::ask};
  // ob::Order buy_order{60, 30, 7, ob::Side::bid};
  //
  // order_book.add_order(ask_order_one);
  // order_book.add_order(ask_order_two);
  // order_book.add_order(buy_order);

  // Cancel order test
  ob::Order ask_order{60, 10, 8, ob::Side::ask};
  order_book.add_order(ask_order);

  bool result = order_book.cancel_order(8);
  std::cout << "Cancel existing order: " << (result ? "true" : "false")
            << std::endl;

  ob::Order bid_order{100, 10, 9, ob::Side::bid};
  order_book.add_order(bid_order);

  bool result2 = order_book.cancel_order(999);
  std::cout << "Cancel non-existing order: " << (result2 ? "true" : "false")
            << std::endl;

  return 0;
}
