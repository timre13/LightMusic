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

## Pre-alpha 1.0-dev #4: 2020.01.22:
    * Refactored
    * Implemented playlist
    * Some fixes done

## Alpha 1.0-dev: 2020.01.26:
    * Refactored `Music`
    * Refactored `Playlist`
    * Added new getters to `Music`
    * Added new getters to `Playlist`
    * Implemented GUI
        * Shows tracks in playlist (mark currently playing)
        * Shows track info
        * Pausing/playing
        * Jumping to next/previous track
        * Shows progress in current track as a slider
        * Shows elapsed/full length of track
