#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <cassert>
template <typename T, typename Alloc = std::allocator<T>>
class Vector {
public:
    explicit Vector(size_t n = 0){
        if (n == 0) return;
        start = alloc.allocate(n);
        try {
            std::uninitialized_value_construct_n(start, n);
        } catch (...) {
            alloc.deallocate(start, n);
            throw;
        }
        size_ = capacity_ = n;
    }
    ~Vector() {
        std::destroy(start, start + size_);
        if (start) {
            alloc.deallocate(start, capacity_);
        }
    }
    Vector(const Vector& v) {
        if (v.capacity_ == 0) {
            return;
        }
        T* copy_start = alloc.allocate(v.capacity_);
        try {
            std::uninitialized_copy(v.start, v.start + v.size_, copy_start);
        }
        catch(...) {
            alloc.deallocate(copy_start, v.capacity_);
            throw;
        }
        this->start = copy_start;
        this->size_ = v.size_;
        this->capacity_ = v.capacity_;
    }
    Vector& operator=(const Vector& other) {
        if (this == &other) {
            return *this;
        }
        T* new_start = start;
        bool otherGreater = other.capacity_ > this->capacity_;
        if (otherGreater) {
            new_start = alloc.allocate(other.capacity_);
        }
        else {
            std::destroy(this->start, this->start + this->size_);
            this->size_ = 0;
        }
        try {
            std::uninitialized_copy(other.start, other.start + other.size_, new_start);
        }
        catch(...) {
            if (otherGreater) {
                alloc.deallocate(new_start, other.capacity_);
            }
            throw;
        }
        if (otherGreater) {
            std::destroy(this->start, this->start + this->size_);
            if (this->start) {
                alloc.deallocate(this->start, this->capacity_);
            }
            this->capacity_ = other.capacity_;
        }
        this->start = new_start;
        this->size_ = other.size_;
        return *this;
    }
    Vector(Vector&& move_from) noexcept {
        this->start = move_from.start;
        this->capacity_ = move_from.capacity_;
        this->size_ = move_from.size_;
        move_from.start = nullptr;
        move_from.capacity_ = 0;
        move_from.size_ = 0;
    }
    Vector& operator=(Vector&& move_from) noexcept {
        if (&move_from != this) {
            std::destroy(this->start, this->start + this->size_);
            if (start) {
                alloc.deallocate(this->start, this->capacity_);
            }
            this->start = move_from.start;
            this->capacity_ = move_from.capacity_;
            this->size_ = move_from.size_;
            move_from.start = nullptr;
            move_from.capacity_ = 0;
            move_from.size_ = 0;
        }
        return *this;
    }
    void push_back(T&& elem) {
        if (size_ == capacity_) {
            size_t new_capacity_ = capacity_ * 2;
            if (capacity_ == 0) {
                new_capacity_ = 1;
            }
            T* new_start = alloc.allocate(new_capacity_);
            try {
                std::uninitialized_move(start, start + size_, new_start);
            }
            catch(...) {
                alloc.deallocate(new_start, new_capacity_);
                throw;
            }
            if (start != nullptr) {
                std::destroy(start, start + size_);
                alloc.deallocate(start, capacity_);
            }
            this->capacity_ = new_capacity_;
            this->start = new_start;
        }
        ::new (static_cast<void*>(start + size_)) T(std::move(elem));
        size_++;
    }
    void pop_back() {
        if (this->size_ == 0) {
            return;
        }
        T* back_one = this->start + this->size_ - 1;
        std::destroy(back_one, back_one + 1);
        this->size_ --;
    }
    size_t size() const{
        return this->size_;
    }
    void swap(Vector& other) noexcept {
        size_t t_size = this->size_;
        size_t t_capacity_ = this->capacity_;
        T* t_start = this->start;
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        this->start = other.start;
        other.size_ = t_size;
        other.capacity_ = t_capacity_;
        other.start = t_start;
    }
    T& operator[](size_t i) {
        assert(i < this->size_);
        return this->start[i];
    }
    const T& operator[](size_t i) const {
        assert(i < this->size_);
        return this->start[i];
    }
    size_t capacity() const {
        return this->capacity_;
    }
    bool empty() const {
        return this->size_ == 0;
    }
    void clear() {
        if (this->size_ > 0) {
            std::destroy(this->start, this->start + this->size_);
            this->size_ = 0;
        }
    }
    void reserve(size_t n) {
        if (n > this->capacity_) {
            T* new_start = alloc.allocate(n);
            try {
                std::uninitialized_move_n(this->start, this->size_, new_start);
            }
            catch(...) {
                alloc.deallocate(new_start, n);
                throw;
            }
            std::destroy(this->start, this->start + this->size_);
            if (this->start) {
                alloc.deallocate(this->start, this->capacity_);
            }
            this->start = new_start;
            this->capacity_ = n;
        }
    }
    T* begin() {
        return this->start;
    }
    const T* begin() const {
        return this->start;
    }
    T* end() {
        return (this->start) + this-> size_;
    }
    const T* end() const {
        return (this->start) + this-> size_;
    }
    template <class... Args>
    void emplace_back(Args&&...args) {
        if (this->size_ == this->capacity_) {
            size_t new_capacity = this->capacity_ * 2;
            if (new_capacity == 0){
                new_capacity = 1;
            }
            T* new_start = alloc.allocate(new_capacity);
            try {
                ::new (static_cast<void*>(new_start + this->size_)) T(std::forward<Args>(args)...);
            }
            catch(...) {
                alloc.deallocate(new_start, new_capacity);
                throw;
            }
            try {
                std::uninitialized_move_n(this->start, this->size_, new_start);
            }
            catch(...) {
                std::destroy_at(new_start + this->size_);
                alloc.deallocate(new_start, new_capacity);
                throw;
            }
            std::destroy(this->start, this->start + this->size_);
            if (tbis->start) {
                alloc.deallocate(this->start, this->capacity_);
            }
            this->capacity_ = new_capacity;
            this->start = new_start;
        }
        else {
            ::new (static_cast<void*>(this->start + this->size_)) T(std::forward<Args>(args)...);
        }
        this->size_++;
    }

private:
    size_t size_ = 0;
    T* start = nullptr;
    size_t capacity_ = 0;
    Alloc alloc;
};
