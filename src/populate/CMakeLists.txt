cmake_minimum_required(VERSION 3.5)


# ============================================================================
# Find packages
# ============================================================================
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


#  # It looks like there's a bug in CMake support for GLUT when libXi or libXmu are
#  # not present on the system. See
#  # http://public.kitware.com/Bug/view.php?id=14060 for more info and workaround
#  #
#  # Workaround (from the aforementioned webpage):
#  set(_glut_libraries)
#  foreach(_lib ${GLUT_LIBRARIES})
#    if(_lib)
#      list(APPEND _glut_libraries ${_lib})
#    endif()
#  endforeach()
#  set(GLUT_LIBRARIES ${_glut_libraries})



# ============================================================================
# Add target populate add its dependencies
# ============================================================================
add_executable(populate populate.cpp)
# We use C++11
target_compile_options(populate PRIVATE "-std=c++11")

# ============================================================================
# Use OpenGL and GLUT
# ============================================================================
target_link_libraries(populate ${OPENGL_LIBRARIES} GLUT::GLUT )


# ============================================================================
# Set behaviour on make install
# ============================================================================
#install(TARGETS populate DESTINATION ${CMAKE_INSTALL_BINDIR}/simuscale)
