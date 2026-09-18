#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace voltxp {
// Structured action payloads, independent of handle-based storage changes.
struct DataValue {
    using Object = std::map<std::string, DataValue>;
    using List = std::vector<std::pair<std::string, DataValue>>;
    using Storage = std::variant<bool, std::int64_t, double, std::string, Object, List>;
    Storage value;
    DataValue(bool v) : value(v) {}
    DataValue(int v) : value(std::int64_t(v)) {}
    DataValue(std::int64_t v) : value(v) {}
    DataValue(double v) : value(v) {}
    DataValue(std::string v) : value(std::move(v)) {}
    DataValue(const char* v) : value(std::string(v)) {}
    DataValue(Object v) : value(std::move(v)) {}
    DataValue(List v) : value(std::move(v)) {}
    bool operator==(const DataValue& other) const { return value == other.value; }
};

struct ActionMessage { std::string name; DataValue payload; };
} // namespace voltxp
