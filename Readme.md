# AverageIterationTimer

A c++ class for timing methods repeatedly, and calculates their average runtime. Also contains a class for single-use, non-repeated, timers. Recommend c++20.

## Example

### Code

```c++
#include <chrono>
#include <iostream>
#include <thread>

#include "AverageIterationTimer.h"

void sleep_ms(const int ms) {
  std::this_thread::sleep_for (std::chrono::milliseconds (ms));
}

int main() {

  AverageIterationTimer timer;

  std::cout << "Processing within a loop..." << std::endl;
  for (int i = 0; i < 1000; i++) {

    timer.SetTimepoint("Task 1.");  // automatically terminated at next SetTimepoint() call
    sleep_ms(7);

    if (i % 8 == 0) {
      timer.SetTimepoint("Doing this task 1/8th of the time.");
      sleep_ms(4);
    }
    timer.EndTimepoint();

    if (i == 500) {
      // Decided you don't want this iteration to count towards the averaged result? Scrap it!
      timer.ScrapIteration();
    }

    timer.IterationFinished();  // Make sure to call at the end of every iteration.
  }

  // Print the results!
  timer.PrintAverageDurations();

  return 0;
}
```

### Output

```
Processing within a loop...
========================= AverageIterationTimer Results =========================
Task 1.-----------------------------> 8.574 ms
Doing this task 1/8th of the time.--> 4.951 ms

Process finished with exit code 0
```