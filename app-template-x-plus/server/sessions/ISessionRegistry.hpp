#pragma once
#include "SessionBase.hpp"

// Networking needs a session, without knowing the app's derived type.
class ISessionRegistry {
public:
    virtual ~ISessionRegistry() = default;
    virtual SessionBase& getOrCreate(const std::string& id) = 0;
};
