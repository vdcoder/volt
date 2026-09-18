#pragma once
#include "MemoryChanges.hpp"
#include "Talkers.hpp"
#include <cstring>
#include <type_traits>

namespace voltxp {
// One complete change per talker message. References are uint32 slots, never generations.
inline MessageBytes encodeChange(const MemoryChange& change) {
    MessageBytes bytes{static_cast<std::uint8_t>(change.operation), static_cast<std::uint8_t>(change.type)};
    auto put = [&](std::uint64_t value, unsigned size) {
        for (unsigned i = 0; i < size; ++i) bytes.push_back(static_cast<std::uint8_t>(value >> (8 * i)));
    };
    if (change.operation != MemoryOperation::SetValue) put(change.parent.slot, 4);
    put(change.handle.slot, 4);
    if (change.operation == MemoryOperation::AllocateValue || change.operation == MemoryOperation::SetValue) {
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) throw std::invalid_argument("Missing scalar");
            else if constexpr (std::is_same_v<T, std::string>) {
                if (value.size() > 1024 * 1024) throw std::length_error("String too large");
                put(value.size(), 4); bytes.insert(bytes.end(), value.begin(), value.end());
            } else if constexpr (std::is_floating_point_v<T>) {
                using Bits = std::conditional_t<sizeof(T) == 4, std::uint32_t, std::uint64_t>;
                Bits bits; std::memcpy(&bits, &value, sizeof(T)); put(bits, sizeof(T));
            } else put(static_cast<std::uint64_t>(value), sizeof(T));
        }, change.value);
    }
    return bytes;
}
inline MemoryChange decodeChange(MessageView bytes) {
    std::size_t offset = 0;
    auto get = [&](unsigned size) {
        if (size > bytes.size - offset) throw std::invalid_argument("Truncated memory change");
        std::uint64_t value = 0;
        for (unsigned i = 0; i < size; ++i) value |= std::uint64_t(bytes.data[offset++]) << (8 * i);
        return value;
    };
    auto op = get(1), type = get(1);
    if (op > 3 || type > 6) throw std::invalid_argument("Unknown memory operation/type");
    MemoryChange change{static_cast<MemoryOperation>(op), static_cast<MemoryType>(type), {}, {}};
    if (change.operation != MemoryOperation::SetValue) change.parent = {static_cast<std::uint32_t>(get(4)), 1};
    change.handle = {static_cast<std::uint32_t>(get(4)), 1};
    // Root is established locally, permanent for the connection.
    if ((change.type == MemoryType::Container && change.handle.slot == 0) ||
        (change.operation != MemoryOperation::SetValue && change.parent.slot == UINT32_MAX))
        throw std::invalid_argument("Invalid root operation");
    if (change.operation == MemoryOperation::AllocateValue || change.operation == MemoryOperation::SetValue) {
        switch (change.type) {
        case MemoryType::Bool: { auto v = get(1); if (v > 1) throw std::invalid_argument("Invalid bool"); change.value = bool(v); break; }
        case MemoryType::I32: { auto bits = static_cast<std::uint32_t>(get(4)); std::int32_t v; std::memcpy(&v, &bits, 4); change.value = v; break; }
        case MemoryType::I64: { auto bits = get(8); std::int64_t v; std::memcpy(&v, &bits, 8); change.value = v; break; }
        case MemoryType::F32: { auto bits = static_cast<std::uint32_t>(get(4)); float v; std::memcpy(&v, &bits, 4); change.value = v; break; }
        case MemoryType::F64: { auto bits = get(8); double v; std::memcpy(&v, &bits, 8); change.value = v; break; }
        case MemoryType::String: {
            auto size = get(4);
            if (size > 1024 * 1024 || size > bytes.size - offset) throw std::invalid_argument("Invalid string length");
            change.value = std::string(reinterpret_cast<const char*>(bytes.data + offset), static_cast<std::size_t>(size)); offset += static_cast<std::size_t>(size); break;
        }
        default: throw std::invalid_argument("Invalid scalar type");
        }
    }
    if (offset != bytes.size) throw std::invalid_argument("Trailing memory bytes");
    return change;
}
} // namespace voltxp
