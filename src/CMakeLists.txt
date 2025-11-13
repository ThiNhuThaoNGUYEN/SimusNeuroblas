# ============================================================================
# Generate config.h
# ============================================================================
configure_file(
        ${PROJECT_SOURCE_DIR}/src/config.h.in
        ${PROJECT_BINARY_DIR}/src/config.h
)

# ============================================================================
# User-defined variables
# ============================================================================
set(PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
set(PLUG "" CACHE STRING "names of plugins to use") 
set(SIM_DIR "" CACHE PATH "path to param.in file") 

# ============================================================================
# Tell cmake about subdirectories to look into
# ============================================================================
#add_subdirectory(view)
#add_subdirectory(populate)
#add_subdirectory(lineagetree)
add_subdirectory(simuscale-core)

# ============================================================================
# List all available plugins
# ============================================================================

message(CHECK_START "Looking for plugins")
# set all available plugins from base repository 
set(PLUGINS Cancer CACHE INTERNAL "list of plugins")

# Add plugins manually from command line: cmake -DPLUG='Plugin1;Plugin2...'
list(APPEND PLUGINS ${PLUG} )

# Add plugins listed in param.in file 

# message(STATUS "SIM_DIR: ${SIM_DIR}")
if(EXISTS ${SIM_DIR}/param.in)
  # message(STATUS "param.in: ${SIM_DIR}/param.in")
  file(STRINGS "${SIM_DIR}/param.in" POPULATION_DECLARATIONS REGEX "^ADD_POPULATION.*")

  # check files in PLUGIN_DIR
  file(GLOB PLUGIN_FILE_LIST LIST_DIRECTORIES false ${PLUGIN_DIR}/*.h)

  # check files in SIM_DIR
  file(GLOB PLUGIN_FILE_LIST LIST_DIRECTORIES false ${SIM_DIR}/*.h)

  foreach (pfile IN LISTS PLUGIN_FILE_LIST)
    foreach(pop IN LISTS POPULATION_DECLARATIONS)
      string(REGEX MATCHALL "[a-zA-Z0-9_]+\ |[a-zA-Z0-9_]+$" POPULATION_ARGS "${pop}")
      list(GET POPULATION_ARGS 3 NEW_PLUGIN_NAME)
      string(STRIP "${NEW_PLUGIN_NAME}" NEW_PLUGIN_NAME)
      file(STRINGS ${pfile} TMPTXT REGEX "[^a-zA-Z]${NEW_PLUGIN_NAME}[^a-zA-Z0-9_]")
      string(FIND "${TMPTXT}" "${NEW_PLUGIN_NAME}" matchres)
      if(${matchres} GREATER 0)
        cmake_path(GET pfile STEM stem )
        list(APPEND PLUGINS ${stem} )
        # message(STATUS "Including ${stem}")
      endif()
    endforeach()
  endforeach()
endif()

# Add plugins from Plugins.in
if(EXISTS ${PLUGIN_DIR}/Plugins.in)
  file(STRINGS "${PLUGIN_DIR}/Plugins.in" NEW_PLUGIN_NAMES REGEX "^[^#]")
  foreach(NEW_PLUGIN_NAME IN LISTS NEW_PLUGIN_NAMES)
    string(STRIP "${NEW_PLUGIN_NAME}" NEW_PLUGIN_NAME)
    list(APPEND PLUGINS ${NEW_PLUGIN_NAME} )
  endforeach()
endif()

foreach (PLUGIN IN LISTS PLUGINS)
  # if the plugin exists in the simulation directory, use it, else, use the plugin in src/plugins 
  if(EXISTS ${SIM_DIR}/${PLUGIN}.h AND EXISTS ${SIM_DIR}/${PLUGIN}.cpp)
    set(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${SIM_DIR}/${PLUGIN}.h ${SIM_DIR}/${PLUGIN}.cpp)
    message(STATUS "${SIM_DIR}/${PLUGIN}")
  else()
    set(PLUGIN_SOURCES ${PLUGIN_SOURCES} ${PLUGIN_DIR}/${PLUGIN}.h ${PLUGIN_DIR}/${PLUGIN}.cpp)
    message(STATUS "${PLUGIN_DIR}/${PLUGIN}")
  endif()
endforeach(PLUGIN)

if(PLUGIN_SOURCES)
  message(CHECK_PASS "Plugins found")
else()
  message(CHECK_FAIL "No plugin found")
endif()

message(NOTICE "Re-execute cmake to add or change plugins.")

# ============================================================================
# Add target simuscale add its dependencies
# ============================================================================
add_executable(simuscale main.cpp ${PLUGIN_SOURCES})
target_link_libraries(simuscale simuscale-core GSL::gsl GSL::gslcblas)
# We use C++11
target_compile_options(simuscale PRIVATE "-std=c++11")
# Make STDC MACROS available (for fixed width integers)
target_compile_definitions(simuscale PUBLIC __STDC_FORMAT_MACROS
                                     PUBLIC __STDC_CONSTANT_MACROS)

target_include_directories(
  simuscale
  PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE "${PROJECT_BINARY_DIR}/src" # for config.h
)


# ============================================================================
# Set behaviour on make install
# ============================================================================
install(TARGETS simuscale RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


# ============================================================================
# Include tests
# ============================================================================
add_subdirectory(tests EXCLUDE_FROM_ALL)
