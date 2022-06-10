# bin
if (WIN32)
    message("windows build")
    add_subdirectory(win64)
endif (WIN32)
if (UNIX)
    message("linux build")
    add_subdirectory(unix)
endif (UNIX)