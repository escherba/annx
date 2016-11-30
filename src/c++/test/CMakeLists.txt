enable_testing()
add_custom_target(check ${CMAKE_CTEST_COMMAND} -V)

set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}-test)

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cc)
file(GLOB PROJ_SRC_FILES ${PROJECT_SOURCE_DIR}/src/ann/*.cc ${PROJECT_SOURCE_DIR}/src/common/*.cc)

add_executable(${PROJECT_TEST_NAME} ${PROJ_SRC_FILES} ${TEST_SRC_FILES})
target_compile_features(${PROJECT_TEST_NAME} PRIVATE cxx_range_for)
target_link_libraries(${PROJECT_TEST_NAME} ${${PROJECT_NAME_STR}_LINKER_LIBS})
target_link_libraries(${PROJECT_TEST_NAME} ${${PROJECT_NAME_STR}_TEST_LINKER_LIBS})
target_link_libraries(${PROJECT_TEST_NAME} ${CMAKE_THREAD_LIBS_INIT})
add_test(test ${PROJECT_TEST_NAME})
