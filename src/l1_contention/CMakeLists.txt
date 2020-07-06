add_library(l1_contention STATIC l1_contention.c)
target_link_libraries(l1_contention util debug_log low ${CMAKE_THREAD_LIBS_INIT})