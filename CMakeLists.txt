cmake_minimum_required(VERSION 3.18)

project(WaifuPics VERSION 1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 向编译器添加编译选项
if (MSVC)
    add_compile_options(/W3 /utf-8)
else()
    add_compile_options(-Wall -utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)

qt_standard_project_setup()

# 构建Android建议使用Qt Creator（已测试）
# Android构建需添加 Android OpenSSL
if (ANDROID)
    if(DEFINED ENV{ANDROID_HOME})
        set(AndroidSDK $ENV{ANDROID_HOME})
    elseif(DEFINE ENV{ANDROID_SDK_ROOT})
        set(AndroidSDK $ENV{ANDROID_SDK_ROOT})
    else()
        # 没有找到SDK环境变量
        message(FATAL_ERROR "ANDROID SDK env not found!")
        return()
    endif()
    include(${AndroidSDK}/android_openssl/android_openssl.cmake)
endif()

# 源文件列表
set(SOURCES
    src/main.cpp
    src/widget.cpp
    src/ImageView.cpp
)

# 头文件列表
set(HEADERS
    src/widget.h
    src/ImageView.h
)

# UI 文件列表
set(UIS
    src/widget.ui
)

# 生成可执行文件
qt_add_executable(WaifuPics ${SOURCES} ${HEADERS} ${UIS})

# 链接Qt库
target_link_libraries(WaifuPics PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

if (ANDROID)
    add_android_openssl_libraries(WaifuPics)
endif()

# 添加 Widgets 模块的头文件路径
target_include_directories(WaifuPics SYSTEM PUBLIC
    ${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS}
    ${Qt${QT_VERSION_MAJOR}Network_INCLUDE_DIRS}
)

target_include_directories(WaifuPics PRIVATE
    src/
)
