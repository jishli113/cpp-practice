#include <vector>
#include <functional>
#include <condition_variable>
#include <mutex>

using namespace std;

class CircularQueue {
public:
    CircularQueue(int n) {
        cqueue.resize(n);
        size = n;
    }
    bool producer(function<void()> f) {
        unique_lock<mutex> lock(mtx);
        full.wait(lock, [this]{return (end < start || !looped || closed);});
        if (closed) {
            return false;
        }
        cqueue[end] = move(f);
        end += 1;
        if (end == size) {
            end = 0;
            looped = true;
        }
        empty.notify_one();
        return true;
    }
    function<void()> consumer() {
        unique_lock<mutex> lock(mtx);
        empty.wait(lock, [this]{return (start < end || looped || closed);});
        if (closed && start == end && !looped) {
            return nullptr;
        }
        function<void()> f = move(cqueue[start]);
        start += 1;
        if (start == size) {
            start = 0;
            looped = false;
        }
        full.notify_one();
        return f;
    }
    void close() {
        lock_guard<mutex> lock(mtx);
        closed = true;
        empty.notify_all();
        full.notify_all();
    }
private:
    vector<function<void()>> cqueue;
    mutex mtx;
    condition_variable empty;
    condition_variable full;
    int start = 0;
    int end = 0;
    bool looped = false;
    int size;
    bool closed = false;
};