project (Tutorial)
add_subdirectory (MathFunctions)
add_executable(Tutorial tutorial.cpp)
target_link_libraries (Tutorial MathFunctions)

# add the install targets
install (TARGETS Tutorial DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
