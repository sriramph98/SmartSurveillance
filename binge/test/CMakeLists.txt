cmake_minimum_required(VERSION 3.10)
project(Binge)

add_executable(binge_test parse_video_frames.cpp)

target_link_libraries(binge_test PRIVATE binge)
target_include_directories(binge_test PRIVATE binge)