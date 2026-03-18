#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <cstddef>

template<typename T, size_t N>
class CircularBuffer {

    public:
        CircularBuffer();

        void clear();
        void push(const T& value);

        bool empty() const;
        bool full() const;

        size_t size() const;
        constexpr size_t capacity() const;

        bool latest(T& value) const;
        bool oldest(T& value) const;
        bool at(size_t index, T& value) const;

    private:
        T buffer_[N];
        size_t head_;
        size_t count_;
};


template<typename T, size_t N>
CircularBuffer<T, N>::CircularBuffer() :
    head_(0), 
    count_(0) 
{}


template<typename T, size_t N>
void CircularBuffer<T, N>::clear() {
    head_ = 0;
    count_ = 0;
}


template<typename T, size_t N>
void CircularBuffer<T, N>::push(const T& value) {
    buffer_[head_] = value;
    head_ = (head_ + 1) % N;
    if (count_ < N)
        count_++;
}


template<typename T, size_t N>
bool CircularBuffer<T, N>::empty() const {
    return (count_ == 0);
}


template<typename T, size_t N>
bool CircularBuffer<T, N>::full() const {
    return (count_ == N);
}


template<typename T, size_t N>
size_t CircularBuffer<T, N>::size() const {
    return count_;
}


template<typename T, size_t N>
constexpr size_t CircularBuffer<T, N>::capacity() const {
    return N;
}


template<typename T, size_t N>
bool CircularBuffer<T, N>::latest(T& value) const {
    if (!empty()) {
        size_t last_index = (head_ + N - 1) % N;
        value = buffer_[last_index];
        return true;
    }
    return false;
}


template<typename T, size_t N>
bool CircularBuffer<T, N>::oldest(T& value) const {
    if (!empty()) {
        size_t oldest_index = (head_ + N - count_) % N;
        value = buffer_[oldest_index];
        return true;
    }
    return false;
}


template<typename T, size_t N>
bool CircularBuffer<T, N>::at(size_t index, T& value) const {
    if (index >= count_) return false;
    size_t oldest_index = (head_ + N - count_) % N;
    size_t real_index = (oldest_index + index) % N;
    value = buffer_[real_index];
    return true;
}


#endif /*CIRCULAR_BUFFER_HPP*/