WRAP_GGO(pctfillholl_GGO_C pctfillholl.ggo)
add_executable(pctfillholl pctfillholl.cxx ${pctfillholl_GGO_C})
target_link_libraries(pctfillholl PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctfillholl)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()