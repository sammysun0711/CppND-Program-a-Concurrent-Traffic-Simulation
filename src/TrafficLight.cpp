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
    T msg = std::move(_messages.front());
    _messages.pop_front();
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
    // std::cout << " Message #" << msg << " will be added to the queue" << std::endl;
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
        auto msg = _message_queue.receive();
        // std::cout << " Message #" << msg << " has been removed from the queue " << std::endl;
        if (msg == TrafficLightPhase::green)
        {
            std::cout << "Receive message of TrafficLightPhase green!\n";
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}
/*
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
*/

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
    std::chrono::duration<double> duration;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4, 6);
    int time_span = distr(eng);
    while(true)
    {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        // toggle current phase and reset time measurement
        if (duration.count() > time_span){
            // std::cout << "Value of time span is : " << time_span << std::endl;
            // trigger _currentPhase
            if (_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }
            // sends an update method to the message queue using move semantics
            _message_queue.send(std::move(_currentPhase));
            // reset the time measurement time span and start point
            time_span = distr(eng);
            start = std::chrono::high_resolution_clock::now();
        }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}
