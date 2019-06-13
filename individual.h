//
// Created by ultrachemical on 13.06.19.
//

#ifndef MUSIC_COMPOSITION_INDIVIDUAL_H
#define MUSIC_COMPOSITION_INDIVIDUAL_H


class individual {

    int fit;
    int instrument[100];
    int volume[100];
    int pitch[100];
    int note_value[100];
    int temp;
    int time_signature;

    void mutate();

    void set_fit();

    void allocate();



};


#endif //MUSIC_COMPOSITION_INDIVIDUAL_H
