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

find_library(FFTW3 fftw3 REQUIRED)
find_library(SNDFILE sndfile REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SNDFILE} ${FFTW3})


find_path(FFTW3_HEADER NAMES "fftw3.h")
find_path(SNDFILE_HEADER NAMES "sndfile.hh")

target_include_directories(${PROJECT_NAME} PUBLIC ${SNDFILE_HEADER} ${FFTW3_HEADER})
