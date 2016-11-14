# # sudo apt install libeigen3-dev
# find_package(eigen3 QUIET)
# if (eigen3_FOUND)
#     # Local eigen3 setup has been found ('eigen3' variable is set)
#     message("using local installation of eigen3")
#     set(eigen3_LIBRARIES eigen3)
# else()
#     # for some reason, on Ubuntu, cmake doesn't find local gfalgs
#     # (but does find it on Mac OS X)
#     message("did not find eigen3: will download and build from source")
#     ExternalProject_Add (eigen3
#         GIT_REPOSITORY https://github.com/eigen3/eigen3.git
#         GIT_TAG v2.1.2
#         UPDATE_DISCONNECTED 1
#         CMAKE_ARGS
#             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext
#         )
#     ExternalProject_Get_Property(eigen3 source_dir)
#     set(eigen3_INCLUDE_DIRS "${source_dir}/include")
#     set(eigen3_LIBRARIES
#         "${CMAKE_BINARY_DIR}/ext/lib/libeigen3.a"
#         )
# endif()


if (NOT __EIGEN3_INCLUDED) # guard against multiple includes
  set(__EIGEN3_INCLUDED TRUE)

  # use the system-wide eigen3 if present
  find_package(Eigen3)
  if (EIGEN3_FOUND)
    set(EIGEN3_EXTERNAL FALSE)
  else()
    message("did not find eigen3: will download headers")
    # eigen3 will use pthreads if it's available in the system, so we must link with it
    find_package(Hg REQUIRED)
    if(HG_FOUND)
      message("hg found: ${HG_EXECUTABLE}")
    endif()

    # build directory
    set(eigen3_PREFIX ${CMAKE_BINARY_DIR}/external/eigen3-prefix)
    # install directory
    set(eigen3_INSTALL ${CMAKE_BINARY_DIR}/external/eigen3-install)

    # we build eigen3 statically, but want to link it into the shared library
    # this requires position-independent code
    if (UNIX)
        set(EIGEN3_EXTRA_COMPILER_FLAGS "-fPIC")
    endif()

    set(EIGEN3_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${EIGEN3_EXTRA_COMPILER_FLAGS})
    set(EIGEN3_C_FLAGS ${CMAKE_C_FLAGS} ${EIGEN3_EXTRA_COMPILER_FLAGS})

    ExternalProject_Add(eigen3
      PREFIX ${eigen3_PREFIX}
      HG_REPOSITORY "https://github.com/eigen3/eigen3.git"
      HG_TAG "3.2.10"
      UPDATE_COMMAND ""
      INSTALL_COMMAND cmake -E echo "Skipping install step"
      INSTALL_DIR ${eigen3_INSTALL}
      CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                 -DCMAKE_INSTALL_PREFIX=${eigen3_INSTALL}
                 -DBUILD_SHARED_LIBS=OFF
                 -DBUILD_STATIC_LIBS=ON
                 -DBUILD_PACKAGING=OFF
                 -DBUILD_TESTING=OFF
                 -DBUILD_NC_TESTS=OFF
                 -BUILD_CONFIG_TESTS=OFF
                 -DINSTALL_HEADERS=ON
                 -DCMAKE_C_FLAGS=${EIGEN3_C_FLAGS}
                 -DCMAKE_CXX_FLAGS=${EIGEN3_CXX_FLAGS}
      LOG_DOWNLOAD 1
      LOG_INSTALL 1
      )

    set(EIGEN3_FOUND TRUE)
    set(EIGEN3_INCLUDE_DIRS ${eigen3_INSTALL}/include)
    set(EIGEN3_LIBRARIES ${eigen3_INSTALL}/lib/libeigen3.a ${CMAKE_THREAD_LIBS_INIT})
    set(EIGEN3_LIBRARY_DIRS ${eigen3_INSTALL}/lib)
    set(EIGEN3_EXTERNAL TRUE)

    list(APPEND external_project_dependencies eigen3)
  endif()

endif()
