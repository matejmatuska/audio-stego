set(CMAKE_CXX_STANDARD 17)

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
    echo_hiding_hc.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE -lsndfile -lfftw3)
