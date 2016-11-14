# # sudo apt install libgflags-dev
# find_package(Gflags QUIET)
# if (Gflags_FOUND)
#     # Local gflags setup has been found ('gflags' variable is set)
#     message("using local installation of gflags")
#     set(Gflags_LIBRARIES gflags)
# else()
#     # for some reason, on Ubuntu, cmake doesn't find local gfalgs
#     # (but does find it on Mac OS X)
#     message("did not find gflags: will download and build from source")
#     ExternalProject_Add (Gflags
#         GIT_REPOSITORY https://github.com/gflags/gflags.git
#         GIT_TAG v2.1.2
#         UPDATE_DISCONNECTED 1
#         CMAKE_ARGS
#             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext
#         )
#     ExternalProject_Get_Property(Gflags source_dir)
#     set(Gflags_INCLUDE_DIRS "${source_dir}/include")
#     set(Gflags_LIBRARIES
#         "${CMAKE_BINARY_DIR}/ext/lib/libgflags.a"
#         )
# endif()


if (NOT __GFLAGS_INCLUDED) # guard against multiple includes
  set(__GFLAGS_INCLUDED TRUE)

  # use the system-wide gflags if present
  find_package(GFlags)
  if (GFLAGS_FOUND)
    set(GFLAGS_EXTERNAL FALSE)
  else()
    message("did not find gflags: will download and build from source")
    # gflags will use pthreads if it's available in the system, so we must link with it
    find_package(Threads)

    # build directory
    set(gflags_PREFIX ${CMAKE_BINARY_DIR}/external/gflags-prefix)
    # install directory
    set(gflags_INSTALL ${CMAKE_BINARY_DIR}/external/gflags-install)

    # we build gflags statically, but want to link it into the shared library
    # this requires position-independent code
    if (UNIX)
        set(GFLAGS_EXTRA_COMPILER_FLAGS "-fPIC")
    endif()

    set(GFLAGS_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GFLAGS_EXTRA_COMPILER_FLAGS})
    set(GFLAGS_C_FLAGS ${CMAKE_C_FLAGS} ${GFLAGS_EXTRA_COMPILER_FLAGS})

    ExternalProject_Add(gflags
      PREFIX ${gflags_PREFIX}
      GIT_REPOSITORY "https://github.com/gflags/gflags.git"
      GIT_TAG "v2.1.2"
      UPDATE_COMMAND ""
      INSTALL_DIR ${gflags_INSTALL}
      CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                 -DCMAKE_INSTALL_PREFIX=${gflags_INSTALL}
                 -DBUILD_SHARED_LIBS=OFF
                 -DBUILD_STATIC_LIBS=ON
                 -DBUILD_PACKAGING=OFF
                 -DBUILD_TESTING=OFF
                 -DBUILD_NC_TESTS=OFF
                 -BUILD_CONFIG_TESTS=OFF
                 -DINSTALL_HEADERS=ON
                 -DCMAKE_C_FLAGS=${GFLAGS_C_FLAGS}
                 -DCMAKE_CXX_FLAGS=${GFLAGS_CXX_FLAGS}
      LOG_DOWNLOAD 1
      LOG_INSTALL 1
      )

    set(GFLAGS_FOUND TRUE)
    set(GFLAGS_INCLUDE_DIRS ${gflags_INSTALL}/include)
    set(GFLAGS_LIBRARIES ${gflags_INSTALL}/lib/libgflags.a ${CMAKE_THREAD_LIBS_INIT})
    set(GFLAGS_LIBRARY_DIRS ${gflags_INSTALL}/lib)
    set(GFLAGS_EXTERNAL TRUE)

    list(APPEND external_project_dependencies gflags)
  endif()

endif()
