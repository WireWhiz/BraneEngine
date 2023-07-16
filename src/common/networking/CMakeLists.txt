#src/common/networking
find_package(asio REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(robin_hood REQUIRED)

add_library(networking STATIC 
	"networking.cpp"
	"connection.cpp")
 target_link_libraries(networking PUBLIC asio::asio OpenSSL::SSL jsoncpp::jsoncpp robin_hood::robin_hood)

 if(win32)
    target_compile_definitions(networking PUBLIC _WIN32_WINNT_WIN10=0x0A00 _WIN32_WINNT=0x0A00)
endif(win32)