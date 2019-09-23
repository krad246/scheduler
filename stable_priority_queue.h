#ifndef STABLE_PRIORITY_QUEUE_H_
#define STABLE_PRIORITY_QUEUE_H_

#include <queue>

template <class T>
struct stable_element
{
    stable_element(T&& o, std::size_t c)
        : object_(std::move(o))
        , insertion_order_(c)
    {
    }
    stable_element(const T& o, std::size_t c)
        : object_(o)
        , insertion_order_(c)
    {
    }
    operator T() { return object_; }

    T object_;
    std::size_t insertion_order_;
};

template <class T>
bool operator<(const stable_element<T>& lhs, const stable_element<T>& rhs)
{
    return (lhs.object_ < rhs.object_) || (!(rhs.object_ < lhs.object_) && (rhs.insertion_order_ < lhs.insertion_order_));
}

template <class T,
          class Container = std::vector<stable_element<T>>,
          class Compare = std::less<typename Container::value_type>>
class stable_priority_queue : public std::priority_queue<stable_element<T>, Container, Compare>
{
    using stableT = stable_element<T>;
    using std::priority_queue<stableT, Container, Compare>::priority_queue;
public:
    const T& top() { return this->c.front().object_; }
    void push(const T& value) {
        this->c.push_back(stableT(value, counter_++));
        std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    void push(T&& value) {
        this->c.push_back(stableT(std::move(value), counter_++));
        std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    template<class ... Args>
    void emplace(Args&&... args) {
        this->c.emplace_back(T(std::forward<Args>(args)...), counter_++);
        std::push_heap(this->c.begin(), this->c.end(), this->comp);
    }
    void pop() {
        std::pop_heap(this->c.begin(), this->c.end(), this->comp);
        this->c.pop_back();
        if (this->empty()) counter_ = 0;
    }

protected:
    std::size_t counter_ = 0;
};

#endif
