
# add a target to generate API documentation with Doxygen
# after https://tty1.net/blog/2014/cmake-doxygen_en.html

if (NOT __DOXYGEN_INCLUDED) # guard against multiple includes
    set(__DOXYGEN_INCLUDED TRUE)

    find_package(Doxygen)
    option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
    if(BUILD_DOCUMENTATION)
        if(NOT DOXYGEN_FOUND)
            message(FATAL_ERROR "Doxygen is needed to build the documentation.")
        endif()
        set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${doxyfile_in} ${doxyfile} @ONLY)
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
    endif()

endif()

