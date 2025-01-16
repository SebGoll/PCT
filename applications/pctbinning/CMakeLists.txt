WRAP_GGO(pctbinning_GGO_C pctbinning.ggo)
add_executable(pctbinning pctbinning.cxx ${pctbinning_GGO_C})
target_link_libraries(pctbinning PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctbinning)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()