add_library(assetHttpServer STATIC assetHttpServer.cpp assetHttpServer.h)
target_link_libraries(assetHttpServer PUBLIC json config http)