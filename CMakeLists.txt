cmake_minimum_required(VERSION 3.16)

project(latent)

set(CMAKE_CXX_STANDARD 20)

# src files
add_executable(latent
    prototype.cpp
)

# headers 
target_include_directories(latent PRIVATE
    include
)

# current dir where the network lib is located
target_link_directories(latent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# link to the network lib
target_link_libraries(latent PRIVATE
    Network
)
