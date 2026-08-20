add_library(lesson_02_wait_status
  examples/wait_status.cpp
)

target_include_directories(lesson_02_wait_status
  PUBLIC
    examples
)

add_executable(process_lifecycle
  examples/process_lifecycle.cpp
)

target_link_libraries(process_lifecycle
  PRIVATE
    lesson_02_wait_status
)

if(BUILD_TESTING)
  add_executable(lesson_02_wait_status_test
    tests/wait_status_test.cpp
  )

  add_executable(lesson_02_run_by_name_test
    tests/run_by_name_test.cpp
  )

  target_link_libraries(lesson_02_wait_status_test
    PRIVATE
      lesson_02_wait_status
  )

  add_test(
    NAME lesson_02_wait_status
    COMMAND lesson_02_wait_status_test
  )

  add_test(
    NAME lesson_02_process_lifecycle
    COMMAND process_lifecycle
  )

  add_test(
    NAME lesson_02_process_lifecycle_via_path
    COMMAND lesson_02_run_by_name_test
            $<TARGET_FILE_DIR:process_lifecycle>
  )
  set_tests_properties(
    lesson_02_process_lifecycle_via_path
    PROPERTIES
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
endif()
