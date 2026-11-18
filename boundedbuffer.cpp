#include <iostream>
#include <mutex>
#include <deque>
#include <condition_variable>
using namespace std;
class BoundedBuffer {
    
public:
    
    BoundedBuffer(int n) {
        buffSize = n;
    }
    
    void insert(int elem) {
        unique_lock<mutex> lock(mtx);
        isFull.wait(lock, [this]{return buffer.size() < buffSize;});
        buffer.push_back(elem);
        isEmpty.notify_one();
    }
    int rem() {
        unique_lock<mutex> lock(mtx);
        isEmpty.wait(lock, [this]{return buffer.size() > 0;});
        int ret = buffer.front();
        buffer.pop_front();
        isFull.notify_one();
        return ret;
    }
private:
    mutex mtx;
    condition_variable isFull;
    condition_variable isEmpty;
    size_t buffSize;
    deque<int> buffer;
    
};

int main()
{
    
}