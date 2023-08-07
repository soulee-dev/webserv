cmake_minimum_required(VERSION 3.25)
project(soulee_git)

set(CMAKE_CXX_STANDARD 14)

include_directories(srcs)

add_executable(soulee_git
        srcs/Client.cpp
        srcs/Client.hpp
        srcs/ClientManager.cpp
        srcs/ClientManager.hpp
        srcs/ConfigFunctions.cpp
        srcs/ConfigParser.cpp
        srcs/ConfigParser.hpp
        srcs/Event.cpp
        srcs/Event.hpp
        srcs/Location.cpp
        srcs/Location.hpp
        srcs/main.cpp
        srcs/Message.cpp
        srcs/Message.hpp
        srcs/RequestMessage.cpp
        srcs/RequestMessage.hpp
        srcs/ResponseMessage.cpp
        srcs/ResponseMessage.hpp
        srcs/Server.cpp
        srcs/Server.hpp
        srcs/ServerManager.cpp
        srcs/ServerManager.hpp srcs/Http/HttpRequestManager.cpp)
