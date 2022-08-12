#src/assetServer

#add_subdirectory(bin)
add_subdirectory(database)

add_executable(AssetServer main.cpp assetServer.cpp assetServer.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AssetServer PRIVATE runtime config networking ecs utility fileManager database assets_server)
target_compile_definitions(AssetServer PRIVATE ${COMMON_DEFS} ${ASSET_SERVER_DEFS} ASIO_STANDALONE)

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/assetServer)

install(TARGETS AssetServer
        RUNTIME
        DESTINATION ${INSTALL_DIR}
        COMPONENT AssetServer)
install(FILES config.json
        DESTINATION ${INSTALL_DIR}
        COMPONENT AssetServer)
install(CODE "
if(NOT EXISTS ${INSTALL_DIR}/database)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database/data.db
        DESTINATION ${INSTALL_DIR}/database)
endif()
if(NOT EXISTS ${INSTALL_DIR}/assets/defaults)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/defaults
        DESTINATION ${INSTALL_DIR}/assets)
endif()
        "
        COMPONENT AssetServer)

add_custom_command(TARGET AssetServer POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cmake --install . --component AssetServer --prefix ${CMAKE_BINARY_DIR}
        COMMENT "Installing AssetServer...")