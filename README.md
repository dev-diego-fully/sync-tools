# sync-tools

`sync-tools` is a C++ project dedicated to exploring and implementing fundamental concurrent programming mechanisms. The primary goal of this repository is to serve as a personal learning platform, demonstrating how common synchronization tools—often found in robust libraries or language runtimes—are built from the ground up using C++'s standard threading primitives.

## Objective

The core objective is to demystify concurrency by providing clear, self-contained implementations of synchronization tools. This approach helps in understanding the internal logic and trade-offs involved, rather than just using these tools as black boxes. This project is a personal study resource and is not intended to be a production-ready library.

## Current and Planned Features

The repository currently contains a thread-safe, generic **Channel** implementation. This is a foundational mechanism for producer-consumer communication.

Future plans for this project include:
* **Semaphore Implementation:** A classic counting semaphore built using mutexes and condition variables.
* **Channel Enhancements:** Adding features such as a proper channel closing mechanism to handle producer-consumer scenarios gracefully.
* **Additional Tools:** Exploration and implementation of other synchronization primitives as the project evolves.

## Building and Testing

Tests will be an integral part of this repository, serving both to validate the correct functionality of the implementations and to provide practical examples of their usage. Detailed instructions for building the project and running the tests will be added here as the test suite is developed.