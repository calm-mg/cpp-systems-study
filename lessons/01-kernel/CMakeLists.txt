add_library(lesson_01_process_info
  examples/process_info.cpp
)

target_include_directories(lesson_01_process_info
  PUBLIC
    examples
)

add_executable(kernel_boundary
  examples/kernel_boundary.cpp
)

target_link_libraries(kernel_boundary
  PRIVATE
    lesson_01_process_info
)

if(BUILD_TESTING)
  add_executable(lesson_01_process_info_test
    tests/process_info_test.cpp
  )

  target_link_libraries(lesson_01_process_info_test
    PRIVATE
      lesson_01_process_info
  )

  add_test(
    NAME lesson_01_process_info
    COMMAND lesson_01_process_info_test
  )
endif()
