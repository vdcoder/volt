#pragma once
#include <cstdint>
#include <limits>
#include <string>
#include <variant>

namespace voltxp {
struct Handle {
    std::uint32_t slot = (std::numeric_limits<std::uint32_t>::max)();
    std::uint32_t generation = 0;
    bool operator==(Handle other) const noexcept { return slot == other.slot && generation == other.generation; }
    bool operator!=(Handle other) const noexcept { return !(*this == other); }
    explicit operator bool() const noexcept { return generation != 0; }
};
enum class MemoryType { Bool, I32, I64, F32, F64, String, Container };
using MemoryValue = std::variant<std::monostate, bool, std::int32_t, std::int64_t, float, double, std::string>;
enum class MemoryOperation { AllocateContainer, AllocateValue, SetValue, RemoveContainer };
struct MemoryChange {
    MemoryOperation operation;
    MemoryType type;
    Handle parent; // supplied for allocation/removal; empty for value updates
    Handle handle;
    MemoryValue value{};
};
static_assert(sizeof(Handle) == 8, "Handles must occupy 64 bits");
} // namespace voltxp
