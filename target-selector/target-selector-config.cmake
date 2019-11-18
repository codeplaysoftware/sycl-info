include(CMakeFindDependencyMacro)
find_dependency(OpenCL)

include("${CMAKE_CURRENT_LIST_DIR}/target-selector-targets.cmake")
