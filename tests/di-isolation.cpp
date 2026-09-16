#include "../app-template-x-plus/client/src/ApplicationServices.hpp"
#include <emscripten.h>
#include <emscripten/bind.h>

struct Runtime : volt::IRuntime {
    int invalidations = 0;
    void invalidate() override { ++invalidations; }
};
Runtime runtime;
EMSCRIPTEN_BINDINGS(di_isolation) {
    emscripten::function("initialize", +[](int value) {
        services().registerDependencies(runtime);
        services().registerDependency<int>(std::make_unique<int>(value));
    });
    emscripten::function("readService", +[]() { return services().resolveDependency<int>(); });
    emscripten::function("readInvalidations", +[]() { return runtime.invalidations; });
    emscripten::function("invalidate", &invalidate);
    emscripten::function("schedule", +[](int delay) {
        emscripten_async_call([](void*) {
            ++services().resolveDependency<int>();
            invalidate();
        }, nullptr, delay);
    });
}
