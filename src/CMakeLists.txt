set(CMAKE_CXX_STANDARD 17)

find_library(ZLIB z)

set(SOURCES)
if(ZLIB)
    list(APPEND SOURCES compression/zlib_compress.cpp)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    args.cpp
    autocorrelation.cpp
    conv.cpp
    echo_hiding.cpp
    fft.cpp
    phase_embedder.cpp
    phase_extractor.cpp
    processing.cpp
    tone_insertion.cpp
    methods.cpp
    bitvector.cpp
    ibitstream.cpp
    obitstream.cpp
    compression/compress.cpp
    ${SOURCES}
)

if(ZLIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
    add_definitions(-DHAS_ZLIB)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE -lsndfile -lfftw3)
