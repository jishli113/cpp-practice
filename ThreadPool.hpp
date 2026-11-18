#include <thread>
#include <vector>
#include <queue>
#pragma once
using namespace std;
class ThreadPool {
public:
    void worker_loop() {
        while (true) {
            function<void()> task;
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [this]{return threads.size() > 0;});
            if (stop && tasks.empty()) {
                return;
            }
            task = move(tasks.front());
            tasks.pop();
            task();
        }

    }
    ThreadPool(size_t pool_size) {
        psize = pool_size;
        threads.reserve(pool_size);
        for (int i = 0; i < pool_size; i ++) {
            threads.emplace_back(&ThreadPool::worker_loop, this);
        }
    }
    void submit(function<void()> fn) {
        lock_guard<mutex> lock(mtx);
        tasks.push(fn);
        cv.notify_one();
    }
    ~ThreadPool() {
        lock_guard<mutex> lock(mtx);
        stop = true;
        cv.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }
private:
    vector<thread> threads;
    size_t psize;
    queue<function<void()>> tasks;
    mutex mtx;
    condition_variable cv;
    bool stop = false;
};