if (NOT __BOOST_INCLUDED) # guard against multiple includes
  set(__BOOST_INCLUDED TRUE)

    find_package(Boost 1.58.0 REQUIRED COMPONENTS system random filesystem REQUIRED)

    # Always use Boost's shared libraries.
    set(Boost_USE_STATIC_LIBS OFF)

    # We need this for all tests to use the dynamic version.
    add_definitions(-DBOOST_TEST_DYN_LINK)

    # Always use multi-threaded Boost libraries.
    set(Boost_USE_MULTI_THREADED ON)

    if (MSVC)
      add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    endif(MSVC)
    if (WIN32)
      add_definitions(-D_WIN32_WINNT=0x0501)
    endif(WIN32)
    # include_directories(${Boost_INCLUDE_DIRS})
endif()
