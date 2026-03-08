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
