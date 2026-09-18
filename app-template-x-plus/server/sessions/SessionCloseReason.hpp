#pragma once

enum class SessionCloseReason {
    ApplicationRequested,
    Expired,
    ServerShutdown,
    StartupFailed
};
