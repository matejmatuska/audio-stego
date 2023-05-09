set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
    main.cpp
    args.cpp
    autocepstrum.cpp
    conv.cpp
    echo_hiding.cpp
    fft.cpp
    ifft.cpp
    lsb_substitution.cpp
    phase_coding.cpp
    dsp_utils.cpp
    tone_insertion.cpp
    method_factory.cpp
    audioparams.cpp
    hamminginbitstream.cpp
    hammingoutbitstream.cpp
    bitvector.cpp
    ibitstream.cpp
    obitstream.cpp
    echo_hiding_hc.cpp
    util.cpp
    stegofile.cpp
    coverfile.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE -lsndfile -lfftw3)
