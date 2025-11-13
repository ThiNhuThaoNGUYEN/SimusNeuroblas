cmake_minimum_required(VERSION 3.5)

# ============================================================================
# Add target lineagetree add its dependencies
# ============================================================================
add_executable(lineagetree lineagetree.cpp)
# We use C++11
target_compile_options(lineagetree PRIVATE "-std=c++11")

# ============================================================================
# Set behaviour on make install
# ============================================================================
#install(TARGETS lineagetree DESTINATION ${CMAKE_INSTALL_BINDIR}/simuscale)
