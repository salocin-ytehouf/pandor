cmake_minimum_required(VERSION 3.0)
project(hw_int_vid)

# Find OpenCV package
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find JsonCpp package
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})

# Find JsonCpp package
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Add the "base64-c" library as a subdirectory
add_subdirectory(./base64)

# Add the following line to link the Pthreads library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Add executable
add_executable(hw_int_vid hw_int_vid.cpp)

# Link libraries
target_link_libraries(hw_int_vid ${OpenCV_LIBS} ${JSONCPP_LIBRARIES} sqlite3 ${CURL_LIBRARIES} base64)

target_include_directories(hw_int_vid PRIVATE ${CMAKE_SOURCE_DIR}/json/include)


