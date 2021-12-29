#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

class QueueBuffer {
public:
    QueueBuffer(int size) {
        maxSize = size;
    }
    
    void Produce(int i) {
        std::unique_lock<std::mutex> lock(mutex);
        
        while (queue.size() == maxSize) {
            std::cout << "produce wait..." << std::endl;
            cond_var.wait(lock);
        }
        
        
        queue.push_back(i);
        cond_var.notify_all();
    }
    
    int Consume() {
        std::unique_lock<std::mutex> lock(mutex);
        
        cond_var.wait(lock, [this](){ return queue.size() > 0; });
        
        int data = queue.front();
        queue.pop_front();
        
        cond_var.notify_one();
        
        return data;
    }
    
private:
    std::mutex mutex;
    std::condition_variable cond_var;
    
    int maxSize;
    std::deque<int> queue;
};

std::mutex printmu;
void Producer(int id, QueueBuffer& buffer) {
    for (int i=0; i<60; ++i) {
        
        printmu.lock();
        std::cout << "p " << id << " Produce " << i << std::endl;
        printmu.unlock();
        
        buffer.Produce(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Producer " << id << "Done" << std::endl;
}

void Consumer(int id, QueueBuffer& buffer) {
    for (int i=0; i<60; ++i) {
        int value = buffer.Consume();
        
        printmu.lock();
        std::cout << "c " << id << " Consumer " << value << std::endl;
        printmu.unlock();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "Consumer " << id << "Done" << std::endl;
}

int main() {
    QueueBuffer buffer(4);
    
    std::thread producer1(Producer, 1, std::ref(buffer));
    std::thread producer2(Producer, 2, std::ref(buffer));
    //std::thread producer3(Producer, 3, std::ref(buffer));
    
    std::thread consumer1(Consumer, 1, std::ref(buffer));
    std::thread consumer2(Consumer, 2, std::ref(buffer));
    
    producer1.join();
    producer2.join();
    //producer3.join();
    
    consumer1.join();
    consumer2.join();
    return 0;
}
