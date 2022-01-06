add_library(http "HTTPRequest.cpp" "HTTPServer.cpp" "HTTPServer.h" "HTTPRequest.h")
target_link_libraries(http PUBLIC acme_lw)