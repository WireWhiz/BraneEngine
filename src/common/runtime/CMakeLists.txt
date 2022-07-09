
add_library(runtime STATIC
        "runtime.h"
        "runtime.cpp"
        module.cpp
        module.h
        timeline.cpp
        timeline.h
        logging.cpp)
target_link_libraries(runtime PUBLIC config)