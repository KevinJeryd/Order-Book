#include "../include/direct_indexed_book.hpp"
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>

int main() {
  // 1. Set up random number generators
  std::mt19937 rng(42);
  std::normal_distribution<double> price_dist(10000.0, 20.0);
  std::uniform_int_distribution<ob::Quantity> qty_dist(1, 100);
  std::uniform_int_distribution<int> side_dist(0, 1);

  // 2. Generate N orders
  std::vector<ob::Order> orders{};
  uint32_t num_orders = 1000000;

  for (int i = 0; i < num_orders; ++i) {
    ob::Price price = static_cast<ob::Price>(price_dist(rng));
    ob::Quantity qty = qty_dist(rng);
    ob::OrderId id = i + 1; // simple incrementing ID
    ob::Side side = side_dist(rng) ? ob::Side::bid : ob::Side::ask;

    ob::Order order{price, qty, id, side};

    orders.push_back(order);
  }

  std::vector<double> times;
  int benchmark_iterations = 10;

  // Run benchmark n times to get average
  for (int run = 0; run < benchmark_iterations; ++run) {
    ob::DirectIndexedBook book{[](const ob::Trade &) {}};

    // Start timer
    auto t0 = std::chrono::high_resolution_clock::now();

    // Process orders
    for (auto &order : orders) {
      book.add_order(order);
    }

    // Stop timer
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    times.push_back(ms);
  }

  // 6. Print results
  double min = std::numeric_limits<double>::max();
  double max = 0;
  double avg = 0;

  for (auto &time : times) {
    if (time < min)
      min = time;
    if (time > max)
      max = time;

    avg += time;
  }

  avg /= benchmark_iterations;

  std::cout << "Min Time: " << min << ", Avg Time: " << avg
            << ", Max Time: " << max << std::endl;

  std::cout << std::fixed << std::setprecision(2);

  double peak_throughput = (num_orders / min) * 1000.0;
  std::cout << "Peak Throughput: " << (peak_throughput / 1000000)
            << "M orders/sec" << std::endl;

  double avg_throughput = (num_orders / avg) * 1000.0;
  std::cout << "Avg Throughput: " << (avg_throughput / 1000000)
            << "M orders/sec" << std::endl;

  double min_throughput = (num_orders / max) * 1000.0;
  std::cout << "Min Throughput: " << (min_throughput / 1000000)
            << "M orders/sec" << std::endl;

  return 0;
}
