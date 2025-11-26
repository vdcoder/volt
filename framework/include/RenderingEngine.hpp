#pragma once

namespace volt {

    class VoltEngine;

    // This is set by VoltEngine before calling app->render()
    // Allows a simpler API for users
    // ALERT: It is purposefully only available during the render() call!
    thread_local VoltEngine* g_pRenderingEngine = nullptr;

}