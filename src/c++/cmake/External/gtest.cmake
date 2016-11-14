# # GoogleTest (gtest)
# # A unit testing library for C/C++
# # Creates a libgtest target packaged with the required include driectories
# include(ExternalProject)
# 
# # Fetch GoogleTest remotely
# ExternalProject_Add(
#     gtest
#     URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
#     URL_MD5 2d6ec8ccdf5c46b05ba54a9fd1d130d7
#     PREFIX ${CMAKE_CURRENT_BINARY_DIR}
#     # Disable INSTALL
#     INSTALL_COMMAND ""
# )
# add_library(libgtest IMPORTED STATIC GLOBAL)
# add_dependencies(libgtest gtest)
# 
# # Setup the build tree and package the target
# ExternalProject_Get_Property(gtest SOURCE_DIR BINARY_DIR)
# file(MAKE_DIRECTORY ${SOURCE_DIR}/include)
# set_target_properties(libgtest PROPERTIES
#     "IMPORTED_LOCATION" "${BINARY_DIR}/libgtest.a"
#     "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#     "INTERFACE_INCLUDE_DIRECTORIES" "${SOURCE_DIR}/include"
# )

if (NOT __GTEST_INCLUDED) # guard against multiple includes
  set(__GTEST_INCLUDED TRUE)

  # use the system-wide gtest if present
  find_package(GTest)
  if (GTEST_FOUND)
    set(GTEST_EXTERNAL FALSE)
  else()
    message("did not find gtest: will download and build from source")
    # gtest will use pthreads if it's available in the system, so we must link with it
    find_package(Threads REQUIRED)

    # build directory
    set(gtest_PREFIX ${CMAKE_BINARY_DIR}/external/gtest-prefix)
    # install directory
    set(gtest_INSTALL ${CMAKE_BINARY_DIR}/external/gtest-install)

    # we build gtest statically, but want to link it into the shared library
    # this requires position-independent code
    if (UNIX)
        set(GTEST_EXTRA_COMPILER_FLAGS "-fPIC")
    endif()

    set(GTEST_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GTEST_EXTRA_COMPILER_FLAGS})
    set(GTEST_C_FLAGS ${CMAKE_C_FLAGS} ${GTEST_EXTRA_COMPILER_FLAGS})

    ExternalProject_Add(gtest
      PREFIX ${gtest_PREFIX}
      GIT_REPOSITORY "https://github.com/google/googletest.git"
      GIT_TAG "release-1.7.0"
      UPDATE_COMMAND ""
      INSTALL_COMMAND cmake -E echo "Skipping install step"
      INSTALL_DIR ${gtest_INSTALL}
      CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                 -DCMAKE_INSTALL_PREFIX=${gtest_INSTALL}
                 -DBUILD_SHARED_LIBS=OFF
                 -DBUILD_STATIC_LIBS=ON
                 -DBUILD_PACKAGING=OFF
                 -DBUILD_TESTING=OFF
                 -DBUILD_NC_TESTS=OFF
                 -BUILD_CONFIG_TESTS=OFF
                 -DINSTALL_HEADERS=ON
                 -DCMAKE_C_FLAGS=${GTEST_C_FLAGS}
                 -DCMAKE_CXX_FLAGS=${GTEST_CXX_FLAGS}
      LOG_DOWNLOAD 1
      LOG_INSTALL 1
      )

    set(GTEST_FOUND TRUE)

    ExternalProject_Get_Property(gtest binary_dir)
    ExternalProject_Get_Property(gtest source_dir)
    # add_library(gtest      UNKNOWN IMPORTED)
    # add_library(gtest_main UNKNOWN IMPORTED)
    # set_target_properties(gtest PROPERTIES
    #     IMPORTED_LOCATION ${binary_dir}/libgtest.a
    # )
    # set_target_properties(gtest_main PROPERTIES
    #     IMPORTED_LOCATION ${binary_dir}/libgtest_main.a
    # )
    # set(Gtest_LIBRARIES gtest gtest_main)
    set(GTEST_INCLUDE_DIRS "${source_dir}/include")
    set(GTEST_LIBRARIES ${binary_dir}/libgtest.a)
    set(GTEST_MAIN_LIBRARIES ${binary_dir}/libgtest_main.a)
    set(GTEST_LIBRARY_DIRS ${binary_dir})
    set(GTEST_EXTERNAL TRUE)

    list(APPEND external_project_dependencies gtest)
  endif()

endif()
