cmake_minimum_required(VERSION 3.10)
project(Binge)
file(GLOB BINGE_SRCS "src/*.cpp")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(clog REQUIRED)
find_package(EventManager REQUIRED)
find_package(Protobuf 3.7.0 REQUIRED)
find_package(FFMPEG MODULE COMPONENTS AVFORMAT AVUTIL SWSCALE AVCODEC REQUIRED)
find_package(OpenCV REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/build/.thirdparty)

set(CMAKE_CXX_STANDARD 11)
add_library(binge ${BINGE_SRCS})
target_include_directories(binge PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(binge PRIVATE Threads::Threads)
target_link_libraries(binge PRIVATE clog::clog)
target_link_libraries(binge PRIVATE EventManager)
target_link_libraries(binge PRIVATE ${OpenCV_LIBS})
target_include_directories(binge PUBLIC ${OpenCV_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DBINGE_LOG_ENABLED)
  # remove after using updated ffmpeg api
  target_compile_options(binge PUBLIC "-Wno-deprecated-declarations")
endif()

# ffmpeg required components
target_include_directories(binge PUBLIC ${AVFORMAT_INCLUDE_DIRS})
target_link_libraries(binge PUBLIC ${AVFORMAT_LIBRARIES})

target_include_directories(binge PUBLIC ${AVUTIL_INCLUDE_DIRS})
target_link_libraries(binge PUBLIC ${AVUTIL_LIBRARIES})

target_include_directories(binge PUBLIC ${SWSCALE_INCLUDE_DIRS})
target_link_libraries(binge PUBLIC ${SWSCALE_LIBRARIES})

target_include_directories(binge PUBLIC ${AVCODEC_INCLUDE_DIRS})
target_link_libraries(binge PUBLIC ${AVCODEC_LIBRARIES})

# protobuf
target_link_libraries(binge PUBLIC protobuf::libprotobuf)

enable_testing()
add_subdirectory(test)