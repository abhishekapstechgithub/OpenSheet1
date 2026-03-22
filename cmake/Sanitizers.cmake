# Sanitizers.cmake
# Enable address, undefined, or thread sanitizers for Debug/CI builds.
# Usage: cmake -DOPENSHEET_SANITIZE=address ..
#        cmake -DOPENSHEET_SANITIZE=undefined ..
#        cmake -DOPENSHEET_SANITIZE=thread ..

option(OPENSHEET_SANITIZE "Sanitizer to enable (address|undefined|thread|none)" "none")

function(opensheet_enable_sanitizers target)
    if("${OPENSHEET_SANITIZE}" STREQUAL "none" OR
       "${OPENSHEET_SANITIZE}" STREQUAL "")
        return()
    endif()

    if(MSVC)
        if("${OPENSHEET_SANITIZE}" STREQUAL "address")
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_link_options(${target} PRIVATE /fsanitize=address)
        else()
            message(WARNING "Only AddressSanitizer is supported on MSVC")
        endif()
        return()
    endif()

    # GCC / Clang
    set(_flags "-fsanitize=${OPENSHEET_SANITIZE} -fno-omit-frame-pointer")
    if("${OPENSHEET_SANITIZE}" STREQUAL "address")
        string(APPEND _flags " -fsanitize-address-use-after-scope")
    endif()

    target_compile_options(${target} PRIVATE ${_flags})
    target_link_options(${target} PRIVATE ${_flags})

    message(STATUS "[OpenSheet] Sanitizer enabled for ${target}: ${OPENSHEET_SANITIZE}")
endfunction()
