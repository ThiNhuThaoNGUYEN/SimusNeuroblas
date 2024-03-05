# ============================================================================
# Library version number
# ============================================================================
set(lib_version_major 1)
set(lib_version_minor 0)

# ============================================================================
# Tell cmake about subdirectories to look into
# ============================================================================


# ============================================================================
# Find packages
# ============================================================================
find_package(ZLIB REQUIRED)

# ============================================================================
# Add the 'generated_headers' target (uses a custom script)
# ============================================================================
# custom commands to generate header files
add_custom_command(
  OUTPUT CellType.h
  COMMAND ${CMAKE_COMMAND}
      -P ${PROJECT_SOURCE_DIR}/cmake/cmake_generate_header.cmake
      --  ${PROJECT_SOURCE_DIR}/src/simuscale-core
          CellType
  DEPENDS CellType.h.in CellType.values.in)
add_custom_command(
  OUTPUT InterCellSignal.h
  COMMAND ${CMAKE_COMMAND}
      -P ${PROJECT_SOURCE_DIR}/cmake/cmake_generate_header.cmake
      --  ${PROJECT_SOURCE_DIR}/src/simuscale-core
          InterCellSignal
  DEPENDS InterCellSignal.h.in InterCellSignal.values.in)

# add target params_header (this will execute the custom command)
add_custom_target(
  generated_headers
  DEPENDS ${CMAKE_BINARY_DIR}/src/simuscale-core/InterCellSignal.h
  DEPENDS ${CMAKE_BINARY_DIR}/src/simuscale-core/CellType.h)

# ============================================================================
# Add library simuscale-core
# ============================================================================
add_library(simuscale-core SHARED
  Alea.h Alea.cpp
  Cell.h Cell.cpp
  Coordinates.h Coordinates.hpp
  Grid.h Grid.cpp
  Observable.h Observable.cpp
  Observer.h ObservableEvent.h
  OutputManager.h OutputManager.cpp
  Population.h Population.cpp
  Simulation.h Simulation.cpp
  params/ParamFileReader.h params/ParamFileReader.cpp
  params/SimulationParams.h params/SimulationParams.cpp
  params/PopulationParams.h
  params/CellParams.h
  params/NicheParams.h
  InterCellSignals.cpp InterCellSignals.h
  CellSize.cpp CellSize.h
  WorldSize.h WorldSize.cpp
  CellTree.h CellTree.cpp
  movement/MoveBehaviour.h movement/MoveBehaviour.cpp
  movement/Immobile.h movement/Immobile.cpp
  movement/Mobile.h movement/Mobile.cpp
  movement/Motile.h movement/Motile.cpp)
# add dependency for generated header files
add_dependencies(simuscale-core generated_headers)
# We use C++11
target_compile_options(simuscale-core PRIVATE "-std=c++11")
# Make STDC MACROS available (for fixed width integers)
target_compile_definitions(simuscale-core PUBLIC __STDC_FORMAT_MACROS
                                        PUBLIC __STDC_CONSTANT_MACROS)
target_include_directories(
  simuscale-core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_BINARY_DIR}/src/simuscale-core/ # for generated headers
)
# We use the GSL and zlib libraries
# target_link_libraries(simuscale-core ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES})
target_link_libraries(simuscale-core GSL::gsl GSL::gslcblas)
target_link_libraries(simuscale-core ${ZLIB_LIBRARY})


# ============================================================================
# Set behaviour on make install
# ============================================================================
# Technically, the soversion needs not match the major version of the lib.
# We choose to enforce this match for simplicity
set(lib_soversion ${lib_version_major})
set_target_properties(simuscale-core PROPERTIES
        VERSION ${lib_version_major}.${lib_version_minor}
        SOVERSION ${lib_soversion})
install(TARGETS simuscale-core LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(CODE "message(\"-- Running ldconfig\")")
install(CODE "execute_process(COMMAND ldconfig)")
