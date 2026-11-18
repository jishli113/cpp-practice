#include <iostream>
#include <utility>
#pragma once
template <typename T>
class UniquePtr {
public:
    UniquePtr(T* resource = nullptr) : resource(resource) {}
    UniquePtr(const UniquePtr& ptr) = delete;
    UniquePtr& operator=(const UniquePtr& copy) = delete;
    UniquePtr(UniquePtr&& move_from) noexcept : resource(move_from.resource) {
        move_from.resource = nullptr;
    }
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            T* data = other.resource;
            other.resource = nullptr;
            delete(this->resource);
            this->resource = data;
        }
        return *this;
    }
    T& operator*() const {
        return *resource;
    }
    T* operator->() const {
    return resource;
    }
    ~UniquePtr() {
        delete(resource);
    }
    T* get() const {
        return resource;
    }
    T* release() noexcept {
        T* data = resource;
        resource = nullptr;
        return data;
    }
    void swap(UniquePtr& other) noexcept {
        T* temp = other.resource;
        other.resource = this->resource;
        this->resource = temp;
    }
    void reset(T* new_resource = nullptr) noexcept {
        // If new pointer isnt the same as old pointer (would free, leaving a dangling pointer)
        if (resource != new_resource) {
            T* temp = this->resource;
            this->resource = new_resource;
            delete(temp);
        }
    }
private:
    T* resource;
};
