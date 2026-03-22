# FindQt6.cmake
# Helper used when Qt6 is installed in a non-system path.
# Usage: cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64 ..
# This file is a thin wrapper; the official Qt6 CMake files handle the real work.

# Common Qt6 install locations to probe
set(_qt6_search_paths
    "$ENV{HOME}/Qt/6.6.3/gcc_64"
    "$ENV{HOME}/Qt/6.6.3/clang_64"
    "$ENV{HOME}/Qt/6.6.3/msvc2019_64"
    "C:/Qt/6.6.3/msvc2019_64"
    "C:/Qt/6.6.3/mingw_64"
    "/opt/Qt/6.6.3/gcc_64"
    "/usr/local/Qt-6.6.3"
)

foreach(_path ${_qt6_search_paths})
    if(EXISTS "${_path}/lib/cmake/Qt6")
        list(APPEND CMAKE_PREFIX_PATH "${_path}")
        message(STATUS "[OpenSheet] Found Qt6 hint: ${_path}")
        break()
    endif()
endforeach()

# Verify Qt6 is findable
find_package(Qt6 QUIET COMPONENTS Core)
if(NOT Qt6_FOUND)
    message(WARNING
        "[OpenSheet] Qt6 not found automatically.\n"
        "  Set CMAKE_PREFIX_PATH to your Qt6 installation, e.g.:\n"
        "  cmake -DCMAKE_PREFIX_PATH=/home/user/Qt/6.6.3/gcc_64 .."
    )
endif()
