cmake_minimum_required(VERSION 3.10)

project(ndkaudiotest)

add_library(api_sles SHARED sles/api.c locallib/asset.c)
add_library(api_aaudio SHARED aaudio/api.c locallib/asset.c)

target_link_libraries(api_sles   PRIVATE android log OpenSLES)
target_link_libraries(api_aaudio PRIVATE android log aaudio  )

target_include_directories(api_sles   PRIVATE "include")
target_include_directories(api_aaudio PRIVATE "include")
