add_library(low STATIC low.c)
add_library(util STATIC util.c)
add_library(addr STATIC addr.c)
target_link_libraries(low util)
target_link_libraries(addr low)
target_link_libraries(addr m)

add_executable(demo demo.c)
target_link_libraries(demo util low addr -static)
