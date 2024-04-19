cmake_minimum_required(VERSION 3.0)
project(mosquitto_plugin)

# 设置CMake的构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置源文件
set(SOURCES
    mosquitto_plugin.c
    )

# 设置包含的头文件路径
include_directories(
    ./include  # 替换为你的Mosquitto头文件的路径
    )

# 设置链接的库文件路径
link_directories(
    ./lib  # 替换为你的Mosquitto库文件的路径
    )

# 添加动态链接库
add_library(mosquitto_plugin MODULE ${SOURCES})

# 链接Mosquitto的库文件
if(WIN32)
    target_link_libraries(mosquitto_plugin
        mosquitto
        )
elseif(UNIX)
    target_link_libraries(mosquitto_plugin
        libmosquitto.so
        )
endif()

# 设置输出文件路径
set_target_properties(mosquitto_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output
    )

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# 设置调试信息
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

# 设置优化选项
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
