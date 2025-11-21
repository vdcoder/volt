#pragma once

namespace volt {

    class VoltRuntime;

    // This is set by VoltRuntime before calling app->render()
    // Allows a simpler API for users
    // ALERT: It is purposefully only available during the render() call!
    thread_local VoltRuntime* g_pRenderingRuntime = nullptr;

}