WRAP_GGO(pctfdktwodweights_GGO_C pctfdktwodweights.ggo)
add_executable(pctfdktwodweights pctfdktwodweights.cxx ${pctfdktwodweights_GGO_C})
target_link_libraries(pctfdktwodweights PCT)

# Installation code
# if(NOT RTK_INSTALL_NO_EXECUTABLES)
foreach(EXE_NAME pctfdktwodweights)
    install(TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${PCT_INSTALL_RUNTIME_DIR} COMPONENT Runtime
        LIBRARY DESTINATION ${PCT_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${PCT_INSTALL_ARCHIVE_DIR} COMPONENT Development)
endforeach()
# endif()