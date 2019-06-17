//
// Created by ultrachemical on 13.06.19.
//

#ifndef MUSIC_COMPOSITION_INDIVIDUAL_H
#define MUSIC_COMPOSITION_INDIVIDUAL_H


class Individual {

private:
    int fit; //fitness
    int* instrument;
    int* volume;
    int* pitch; //высота звука
    int* note_value; //длительность
    int temp;
    int time_signature; //размер

public:
    Individual(int* instrument, int* volume, int* pitch, int* note_value, int temp, int time_signature);

    //fitness function
    void set_fit();

    int get_fit();

    void set_instrument(int* instrument);

    int* get_instrument();

    void set_volume(int* volume);

    int* get_volume();

    void set_pitch(int* pitch);

    int* get_pitch();

    void  set_note_value(int* note_value);

    int* get_note_value();

    void set_temp(int temp);

    int get_temp();

    void set_time_signature(int time_signature);

    int get_time_signature();

    void mutate();




};


#endif //MUSIC_COMPOSITION_INDIVIDUAL_H
