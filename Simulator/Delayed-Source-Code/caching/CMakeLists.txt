cmake_minimum_required(VERSION 3.10)
project(Caching)

# Library
link_libraries(hashing)

# Sources
include_directories(include)
##### Offline Policies #####
add_executable(cache_belady src/cache_belady.cpp)
add_executable(cache_aggdelay src/cache_aggdelay.cpp)

##### Online Policies #####
add_executable(cache_lru src/cache_lru.cpp)
add_executable(cache_lruk src/cache_lruk.cpp)
add_executable(cache_lfu src/cache_lfu.cpp)
add_executable(cache_2q src/cache_2q.cpp)
add_executable(cache_la src/cache_la.cpp)
add_executable(cache_lhd src/cache_lhd.cpp)
add_executable(cache_lru_aggdelay src/cache_lru_aggdelay.cpp)
add_executable(cache_lhd_aggdelay src/cache_lhd_aggdelay.cpp)
