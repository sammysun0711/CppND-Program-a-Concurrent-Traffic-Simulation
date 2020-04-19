#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    // perform vector modification under the lock
    std::unique_lock<std::mutex> ulck(_mtx);
    _cond.wait(ulck, [this] {return !_messages.empty(); });

    // remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();
    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // perform vector modification under the lock
    std::lock_guard<std::mutex> lck(_mtx);

    // add vector to queue
    std::cout << " Message #" << msg << " will be added to the queue" << std::endl;
    _messages.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        auto msg = _queue->receive();
        std::cout << " Message #" << msg << " has been removed from the queue " << std::endl;
        if (msg == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::toggleCurrentPhase()
{
    switch(this->getCurrentPhase())
    {
        case TrafficLightPhase::red: 
            _currentPhase = TrafficLightPhase::green;
            std::cout << "Toggle TrafficLight from red to green\n";
            break;
        case TrafficLightPhase::green: 
            _currentPhase = TrafficLightPhase::red;
            std::cout << "Toggle TrafficLight from green to red\n";
            break;
        default:
            std::cout << "Not valid TraficLight is set, skipping ...\n"; break;
    }
};

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // Time measurement
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::high_resolution_clock::duration();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4, 6);
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        auto time_span = distr(eng);
        // toggle current phase and reset time measurement
        if (duration.count() > time_span){
            std::cout << "Value of time span is : " << time_span << std::endl;
            this->toggleCurrentPhase();
            // reset the time measurement start point
            start = end;
        }
        // sends an update method to the message queue using move semantics
        //MessageQueue<T>::send(T &&msg)
        //_messages<TrafficLightPhase>.send(std::move(this->getCurrentPhase));
        _queue->send(std::move(this->getCurrentPhase()));
        //_queue.send(std::move(this->getCurrentPhase()));
    }
}
