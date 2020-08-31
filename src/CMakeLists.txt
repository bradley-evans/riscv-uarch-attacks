add_subdirectory(l1_contention)
add_subdirectory(l1pp)

add_library(low STATIC low.c)
add_library(util STATIC util.c)
add_library(addr STATIC addr.c)
add_library(debug_log STATIC debug_log.c)
target_link_libraries(low m util debug_log)
target_link_libraries(addr low m debug_log)

set(EXECUTABLE_OUTPUT_PATH "../bin")

add_executable(demo demo.c)
target_link_libraries(demo util low addr debug_log l1_contention l1pp -static -lpthread)

add_executable(l1pp_datacollect l1pp_datacollect.c)
target_link_libraries(l1pp_datacollect low l1pp -static pthread m)