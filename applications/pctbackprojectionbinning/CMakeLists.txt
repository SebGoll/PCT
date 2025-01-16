WRAP_GGO(pctbackprojectionbinning_GGO_C pctbackprojectionbinning.ggo)
add_executable(pctbackprojectionbinning pctbackprojectionbinning.cxx ${pctbackprojectionbinning_GGO_C})
target_link_libraries(pctbackprojectionbinning PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctbackprojectionbinning)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()