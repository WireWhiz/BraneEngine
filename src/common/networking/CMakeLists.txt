#src/common/networking
add_library(networking STATIC 
	"networking.cpp"
	"connection.cpp")

# OpenSSL
find_package(OpenSSL REQUIRED)

target_link_libraries(networking PUBLIC
		project_properties OpenSSL::SSL OpenSSL::Crypto robin_hood config)
target_compile_definitions(networking PUBLIC)

if(win32)
    target_compile_definitions(networking PUBLIC _WIN32_WINNT_WIN10=0x0A00 _WIN32_WINNT=0x0A00)
endif(win32)