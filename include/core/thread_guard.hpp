#ifndef CONCURRENCY_TOOLKIT_CORE_THREAD_GUARD_HPP
#define CONCURRENCY_TOOLKIT_CORE_THREAD_GUARD_HPP

#include <thread>

namespace concurrency {
namespace core {

class ThreadGuard {
private:
    std::thread& t;

public:
    explicit ThreadGuard(std::thread& t_): t(t_) {}; 
    
    ~ThreadGuard() {
        if(t.joinable()) t.join();
    }

    ThreadGuard(ThreadGuard const&) = delete;
    ThreadGuard& operator=(ThreadGuard const&) = delete;
    
    ThreadGuard(ThreadGuard const&&) = delete;
    ThreadGuard& operator=(ThreadGuard const&&) = delete;
};

}
} 

#endif