WRAP_GGO(pctswapcoordinates_GGO_C pctswapcoordinates.ggo)
add_executable(pctswapcoordinates pctswapcoordinates.cxx ${pctswapcoordinates_GGO_C})
target_link_libraries(pctswapcoordinates PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctswapcoordinates)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()