# ITQ - Lock-Free Queues

This project provides high-performance lock-free queue implementations for inter-thread communication.

## WARNING

*THIS QUEUE ASSUMES THE CONSUMERS ARE FASTER THAN THE PRODUCER. OTHERWISE DATA LOSS WILL OCCUR AND ORDER OF ITEMS WILL BE BROKEN.*

*i suggest keeping the capacity to at least the maximum number of burst pushes that you expect at any given time*

*data loss should be detected separately*

## Running Tests

```bash
cmake -B build
cmake --build build
ctest --test-dir build --verbose
```

## Installing

```bash
cmake -B build -D BUILD_TESTS=OFF
sudo cmake --install build
```