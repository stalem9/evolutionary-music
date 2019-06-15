cmake_minimum_required(VERSION 3.14)
project(music_composition)

set(CMAKE_CXX_STANDARD 17)

add_executable(music_composition
        src/main.cpp src/individual.cpp src/Binasc.cpp src/MidiEvent.cpp src/MidiEventList.cpp src/MidiFile.cpp src/MidiMessage.cpp src/Options.cpp
        include/Binasc.h include/MidiEvent.h include/MidiEventList.h include/MidiFile.h include/MidiMessage.h include/individual.h include/Options.h)