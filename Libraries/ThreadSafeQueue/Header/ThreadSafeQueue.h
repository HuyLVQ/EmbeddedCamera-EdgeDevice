#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * @class ThreadSafeQueue
 * @brief A thread-safe queue supporting blocking and non-blocking pop operations.
 *
 * Uses a mutex and condition variable to synchronize concurrent producers
 * and consumers. Supports graceful shutdown to unblock waiting threads.
 *
 * Typical usage in this project:
 *   - Producer (CLI thread) calls push() to enqueue outbound messages.
 *   - Consumer (publisher thread) calls pop() which blocks until a message
 *     is available, then sends it to the Broker.
 *   - On shutdown, shutdown() is called so pop() stops blocking and returns
 *     nullopt, letting the consumer thread exit cleanly.
 *
 * @tparam T The type of elements stored in the queue.
 */
template<typename T>
class ThreadSafeQueue {
public:
    /**
     * @brief Pushes an item onto the queue and wakes up one waiting consumer.
     *
     * If the queue has already been shut down, the item is silently discarded
     * to avoid pushing work that will never be consumed.
     *
     * @param item The item to push.
     */
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (shutdownFlag) return; // discard — nobody is consuming anymore
            items.push(std::move(item));
        }
        // Wake one waiting pop() call so it can pick up the new item.
        cond.notify_one();
    }

    /**
     * @brief Blocking pop — waits until an item is available or the queue is shut down.
     *
     * The calling thread sleeps here until either:
     *   - An item is pushed (push() calls notify_one), or
     *   - The queue is shut down (shutdown() calls notify_all).
     *
     * @return The front item wrapped in std::optional, or std::nullopt if shut down.
     */
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        // Sleep until there is work to do OR the queue is shutting down.
        cond.wait(lock, [this]{ return !items.empty() || shutdownFlag; });
        if (items.empty()) return std::nullopt; // woken by shutdown, no item
        T item = std::move(items.front());
        items.pop();
        return item;
    }

    /**
     * @brief Non-blocking pop — returns immediately if the queue is empty.
     *
     * Used by the Broker's Subscribe loop so it can check IsCancelled()
     * between attempts rather than blocking indefinitely.
     *
     * @return The front item wrapped in std::optional, or std::nullopt if empty.
     */
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex);
        if (items.empty()) return std::nullopt;
        T item = std::move(items.front());
        items.pop();
        return item;
    }

    /**
     * @brief Signals the queue to shut down.
     *
     * Sets the shutdown flag and wakes ALL waiting threads (via notify_all)
     * so every blocked pop() returns std::nullopt and the threads can exit.
     */
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            shutdownFlag = true;
        }
        // Wake every thread waiting on pop() so they can all see shutdownFlag = true.
        cond.notify_all();
    }

    /**
     * @brief Checks whether the queue has been shut down.
     *
     * @return true if shutdown() has been called, false otherwise.
     */
    bool isShutdown() const {
        std::lock_guard<std::mutex> lock(mutex);
        return shutdownFlag;
    }

private:
    // mutex protects all shared state below. Every access to items or
    // shutdownFlag must be done while holding this lock.
    mutable std::mutex mutex;

    // cond is used to put consumers to sleep while the queue is empty and
    // wake them up when push() adds an item or shutdown() is called.
    std::condition_variable cond;

    // The underlying queue. std::queue gives us O(1) push/pop from both ends.
    std::queue<T> items;

    // Once true, push() discards new items and pop() returns nullopt instead
    // of blocking. Lets threads exit cleanly without needing an external flag.
    bool shutdownFlag = false;
};
