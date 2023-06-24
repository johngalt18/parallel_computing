#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

const int num_philosophers = 5;

class Fork {
public:
    mutex mtx;
};

class Philosopher {
public:
    Philosopher(int id, Fork& left_fork, Fork& right_fork)
        : id(id), left_fork(left_fork), right_fork(right_fork), num_eats(0) {}

    void operator()() {
        while (true) {
            cout << "Philosopher " << id << " is thinking.\n";
            this_thread::sleep_for(std::chrono::milliseconds(3000));

            // Pick up left fork
            unique_lock<mutex> left_lock(left_fork.mtx);
            cout << "Philosopher " << id << " picked up left fork.\n";

            // Try to pick up right fork
            bool got_right_fork = false;
            while (!got_right_fork) {
                if (right_fork.mtx.try_lock()) {
                    cout << "Philosopher " << id << " picked up right fork.\n";
                    got_right_fork = true;
                } else {
                    // Release left fork and try again later
                    left_lock.unlock();
                    this_thread::sleep_for(chrono::milliseconds(3000));
                    left_lock.lock();
                    cout << "Philosopher " << id << " is waiting for right fork.\n";
                }
            }

            cout << "Philosopher " << id << " is eating.\n";
            this_thread::sleep_for(chrono::milliseconds(3000));
            num_eats++;
            cout << "Philosopher " << id << " ate " << num_eats << " times.\n";

            right_fork.mtx.unlock();
            cout << "Philosopher " << id << " released right fork.\n";
            left_lock.unlock();
            cout << "Philosopher " << id << " released left fork.\n";
        }
    }

    int get_num_eats() {
        return num_eats; // dont know why i get always 0
    }

private:
    int id;
    Fork& left_fork;
    Fork& right_fork;
    int num_eats;
};

int main() {
    array<Fork, num_philosophers> forks;
    

    array<Philosopher, num_philosophers> philosophers = {
        Philosopher(0, forks[0], forks[1]),
        Philosopher(1, forks[1], forks[2]),
        Philosopher(2, forks[2], forks[3]),
        Philosopher(3, forks[3], forks[4]),
        Philosopher(4, forks[4], forks[0])
    };

    array<thread, num_philosophers> threads;
    for (int i = 0; i < num_philosophers; i++) {
        threads[i] = thread(philosophers[i]);
    }

    // Run for a fixed amount of time
    this_thread::sleep_for(chrono::seconds(10));
    
    for (int i = 0; i < num_philosophers; i++) {
        threads[i].detach();
    }

    return 0;
}