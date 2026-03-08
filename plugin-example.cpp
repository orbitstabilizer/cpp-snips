#include <chrono>
#include <thread>

#define PLUGIN_IMPL
#include "plugin.hpp"


int main() {
    DebugContext ctx = {0, 0.0f};
    plug_reload();

    printf("[Host] Starting main loop. Modify and recompile plugin.cpp to see hot reload in action.\n");
    while (true) {
        plug_reload();
        plug_next_frame(ctx);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
