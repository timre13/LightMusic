# Versions

## Pre-alpha 1.0-dev #1: 2020.01.10 - 2020.01.11
    * LightMusic was born :)
    * Basic core works
    * Can play one file - filename is constant
    * Prints info of container and streams
    * Cannot play float/double sample formats
    * Command line only, non-interactive

## Pre-alpha 1.0-dev #2: 2020.01.16
    * Use libavdevice library instead of libao
    * Don't block the audio device when playing (libao blocked it)

## Pre-alpha 1.0-dev #3: 2020.01.19
    * Convert input format to output device's format (MP3 finally works!)
