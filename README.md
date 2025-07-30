**Author**: Matej Matuška

## Building
### Prerequisites
Two libraries are required for the build:

- FFTW3 -
to install the library on Fedora Linux system:
```
# dnf install fftw-devel
```
- libsndfile - on Fedora Linux installed via:
```
# dnf install libsndfile-devel
```

### Building with CMake
First enter the `src` directory and create `build` directory:
```
$ cd src/
$ mkdir build/
```

Then configure the project. For release (optimized) build:
```
$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
```
or, for debug build:
```
$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
```
To build with bundled libraries add the following option:
```
-DCMAKE_PREFIX_PATH=$(realpath ./src/libs)
```

After configuration build the project using:
```
$ make -C build -j 4
```

## Usage
The program always runs in one of four main modes:

- `embed` - Embeds message into cover file.

- `extract` - Extract message from stego file.

- `info` -  Prints information about audio file (sample rate, duration, ...) and
  embedding capacity for individual methods.

- `--help` - Prints help.

The syntax for the individual commands is following:
```
embed -m <method> -cf <coverfile> -sf <stegofile> -mf <msgfile> [-k <key>] [-e] [-l <limit>]
extract -m <method> -sf <stegofile> -mf <msgfile> [-k <key>] [-e] [-l <limit>]
info <file> [-k key]
```

The following arguments are accepted:

|Option|Meaning                                         |
|------|------------------------------------------------|
|-cf   |The cover file                                  |
|-sf   |The stego file                                  |
|-mf   |Message file, if ommitted stdin/stdout is used  |
|-m    |The steganographic method to use.               |
|-k    |The stego key (method parameter)                |
|-e    |Use Hamming code for the message                |
|-l    |Limit the message length                        |

The following methods are supported:

|Name    |Full Name                         |
|--------|----------------------------------|
|lsb     |Least significant bit substitution|
|phase   |Phase coding                      |
|echo    |Echo hiding                       |
|echo-hc |Echo hiding high capacity         |
|tone    |Tone insertion                    |

See `--help` for more information.

## Used libraries
The following libraries are used by the project:

- FFTW3 - https://www.fftw.org/
- libsndfile - http://www.mega-nerd.com/libsndfile/. libsndfile can optionally
  be compiled with the following libraries:
    - FLAC - https://xiph.org/flac/
    - Opus - https://opus-codec.org/
    - Ogg - https://xiph.org/ogg/
    - Vorbis - https://xiph.org/vorbis/
