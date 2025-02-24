
if (NOT TARGET gengetopt)
    find_program(GENGETOPT gengetopt "${ITK_RUNTIME_LIBRARY_DIRS}")
    if ((GENGETOPT STREQUAL "GENGETOPT-NOTFOUND") OR (GENGETOPT STREQUAL ""))
        message("GENGETOPT not found at ${ITK_RUNTIME_LIBRARY_DIRS}")
    else()
        add_executable(gengetopt IMPORTED)
        set_property(TARGET gengetopt PROPERTY IMPORTED_LOCATION ${GENGETOPT})
    endif()
endif()

function(CAT_GGO_FILES INPUTS OUTPUT)
    file(WRITE "${OUTPUT}" "")
    foreach(INPUT ${INPUTS})
        string(REPLACE ";" " " INPUT ${INPUT})
        file(READ "${INPUT}" CONTENT)
        file(APPEND "${OUTPUT}" "${CONTENT}")
    endforeach()
endfunction()


macro (WRAP_GGO GGO_SRCS)
    # Set current list of files to zero for a new target
    set(GGO_FILES_ABS "")

    # Convert list of a file in a list with absolute file names
    foreach(GGO_FILE ${ARGN})
        get_filename_component(GGO_FILE_ABS ${GGO_FILE} ABSOLUTE)
        list(APPEND GGO_FILES_ABS "${GGO_FILE_ABS}")
    endforeach()

    # Append to a new ggo file containing all files
    list(GET GGO_FILES_ABS 0 FIRST_GGO_FILE)
    get_filename_component(FIRST_GGO_BASEFILENAME ${FIRST_GGO_FILE} NAME)
    separate_arguments(GGO_FILES_ABS_LIST NATIVE_COMMAND "${GGO_FILES_ABS}")
    CAT_GGO_FILES(${GGO_FILES_ABS_LIST} "${CMAKE_CURRENT_BINARY_DIR}/${FIRST_GGO_BASEFILENAME}")
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${FIRST_GGO_BASEFILENAME} PROPERTIES GENERATED TRUE)

    # Now add ggo command
    get_filename_component(GGO_BASEFILENAME ${FIRST_GGO_FILE} NAME_WE)
    set(GGO_H ${GGO_BASEFILENAME}_ggo.h)
    set(GGO_C ${GGO_BASEFILENAME}_ggo.c)
    set(GGO_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GGO_H} ${CMAKE_CURRENT_BINARY_DIR}/${GGO_C})


    add_custom_command(OUTPUT ${GGO_OUTPUT}
                     COMMAND gengetopt
                     ARGS < ${CMAKE_CURRENT_BINARY_DIR}/${FIRST_GGO_BASEFILENAME}
                            --output-dir=${CMAKE_CURRENT_BINARY_DIR}
                            --arg-struct-name=args_info_${GGO_BASEFILENAME}
                            --func-name=cmdline_parser_${GGO_BASEFILENAME}
                            --file-name=${GGO_BASEFILENAME}_ggo
                            --unamed-opts
                            --conf-parser
                            --include-getopt
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FIRST_GGO_BASEFILENAME}
                    )
    set(${GGO_SRCS} ${${GGO_SRCS}} ${GGO_OUTPUT})
    include_directories("${CMAKE_CURRENT_BINARY_DIR}")

    set_source_files_properties(${${GGO_SRCS}} PROPERTIES GENERATED TRUE)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set_source_files_properties(${${GGO_SRCS}} PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable")
    endif()
    if(MSVC)
        # Disable double to float truncation warning as gengetopt cannot append "f"
        # to force default numeric float values in the .ggo config file
        set_source_files_properties(${${GGO_SRCS}} PROPERTIES COMPILE_FLAGS "/wd4305")
    endif()
    
endmacro()