#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

static std::mutex mtx;
constexpr int nummber_1{1};
constexpr int nummber_2{2};
constexpr int nummber_3{3};
static volatile int num_main = nummber_1;
static std::condition_variable cv;
static void num_1(void);
static void num_2(void);
static void num_3(void);

int main(void)
{
    std::thread t_1(num_1);
    std::thread t_2(num_2);
    std::thread t_3(num_3);

    t_1.join();
    t_2.join();
    t_3.join();

    return 0;
}
static void num_1(void)
{
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lck{mtx};

        cv.wait(lck, []
                { return (num_main == nummber_1); });

        std::cout << "1 - ";
        num_main = nummber_2;

        lck.unlock();
        cv.notify_all();
    }
}

static void num_2(void)
{
    while (1)
    {
        std::unique_lock<std::mutex> lck{mtx};

        cv.wait(lck, []
                { return (num_main == nummber_2); });

        std::cout << "2 - ";
        num_main = nummber_3;

        lck.unlock();
        cv.notify_all();
    }
}

static void num_3(void)
{
    while (1)
    {
        std::unique_lock<std::mutex> lck{mtx};

        cv.wait(lck, []
                { return (num_main == nummber_3); });

        std::cout << "3" << std::endl;
        num_main = nummber_1;

        lck.unlock();
        cv.notify_all();
    }
}
