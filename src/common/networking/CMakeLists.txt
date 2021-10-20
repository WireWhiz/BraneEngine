#src/common/networking

add_library(networking STATIC 
	"networking.cpp"
	"connection.cpp"
   "networkError.cpp"
   "client.cpp" 
   "server.cpp"
    "serverAddress.h" "serverAddress.cpp")
 target_link_libraries(networking PUBLIC ssl)