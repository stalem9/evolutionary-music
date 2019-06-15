//
// Created by ultrachemical on 13.06.19.
//

#ifndef MUSIC_COMPOSITION_INDIVIDUAL_H
#define MUSIC_COMPOSITION_INDIVIDUAL_H


class individual {

    int fit;
    int* instrument;
    int* volume;
    int* pitch;
    int* note_value;
    int temp;
    int time_signature;

    void mutate();

    void set_fit();

    void allocate();



};


#endif //MUSIC_COMPOSITION_INDIVIDUAL_H
