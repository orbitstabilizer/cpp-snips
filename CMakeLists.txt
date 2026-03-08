cmake_minimum_required(VERSION 3.16)
project(cpp-snips)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_CC "${CMAKE_BINARY_DIR}/compile_commands.json")
set(TARGET_CC "${CMAKE_SOURCE_DIR}/compile_commands.json")

# Create the symlink after the build starts
add_custom_command(
    OUTPUT "${TARGET_CC}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${SOURCE_CC}" "${TARGET_CC}"
    COMMENT "Creating symlink for compile_commands.json"
)

# You need a target to trigger the custom command
add_custom_target(copy_compile_commands ALL DEPENDS "${TARGET_CC}")

set(CMAKE_CXX_STANDARD 23)
add_executable(hello-world hello-world.cpp)

add_subdirectory(external/glaze)
add_executable(glaze-hello-world glaze-hello-world.cpp)
target_link_libraries(glaze-hello-world PRIVATE glaze::glaze)


add_executable(cbind cbind.cpp)
target_link_libraries(cbind PRIVATE glaze::glaze)

add_library(plugin SHARED plugin.cpp)
set_target_properties(plugin PROPERTIES PREFIX "" SUFFIX ".so")

# Custom command to "touch" the file or move it to a 'live' location
# This prevents the Host from loading a partially written file
add_custom_command(TARGET plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Plugin rebuild complete."
)

add_executable(plugin-example plugin-example.cpp)
