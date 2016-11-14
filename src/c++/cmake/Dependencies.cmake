set(${PROJECT_NAME_STR}_LINKER_LIBS "")
set(${PROJECT_NAME_STR}_TEST_LINKER_LIBS "")

# --[ Threads
# find_package(Threads REQUIRED)

# ---[ Google-gflags
include("cmake/External/gflags.cmake")
include_directories(SYSTEM ${GFLAGS_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME_STR}_LINKER_LIBS ${GFLAGS_LIBRARIES})

# ---[ Google-gtest
include("cmake/External/gtest.cmake")
include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME_STR}_TEST_LINKER_LIBS ${GTEST_LIBRARIES})
list(APPEND ${PROJECT_NAME_STR}_TEST_LINKER_LIBS ${GTEST_MAIN_LIBRARIES})

# ---[ Eigen3
include("cmake/External/eigen3.cmake")
include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
# list(APPEND ${PROJECT_NAME_STR}_LINKER_LIBS ${EIGEN3_LIBRARIES})

# ---[ Doxygen
include("cmake/External/doxygen.cmake")

# ---[ Boost
include("cmake/External/boost.cmake")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME_STR}_LINKER_LIBS ${Boost_LIBRARIES})
