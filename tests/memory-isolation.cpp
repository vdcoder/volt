#include <MemoryViews.hpp>
#include <emscripten/bind.h>
using namespace voltxp;
int changes = 0;
Handle root;
Handle number;
EMSCRIPTEN_BINDINGS(memory_isolation) {
    emscripten::function("initialize", +[](int value) {
        authoringMemoryStore().setOnChange([](MemoryChange) { ++changes; });
        root = authoringMemoryStore().createRoot();
        number = authoringMemoryStore().allocate<std::int32_t>(root, value);
    });
    emscripten::function("readNumber", +[]() { return Field<std::int32_t>(authoringMemoryStore(), number).get(); });
    emscripten::function("writeNumber", +[](int value) { Field<std::int32_t> field(authoringMemoryStore(), number); field = value; });
    emscripten::function("changeCount", +[]() { return changes; });
}
