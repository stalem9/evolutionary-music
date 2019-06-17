//
// Created by ultrachemical on 13.06.19.
//

#include "../include/individual.h"

Individual::Individual(int *instrument, int *volume, int *pitch, int *note_value, int temp, int time_signature) {
    this->fit = 0;
    set_instrument(instrument);
    set_volume(volume);
    set_pitch(pitch);
    set_note_value(note_value);
    set_temp(temp);
    set_time_signature(time_signature);
}

void Individual::set_fit() {

}

int Individual::get_fit() {
    return 0;
}

void Individual::set_instrument(int *instrument) {
    this->instrument = instrument;
}

int *Individual::get_instrument() {
    return this->instrument;
}

void Individual::set_volume(int *volume) {
    this->volume = volume;
}

int *Individual::get_volume() {
    return this->volume;
}

void Individual::set_pitch(int *pitch) {
    this->pitch = pitch;
}

int *Individual::get_pitch() {
    return this->pitch;
}

void Individual::set_note_value(int *note_value) {
    this->note_value = note_value;
}

int *Individual::get_note_value() {
    return this->note_value;
}

void Individual::set_temp(int temp) {
    this->temp = temp;
}

int Individual::get_temp() {
    return this->temp;
}

void Individual::set_time_signature(int time_signature) {
    this->time_signature = time_signature;
}

int Individual::get_time_signature() {
    return this->time_signature;
}

void Individual::mutate() {

}
