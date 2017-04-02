cmake_minimum_required (VERSION 2.6)
project (avalanche)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(avalanche ${SOURCES})
target_link_libraries( avalanche serial ${OpenCV_LIBS} )

