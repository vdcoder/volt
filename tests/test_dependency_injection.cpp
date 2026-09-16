#include <DependencyInjection.hpp>
#include "../app-template-x-plus/client/src/AppDI.hpp"
#include <cassert>
#include <vector>

struct Provider {
    std::vector<int>& order;
    ~Provider() { order.push_back(1); }
};
struct Consumer {
    voltxp::DependencyInjection& container;
    std::vector<int>& order;
    ~Consumer() {
        assert(&container.resolveDependency<Provider>().order == &order);
        order.push_back(2);
        container.releaseDependencies(); // reentrant base release is harmless
    }
};
struct Runtime : volt::IRuntime {
    int calls = 0;
    void invalidate() override { ++calls; }
};
struct CustomDI : voltxp::DependencyInjection {
    bool& cleaned;
    explicit CustomDI(bool& value) : cleaned(value) {}
    ~CustomDI() override { releaseDependencies(); }
    void releaseDependencies() noexcept override {
        cleaned = true;
        DependencyInjection::releaseDependencies();
    }
};
template<class Exception, class F> void expectThrow(F action) {
    bool caught = false;
    try { action(); } catch (const Exception&) { caught = true; }
    assert(caught);
}
int main() {
    std::vector<int> order;
    voltxp::DependencyInjection di;
    expectThrow<std::runtime_error>([&] { di.resolveDependency<int>(); });
    expectThrow<std::invalid_argument>([&] { di.registerDependency<int>(nullptr); });
    di.registerDependency<int>(std::make_unique<int>(42));
    auto* original = &di.resolveDependency<int>();
    expectThrow<std::logic_error>([&] { di.registerDependency<int>(std::make_unique<int>(7)); });
    assert(&di.resolveDependency<int>() == original && *original == 42);
    di.registerDependency<Provider>(std::unique_ptr<Provider>(new Provider{order}));
    di.registerDependency<Consumer>(std::unique_ptr<Consumer>(new Consumer{di, order}));
    di.releaseDependencies();
    assert((order == std::vector<int>{2, 1}));
    di.releaseDependencies();
    di.registerDependency<int>(std::make_unique<int>(99));
    assert(di.resolveDependency<int>() == 99);

    bool cleaned = false;
    { std::unique_ptr<voltxp::DependencyInjection> derived = std::make_unique<CustomDI>(cleaned); }
    assert(cleaned);
    Runtime runtime;
    { AppDI app; app.registerDependencies(runtime);
      app.resolveDependency<voltxp::VoltRuntimeService>().getRuntime().invalidate(); }
    assert(runtime.calls == 1);
}
