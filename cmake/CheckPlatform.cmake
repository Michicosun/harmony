if(NOT DEFINED UNIX)
    set(UNIX false)
endif()

set(LINUX false)
if(DEFINED UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("Running on Linux")
else()
    message(FATAL_ERROR
        "Unsupported platform, required Linux"
    )
endif()
