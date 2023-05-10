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
find_library(OGG ogg REQUIRED)
find_library(VORBIS vorbis REQUIRED)
find_library(VORBISENC vorbisenc REQUIRED)
find_library(OPUS opus REQUIRED)
find_library(FLAC FLAC REQUIRED)
find_library(LAME NAMES mp3lame lame REQUIRED)
find_library(MPG123 mpg123 REQUIRED)
find_library(SNDFILE sndfile REQUIRED)

find_path(SNDFILE_HEADER sndfile.hh)

target_include_directories(${PROJECT_NAME} PUBLIC ${SNDFILE_HEADER})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${FFTW3}
    ${SNDFILE}
    ${LAME}
    ${MPG123}
    ${OPUS}
    ${FLAC}
    ${VORBISENC}
    ${VORBIS}
    ${OGG}
)
