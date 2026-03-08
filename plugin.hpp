#pragma once

#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <thread>
#include <dlfcn.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

constexpr const char* PLUG_PATH = "./build/plugin.so";

struct DebugContext {
    int counter;
    float runtime;
};

extern "C" void debug_handle(DebugContext& ctx);

typedef void (*debug_handle_t)(DebugContext& ctx);


extern void* lib_handle;
extern debug_handle_t plugin_update;
extern fs::file_time_type last_load_time;

#ifdef PLUGIN_IMPL
void* lib_handle = nullptr;
debug_handle_t plugin_update = nullptr;
fs::file_time_type last_load_time;
#endif


inline void plug_reload() {
    if (fs::exists(PLUG_PATH)) {
        auto current_write_time = fs::last_write_time(PLUG_PATH);
        if (current_write_time > last_load_time) {
            // Small sleep to ensure the compiler has finished writing the file
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            // No changes detected, no need to reload
            return;
        }
    }

    if (lib_handle) {
        dlclose(lib_handle);
        std::cout << "[Host] Unloaded old plugin." << std::endl;
    }
    
    if (!fs::exists(PLUG_PATH)) return;
    lib_handle = dlopen(PLUG_PATH, RTLD_NOW);
    if (!lib_handle) {
        std::cerr << "[Host] Load Error: " << dlerror() << std::endl;
        return;
    }
    
    plugin_update = (debug_handle_t)dlsym(lib_handle, "debug_handle");
    last_load_time = fs::last_write_time(PLUG_PATH);
    std::cout << "[Host] Loaded new plugin version." << std::endl;
}


inline void plug_next_frame(DebugContext& ctx) {
    if (plugin_update) {
        plugin_update(ctx);
    }

}

#endif // PLUGIN_HPP
