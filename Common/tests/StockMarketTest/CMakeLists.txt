cmake_minimum_required(VERSION 3.0)
project(StockMarketTest)

# Default to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(
  ../../
  ../../NetCommon
  ../../../Client/StockMarketClient/src/include
  ../../../Server/StockMarketServer/src/include
)

set(SOURCES ../../ms_types.cpp ../../../Client/StockMarketClient/src/client.cpp
	../../../Server/StockMarketServer/src/core.cpp ../../../Server/StockMarketServer/src/server.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} pthread ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../../../../install)
