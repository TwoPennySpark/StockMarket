cmake_minimum_required(VERSION 3.0)
project(StockMarketClient)

# Default to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(
  src/include
  ../../Common
  ../../Common/NetCommon
)

set(SOURCES src/client.cpp ../../Common/ms_types.cpp)

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} pthread ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../../../install)
