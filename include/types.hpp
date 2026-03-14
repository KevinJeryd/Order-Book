#ifndef INCLUDE_TYPES_HPP
#define INCLUDE_TYPES_HPP

#include <cstdint>

namespace ob {
using Price = int32_t;
using Quantity = uint32_t;
using OrderId = uint64_t;

enum class Side { bid, ask };

struct Order {
  Price price;
  Quantity quantity;
  OrderId ID;
  Side side;
};

struct Trade {
  Price price;
  Quantity quantity;
  OrderId buyer;
  OrderId seller;
};
} // namespace ob

#endif // !INCLUDE_TYPES_HPP
