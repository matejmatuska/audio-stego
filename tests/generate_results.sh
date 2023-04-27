#!/bin/bash

# Script for testing various properties of different steganography methods
#
# Test structure:
# INPUT_DIR/
#   cover1
#   cover2
#   ...
#   coverN
#
# OUTPUT_DIR/
#   INPUT_DIR/
#      covern.output/
#          lsb/
#              message.txt
#              ber.txt
#              ...
#          phase/
#              message.txt
#              ber.txt
#              ...
#          echo/
#          tone/

BIN="../build/src/stego"
PARAMS_FILE="./params.txt"
_ARGS=${ARGS:-}



COVERS_DIR=$1
OUTPUT_DIR=$2
MSG_FILE=$3

MAX_JOBS=8

test_method() {
    local method=$1
    local cover=$2
    local test_dir=$3
    local params=$4

    local stego="$test_dir/out.wav"
    local out;

    if ! "$BIN" embed -m "$method" -cf "$cover" -sf "$stego" $_ARGS -k "$params" \
        < "$MSG_FILE"  2> "$test_dir/stderr.txt";
    then
        echo 1>&2 "Failed to embed to cover: $cover"
        return 1
    fi

    out="$method;$(./snr -db "$cover" "$stego")"

    mkdir -p "$test_dir/extract"
    if ! "$BIN" extract -m "$method" -sf "$stego" $_ARGS -k "$params" \
        > "$test_dir/extract/stdout.txt" 2> "$test_dir/extract/stderr.txt";
    then
        echo 1>&2 "Failed to extract from stego: $stego"
        return 1
    fi
    out+=";$(./ber "$MSG_FILE" "$test_dir/extract/stdout.txt")"

    #local fs
    #fs=$(soxi -r "$cover")
    #local target_fs=$(( fs / 2 ))
    #sox "$stego" "$test_dir/resample/out.wav" rate "$target_fs"
    #
    mkdir -p "$test_dir/resample"
    sox "$stego" "$test_dir/resample/tmp.wav" downsample
    sox "$test_dir/resample/tmp.wav" "$test_dir/resample/out.wav" upsample
    "$BIN" extract -m "$method" -sf "$test_dir/resample/out.wav" $_ARGS -k "$params" \
        > "$test_dir/resample/stdout.txt" 2> "$test_dir/resample/stderr.txt"
    out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/resample/stdout.txt")"

    mkdir -p "$test_dir/amplify"
    sox "$stego" "$test_dir/amplify/out.wav" vol 3dB
    "$BIN" extract -m "$method" -sf "$test_dir/amplify/out.wav" $_ARGS -k "$params" \
        > "$test_dir/amplify/stdout.txt" 2> "$test_dir/amplify/stderr.txt"
    out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/amplify/stdout.txt")"

    mkdir -p "$test_dir/attenuate"
    sox "$stego" "$test_dir/attenuate/out.wav" vol -3dB
    "$BIN" extract -m "$method" -sf "$test_dir/attenuate/out.wav" $_ARGS -k "$params" \
        > "$test_dir/attenuate/stdout.txt" 2> "$test_dir/attenuate/stderr.txt"
    out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/attenuate/stdout.txt")"

    local bit_depth
    bit_depth=$(soxi -b "$stego")
    if [ "$bit_depth" -ge 8 ]; then
        local target_bits=$(( bit_depth - 8 ))
        mkdir -p "$test_dir/less_bits"
        sox "$stego" -b "$target_bits" "$test_dir/less_bits/tmp.wav"
        sox "$test_dir/less_bits/tmp.wav" -b "$bit_depth" "$test_dir/less_bits/out.wav"
        "$BIN" extract -m "$method" -sf "$test_dir/less_bits/out.wav" $_ARGS -k "$params" \
            > "$test_dir/less_bits/stdout.txt" 2> "$test_dir/less_bits/stderr.txt"
        out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/less_bits/stdout.txt")"
    else
        out+="N/A"
    fi

    mkdir -p "$test_dir/mp3-compress"
    sox "$stego" -C 128 "$test_dir/mp3-compress/out.mp3"
    "$BIN" extract -m "$method" -sf "$test_dir/mp3-compress/out.mp3" $_ARGS -k "$params" \
        > "$test_dir/mp3-compress/stdout.txt" 2> "$test_dir/mp3-compress/stderr.txt"
    out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/mp3-compress/stdout.txt")"

    mkdir -p "$test_dir/aac-compress"
    ffmpeg -y -i "$stego" -c:a aac -b:a 128k "$test_dir/aac-compress/out.m4a"
    ffmpeg -y -i "$test_dir/aac-compress/out.m4a" "$test_dir/aac-compress/out.wav"
    "$BIN" extract -m "$method" -sf "$test_dir/aac-compress/out.wav" $_ARGS -k "$params" \
        > "$test_dir/aac-compress/stdout.txt" 2> "$test_dir/aac-compress/stderr.txt"
    out+=";$(./ber "$test_dir/extract/stdout.txt" "$test_dir/aac-compress/stdout.txt")"

    type="${cover##*/}" # strip path
    type=${type%-*}
    echo "$out;$params;${type}" # newline

    # TODO low/high pass?
    # TODO take a look at steganalysis attacks
}

print_help() {
    echo "Usage: test BIN COVERS_DIR OUTPUT_DIR MSG_FILE"
}

if [ "$1" = "--help" ]; then
    print_help
    exit 0
fi

if [ ! -d "$COVERS_DIR" ]; then
    echo "COVERS_DIR must be a directory, see --help" 1>&2
    exit 1
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    echo "OUTPUT_DIR must be a directory, see --help" 1>&2
    exit 1
fi

print_header() {
    echo "method;snr;extraction;resampling;amplification;attenuation;requantization;mp3-compress;aac-compress;params;type"
}

print_header
for cover in "$COVERS_DIR"/*; do
    test_dir="$OUTPUT_DIR/${cover##*/}.out"

    jobs=0
    mkdir -p "$test_dir"
    while IFS=' ' read -r method params; do
        [ "${method::1}" = \# ] && continue
        method_dir="$test_dir/$method-$params"
        mkdir -p "$method_dir"
        test_method "$method" "$cover" "$method_dir" "$params" > "$OUTPUT_DIR/$method-$params.tmp" &
        if [ $jobs -eq $MAX_JOBS ]; then
            wait
            for tmpf in "$OUTPUT_DIR/"*.tmp; do
                cat "$tmpf"
                rm "$tmpf"
            done
            jobs=0
        fi
        jobs=$((jobs + 1))
    done < $PARAMS_FILE
    # wait if there are less than MAX_JOBS jobs
    wait
    for tmpf in "$OUTPUT_DIR/"*.tmp; do
        cat "$tmpf"
        rm "$tmpf"
    done
done
