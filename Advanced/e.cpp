#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <array>
#include <chrono>
#include <cstdlib>

template <typename T, int CAPACITY>
class CircularBuffer
{
private:
    T buffer[4][CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
    size_t item{0};
    std::mutex mtx;
    std::condition_variable cv;

public:
    using Type = T;
    CircularBuffer() : head(0), tail(0), count(0)
    {
        static_assert(CAPACITY > 7, "Buffer size must be at least 8");
    }

    CircularBuffer(const CircularBuffer &) = delete;
    CircularBuffer &operator=(const CircularBuffer &) = delete;

    void clear(void)
    {
        head = tail = count = 0;
    }

    T consume(void)
    {
        std::unique_lock lock{mtx};
        cv.wait(lock, [this]
                { return !this->empty(); });
        T data = buffer[item][count];
        head = (head + 1) % CAPACITY;
        if (count != CAPACITY)
        {
            count--;
            item = 0;
        }
        lock.unlock();
        cv.notify_all();
        return data;
    }

    void produce(const T &data)
    {
        std::unique_lock lock{mtx};
        cv.wait(lock, [this]
                { return !this->full(); });
        buffer[item][tail] = data;
        tail = (tail + 1) % CAPACITY;
        if (count == CAPACITY)
        {
            head = (head + 1) % CAPACITY;
        }
        if (count != CAPACITY)
        {
            count++;
            item = 0;
        }
        lock.unlock();
        cv.notify_all();
    }

    bool full(void) const
    {
        return count == CAPACITY;
    }

    bool empty(void) const
    {
        return count == 0;
    }
};

class Vehicle
{
};

class Truck : public Vehicle
{
public:
    void operator()()
    {
        std::cout << "Produced Truck" << std::endl;
    }
};

class Car : public Vehicle
{
public:
    void operator()()
    {
        std::cout << "Produced Car" << std::endl;
    }
};

template <typename T, size_t CAPACITY>
static void consumer(CircularBuffer<T, CAPACITY> &buffer)
{
    while (true)
    {
        T value = buffer.consume();
        std::cout << "Consumed: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + (rand() % 501)));
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <num_consumers>" << std::endl;
        return 1;
    }

    int CONSUMERS = std::stoi(argv[1]);
    if (CONSUMERS < 2)
    {
        std::cerr << "There must be at least 2 consumers" << std::endl;
        return 1;
    }

    CircularBuffer<int, CAPACITY> buffer;
    std::array<std::thread, 10> cthreads;

    std::srand(std::time(nullptr));
    for (int i = 0; i < CONSUMERS; i++)
    {
        cthreads[i] = std::thread{consumer<decltype(buffer)::Type, decltype(buffer)::CAPACITY>, std::ref(buffer)};
    }

    for (int i = 0; i < CONSUMERS; i++)
    {
        if (cthreads[i].joinable())
        {
            cthreads[i].join();
        }
    }

    return 0;
}
