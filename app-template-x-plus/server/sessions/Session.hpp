#pragma once
#include "SessionBase.hpp"

// Customize per-session application behavior here.
class Session : public SessionBase {
    bool m_echoRegistered = false;
public:
    explicit Session(std::string id);
protected:
    void onStarted() override;
    void onClosed(SessionCloseReason reason) noexcept override;
};
