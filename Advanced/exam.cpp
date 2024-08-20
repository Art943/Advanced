#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <chrono>

// Vehicle Class
class Vehicle
{
public:
    Vehicle(int id, std::string model, std::string type) : id(id), model(model), type(type) {}

    virtual void print()
    {
        std::cout << "ID: " << id << ", Model: " << model << ", Type: " << type << std::endl;
    }

protected:
    int id;
    std::string model;
    std::string type;
};

// Car Class
class Car : public Vehicle
{
public:
    Car(int id, std::string model, std::string type, int maxPassengers) : Vehicle(id, model, type), maxPassengers(maxPassengers) {}

    void print() override
    {
        std::cout << "ID: " << id << ", Model: " << model << ", Type: " << type << ", Max Passengers: " << maxPassengers << std::endl;
    }

private:
    int maxPassengers;
};

// Truck Class
class Truck : public Vehicle
{
public:
    Truck(int id, std::string model, std::string type, int maxLoadWeight) : Vehicle(id, model, type), maxLoadWeight(maxLoadWeight) {}

    void print() override
    {
        std::cout << "ID: " << id << ", Model: " << model << ", Type: " << type << ", Max Load Weight: " << maxLoadWeight << std::endl;
    }

private:
    int maxLoadWeight;
};

// Circular Buffer Class
template <typename T, int CAPACITY>
class CircularBuffer
{
public:
    CircularBuffer() : head(0), tail(0), count(0) {}

    void produce(const T &data)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv_not_full.wait(lock, [this]
                         { return count != CAPACITY; });

        buffer[tail] = data;
        tail = (tail + 1) % CAPACITY;
        count++;

        lock.unlock();
        cv_not_empty.notify_all();
    }

    T consume()
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv_not_empty.wait(lock, [this]
                          { return count != 0; });

        T data = buffer[head];
        head = (head + 1) % CAPACITY;
        count--;

        lock.unlock();
        cv_not_full.notify_all();

        return data;
    }

private:
    size_t head;
    size_t tail;
    size_t count;
    T buffer[CAPACITY];
    std::mutex mutex;
    std::condition_variable cv_not_empty;
    std::condition_variable cv_not_full;
};

// Function to generate random number within a range
int getRandomNumber(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

// Function to generate unique vehicle ID
int generateUniqueID()
{
    static std::mutex idMutex;
    static int idCounter = 1001; // Initial value

    std::lock_guard<std::mutex> lock(idMutex);
    return idCounter++;
}

// Producer function
void producer(CircularBuffer<Vehicle *, 10> &buffer)
{
    while (true)
    {
        int id = generateUniqueID();
        std::string model;
        std::string type;
        int maxParam;

        // Randomly decide whether to produce a car or a truck
        int vehicleType = getRandomNumber(0, 1);
        if (vehicleType == 0)
        {
            model = "Ford Focus";
            type = "Car";
            maxParam = getRandomNumber(1, 5);
            buffer.produce(new Car(id, model, type, maxParam));
        }
        else
        {
            model = "Ford Maveric";
            type = "Truck";
            maxParam = getRandomNumber(1000, 5000);
            buffer.produce(new Truck(id, model, type, maxParam));
        }

        // Sleep to simulate variable production rates
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// Consumer function
void consumer(CircularBuffer<Vehicle *, 10> &buffer, int id)
{
    while (true)
    {
        Vehicle *vehicle = buffer.consume();
        std::cout << "Consumer " << id << " ";
        vehicle->print();
        delete vehicle;

        // Sleep to simulate variable consumption rates
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << "<num_consumers>" << std::endl;
        return 1;
    }

    int numConsumers = std::stoi(argv[1]);

    if (numConsumers < 2)
    {
        std::cerr << "Number of consumers must be at least 2" << std::endl;
        return 1;
    }

    CircularBuffer<Vehicle *, 10> buffer;

    // Start producer thread
    std::thread producerThread(producer, std::ref(buffer));

    // Start consumer threads
    std::vector<std::thread> consumerThreads;
    for (int i = 0; i < numConsumers; ++i)
    {
        consumerThreads.emplace_back(consumer, std::ref(buffer), i + 1);
    }

    // Join threads
    producerThread.join();
    for (auto &thread : consumerThreads)
    {
        thread.join();
    }

    return 0;
}
