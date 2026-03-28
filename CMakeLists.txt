cmake_minimum_required(VERSION 3.16)
project(SchackMedMotor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SFML package (try different approaches)
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
if(NOT SFML_FOUND)
    # Fallback: assume SFML is in standard locations
    message("SFML not found with find_package, using fallback paths")
endif()

# Create executable
add_executable(schack_med_motor schack_med_bilder.cpp)

# Link SFML libraries
if(SFML_FOUND)
    target_link_libraries(schack_med_motor
        sfml-graphics
        sfml-window
        sfml-system
    )
else()
    # Fallback linking
    target_link_libraries(schack_med_motor
        sfml-graphics
        sfml-window
        sfml-system
    )
endif()

# Include directories (try multiple locations)
target_include_directories(schack_med_motor PRIVATE
    "C:/SFML/include"
    "C:/Program Files/SFML/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SFML/include"
    # Add other possible paths
)

# Link directories
target_link_directories(schack_med_motor PRIVATE
    "C:/SFML/lib"
    "C:/Program Files/SFML/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/../SFML/lib"
)

# Copy SFML DLLs to output directory (Windows)
if(WIN32)
    # Copy SFML DLLs to the build directory
    add_custom_command(TARGET schack_med_motor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/libsfml-graphics-2.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/libsfml-window-2.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/libsfml-system-2.dll"
            $<TARGET_FILE_DIR:schack_med_motor>
    )

    # Copy stockfish.exe to the build directory
    add_custom_command(TARGET schack_med_motor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/stockfish.exe"
            $<TARGET_FILE_DIR:schack_med_motor>
    )

    # Copy image files to the build directory
    add_custom_command(TARGET schack_med_motor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/images"
            $<TARGET_FILE_DIR:schack_med_motor>/images
    )
endif()

# Set output directory
set_target_properties(schack_med_motor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
