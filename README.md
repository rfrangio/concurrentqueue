# concurrentqueue

A small C++17 concurrent queue implementation backed by a singly linked list,
with a demo program that exercises multiple producers and consumers.

## Files

- `concurrent_queue.h`: header-only `concurrent_queue<T>` implementation.
- `main.cpp`: demo program that starts producer and consumer threads.
- `Makefile`: build, run, and clean targets for the demo executable.

## Build

Build the demo with:

```sh
make
```

This produces the `cqueue` executable in the project root.

## Run

Run the demo with:

```sh
make run
```

The program launches multiple consumer threads, pushes work from three
producer threads, waits for the queue to drain, then shuts consumers down
cleanly.

## Clean

Remove the built executable with:

```sh
make clean
```

## Queue API

The queue exposes these operations:

- `push(T value)`: enqueue a new value.
- `wait_and_pop()`: block until an item is available and return it.
- `wait_and_pop(T& value)`: block until an item is available and move it into `value`.
- `pop()`: attempt a non-blocking pop and return a `std::shared_ptr<T>`.
- `pop(T& value)`: attempt a non-blocking pop into `value`.
- `empty()`: check whether the queue is currently empty.
- `get_size()`: return the current atomic size counter.

## Notes

- The implementation is template-based, so all definitions live in `concurrent_queue.h`.
- The demo uses a sentinel value to stop consumer threads after all work is drained.
