#file: lib
if (WIN32)
    message("windows build")
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

        file(GLOB_RECURSE LLIBS win64/shared/*.lib win64/debug/*.lib)
    else ()
        file(GLOB_RECURSE LLIBS win64/shared/*.lib win64/release/*.lib)
    endif()
    set(LIBS ${LLIBS} PARENT_SCOPE)
    message("current libs: " ${LIBS} )
endif (WIN32)
if (UNIX)
    message("linux build")

    file(GLOB_RECURSE LLIBS unix/*.a)
    
    set(LIBS -lvulkan -lglfw -ldl  -lX11 -lXxf86vm -lXrandr -lXi  -lglslang -lHLSL -lSPIRV -pthread -lrt ${LLIBS} 
	PARENT_SCOPE)
   

endif (UNIX)