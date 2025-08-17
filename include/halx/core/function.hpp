#pragma once

#include <functional>
#include <memory>
#include <utility>

namespace halx::core {

template <class T> class Function;

template <class R, class... Args> class Function<R(Args...)> {
public:
  Function() = default;

  Function(std::move_only_function<R(Args...)> &&function)
      : function_{std::make_unique<std::move_only_function<R(Args...)>>(
            std::move(function))} {}

  std::move_only_function<R(Args...)> *c_ptr() const { return function_.get(); }

  static inline R call(void *c_ptr, Args... args) {
    auto function = static_cast<std::move_only_function<R(Args...)> *>(c_ptr);
    return (*function)(std::forward<Args>(args)...);
  }

private:
  std::unique_ptr<std::move_only_function<R(Args...)>> function_;
};

} // namespace halx::core