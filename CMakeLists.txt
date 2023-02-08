cmake_minimum_required(VERSION 3.25.2)

set(CMAKE_CXX_STANDARD 17)

project(stego)

add_executable(${PROJECT_NAME} main.cpp phase_embedder.cpp phase_extractor.cpp echo_hiding.cpp processing.cpp fft.cpp conv.cpp autocorrelation.cpp)


add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE -lsndfile -lfftw3)
