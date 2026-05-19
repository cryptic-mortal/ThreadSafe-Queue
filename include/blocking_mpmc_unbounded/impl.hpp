#ifndef BLOCKING_MPMC_UNBOUNDED_IMPL
#define BLOCKING_MPMC_UNBOUNDED_IMPL

#include "defs.hpp"

namespace tsfqueue::__impl {
template <typename T> void blocking_mpmc_unbounded<T>::push(T value) {
    std::shared_ptr<T> data = std::make_shared<T>(std::move(value));
    std::unique_ptr<node> dummy = std::make_unique<node>();
    {
        std::lock_guard<std::mutex> lck(tail_mutex);
        tail->data = data;
        tail->next = std::move(dummy);
        tail = tail->next.get();
    }
    cond.notify_one();
}

template <typename T> typename blocking_mpmc_unbounded<T>::node *blocking_mpmc_unbounded<T>::get_tail() {
    std::lock_guard<std::mutex> lck(tail_mutex);
    return tail;
}

template <typename T>
std::unique_ptr<typename blocking_mpmc_unbounded<T>::node> blocking_mpmc_unbounded<T>::wait_and_get() {
    std::unique_lock<std::mutex> lck(head_mutex);
    cond.wait(lck,[this]{return head.get()!=get_tail();});
    std::unique_ptr<node> temp = std::move(head);
    head = std::move(temp->next);
    return temp;
}

template <typename T> std::unique_ptr<typename blocking_mpmc_unbounded<T>::node> blocking_mpmc_unbounded<T>::wait_for_and_get(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lck(head_mutex);
    if(!cond.wait_for(lck, timeout, [this]{return head.get()!=get_tail();})) {
        return nullptr;
    }
    std::unique_ptr<node> temp = std::move(head);
    head = std::move(temp->next);
    return temp;
}

template <typename T> std::unique_ptr<typename blocking_mpmc_unbounded<T>::node> blocking_mpmc_unbounded<T>::try_get() {
    std::lock_guard<std::mutex> lck(head_mutex);
    if(head.get() == get_tail()) {
      return std::unique_ptr<node>();
    }
    std::unique_ptr<node> temp = std::move(head);
    head = std::move(temp->next);
    return temp;
}

template <typename T> void blocking_mpmc_unbounded<T>::wait_and_pop(T &value) {
    std::unique_ptr<node> popped = wait_and_get();
    value = std::move(*popped->data);
}

template <typename T> bool blocking_mpmc_unbounded<T>::wait_for_and_pop(T &value, std::chrono::milliseconds timeout) {
    std::unique_ptr<node> popped = wait_for_and_get(timeout);
    if(popped==nullptr) return false;
    value = std::move(*popped->data);
    return true;
}

template <typename T> std::shared_ptr<T> blocking_mpmc_unbounded<T>::wait_and_pop() {
    std::unique_ptr<node> popped = wait_and_get();
    return std::move(popped->data);
}

template <typename T> std::shared_ptr<T> blocking_mpmc_unbounded<T>::wait_for_and_pop(std::chrono::milliseconds timeout) {
    std::unique_ptr<node> popped = wait_for_and_get(timeout);
    if(popped==nullptr) return nullptr;
    return std::move(popped->data);
}

template <typename T> bool blocking_mpmc_unbounded<T>::try_pop(T &value) {
    std::unique_ptr<node> popped = try_get();
    if(popped) {
        value = std::move(*popped->data);
        return true;
    }
    return false;
}

template <typename T> std::shared_ptr<T> blocking_mpmc_unbounded<T>::try_pop() {
    std::unique_ptr<node> popped = try_get();
    if(popped) return std::move(popped->data);
    return nullptr;
}

template <typename T> bool blocking_mpmc_unbounded<T>::empty() {
    std::lock_guard<std::mutex> lck(head_mutex);
    return head.get() == get_tail();
}

template <typename T>
template <typename... Args>
void blocking_mpmc_unbounded<T>::emplace_back(Args&&... args) {
    std::shared_ptr<T> data = std::make_shared<T>(std::forward<Args>(args)...);
    std::unique_ptr<node> dummy = std::make_unique<node>();
    {
        std::lock_guard<std::mutex> lck(tail_mutex);
        tail->data = data;
        tail->next = std::move(dummy);
        tail = tail->next.get();
    }
    cond.notify_one();
}
}
#endif