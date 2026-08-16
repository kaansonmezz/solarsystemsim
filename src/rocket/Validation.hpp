#pragma once

#include <string>
#include <utility>
#include <vector>

namespace solar::rocket {

struct ValidationResult final {
    std::vector<std::string> errors;

    [[nodiscard]] bool valid() const noexcept { return errors.empty(); }
    void add(std::string error) { errors.push_back(std::move(error)); }
};

} // namespace solar::rocket
