find_package(Gengetopt REQUIRED)

add_subdirectory(pctpairgeometry)
add_subdirectory(pctmostlikelypath)
add_subdirectory(pctbinning)
add_subdirectory(pctfillholl)
add_subdirectory(pctbackprojectionbinning)


add_subdirectory(pctbackprojections)
add_subdirectory(pctfdktwodweights)
add_subdirectory(pctfdk)


if(BUILD_TESTING)
endif()