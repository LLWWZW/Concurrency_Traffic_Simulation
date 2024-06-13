#include "TrafficLight.h"
#include <future>
#include <iostream>
#include <random>
/* Implementation of class "MessageQueue" */

/*

*/

/* Implementation of class "TrafficLight" */

/*




*/

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    // LW: solving
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_msg_queue->receive() == green)
        {
            return;
        }
    }
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

template <typename T> T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    // LW: solving
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_msgs.empty(); });

    auto message = std::move(_msgs.back());
    _msgs.pop_back();
    return message;
}

template <typename T> void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // LW: solving
    std::lock_guard<std::mutex> lck(_mutex);
    // add vector to queue
    std::cout << "   Message " << msg << " will be added to the queue" << std::endl;
    _msgs.push_back(std::move(msg));

    _cond.notify_one();
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method
    // „simulate“ is called.To do this, use the thread queue in the base class.
    // LW
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    // LW: solving

    auto lastUpdate = std::chrono::system_clock::now();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(4, 6);

    long random_duration = dis(gen);
    while (true)
    {
        // sleep at every iteration to reduce CPU usage

        // std::this_thread::sleep_for(std::chrono::milliseconds(random_duration * 1000));
        long timeSinceUpdated =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate)
                .count();

        // first loop of distinguishing
        if (random_duration <= timeSinceUpdated)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            _currentPhase = (_currentPhase == red) ? green : red;

            auto is_sent = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _msg_queue,
                                      std::move(_currentPhase));
            is_sent.wait();
            lastUpdate = std::chrono::system_clock::now();
            random_duration = dis(gen);
        }
    }
}