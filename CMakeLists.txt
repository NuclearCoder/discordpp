cmake_minimum_required(VERSION 3.6)
project(discordpp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES discordpp/discordpp.hh src/discordpp.cc discordpp/bot.hh src/bot.cc discordpp/client.hh src/client.cc)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

add_subdirectory(lib/curlpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/curlpp/include/)

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib/beast/include)

find_package(OpenSSL)
find_package(Boost REQUIRED system)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} curlpp curl) #pthread ssl crypto
