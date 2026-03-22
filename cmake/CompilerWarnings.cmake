# CompilerWarnings.cmake
# Call opensheet_set_warnings(target) on any library or executable
# to apply a consistent, strict warning set.

function(opensheet_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /wd4100   # unreferenced formal parameter
            /wd4127   # conditional expression is constant
            /wd4456   # local variable hides outer scope
            /wd4458   # class member hides outer scope
            /wd4459   # global declaration hides local
            /wd4702   # unreachable code (Qt emits these)
            /permissive-
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
            -Wno-unused-parameter
            -Wno-missing-field-initializers
        )
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} PRIVATE
                -Wlogical-op
                -Wuseless-cast
                -Wno-maybe-uninitialized
            )
        endif()
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target} PRIVATE
                -Wno-gnu-zero-variadic-macro-arguments
            )
        endif()
    endif()
endfunction()

# Treat warnings as errors in CI (set OPENSHEET_WARNINGS_AS_ERRORS=ON)
function(opensheet_warnings_as_errors target)
    if(OPENSHEET_WARNINGS_AS_ERRORS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /WX)
        else()
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
