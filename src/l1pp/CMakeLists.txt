add_library(l1pp STATIC l1pp.c)
target_link_libraries(l1pp util debug_log addr low ${CMAKE_THREAD_LIBS_INIT})