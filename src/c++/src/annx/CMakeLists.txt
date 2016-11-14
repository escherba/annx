include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB COMMON_SRC_FILES ${PROJECT_SOURCE_DIR}/src/common/*.cc)
add_executable(annx ${COMMON_SRC_FILES} space.cc main.cc)
target_compile_features(annx PRIVATE cxx_range_for)
target_link_libraries(annx ${${PROJECT_NAME_STR}_LINKER_LIBS})
target_link_libraries(annx ${CMAKE_THREAD_LIBS_INIT})
