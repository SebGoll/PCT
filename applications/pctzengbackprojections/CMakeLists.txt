WRAP_GGO(pctzengbackprojections_GGO_C pctzengbackprojections.ggo)
add_executable(pctzengbackprojections pctzengbackprojections.cxx ${pctzengbackprojections_GGO_C})
target_link_libraries(pctzengbackprojections PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctzengbackprojections)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()