#pragma once

#include <DXD/Event.h>
#include <cassert>
#include <memory>
#include <mutex>

template <typename Data>
class EventImpl : public DXD::Event<Data> {
    EventImpl() = default;
    friend DXD::Event<Data>;

public:
    void signal(const Data &data) override {
        auto lock = this->lock();
        this->data = std::make_unique<Data>(data);
        cv.notify_all();
    }

    bool isComplete() const override {
        return data != nullptr;
    }

    const Data &getData() const override {
        auto lock = this->lock();
        assert(data != nullptr);
        return *data;
    }

    const Data &wait() const override {
        auto lock = this->lock();
        while (data == nullptr) {
            cv.wait(lock);
        }
        return *data;
    }

private:
    auto lock() const {
        return std::unique_lock<std::mutex>{mutex};
    }

    mutable std::mutex mutex{};
    mutable std::condition_variable cv{};
    std::unique_ptr<Data> data{};
};
