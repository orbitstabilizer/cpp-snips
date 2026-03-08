#include <print>

#include "plugin.hpp"

extern "C" void debug_handle(DebugContext& ctx) {
    ctx.counter++;
    // Change this message to see hot reloading in action!
    std::print("[Plugin v1] Counter: {:<30}\r", ctx.counter);
}
