#include <iostream>
#include <random>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include "../include/MidiFile.h"
#include "../include/Options.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <bits/stdc++.h>

using namespace std;
using namespace smf;


int pop_size;

MidiFile* population;

int getNotesCount(const MidiFile &a);

void breed(const MidiFile &a, const MidiFile &b, int pair_n);

int calculate_fitness(const MidiFile &target);

int calculate_tonality(MidiFile &a);

void mutate(MidiFile &a);


bool sort_rule(const MidiFile &a, const MidiFile &b){
    //cout << a.fit << " " << b.fit << " bool:"  << (a.fit < b.fit) << endl;
    return a.fit > b.fit;
}

int main(int argc, char** argv) {
    ofstream log_writer ("log.txt");
    srand(time(NULL));
    auto start = chrono::system_clock::now();
    log_writer << "started  at " << endl;
    Options options;
    options.process(argc, argv);

    system("exec rm -r ./population/*");

    MidiFile target;
    string filename = "../input.mid";
    //cout << "Enter target file name\n";
    //cin >> filename;
    if (options.getArgCount() > 0) target.read(options.getArg(1));
    else target.read(filename);

    target.joinTracks();
    target.doTimeAnalysis();
    target.linkNotePairs();

    options.define("n|note-count=i:"+to_string(getNotesCount(target)), "How many notes to randomly play");
    options.define("o|output-file=s",   "Output filename (stdout if none)");
    options.define("i|instrument=i:0",  "General MIDI instrument number");
    options.define("x|hex=b",           "Hex byte-code output");


    cout << "Enter population size\n";
    cin >> pop_size;
    population = new MidiFile[pop_size];

    // create initial population

    for (int i = 0; i < pop_size; ++i){
        //read and copy data from target track


        MidiFile midifile;
        midifile.read("./population/individual" + to_string(i + 1) + ".mid");
        midifile.addTrack();
        int track   = midifile.getTrackCount() - 1;
        int channel = 0;
        int instr   = options.getInteger("instrument");
        midifile.addTimbre(track, 0, channel, instr);

        int tpq     = target.getTPQ();
        int shift = random()%20 - 10;
        //cout << tpq;
        for(int j = 0; j < target[0].getEventCount(); ++j){
            if(target[0][j].isNoteOn()){
                //int key = pitch(mt);
                int key = target[0][j].getKeyNumber() + shift;
                if(key < 0) key = 0;

                int starttick = target[0][j].tick;
                midifile.addNoteOn (track, starttick, channel, key, target[0][j].getVelocity());
                if(target[0][j].isLinked()){
                    int endtick = target[0][j].getLinkedEvent()->tick;
                    midifile.addNoteOff(track, endtick, channel, key);
                }

            }

        }

        midifile.sortTracks();
        midifile.deleteTrack(0);
        midifile.joinTracks();
        midifile.setTPQ(tpq);
        midifile.doTimeAnalysis();
        midifile.linkNotePairs();

        string outname = "./population/gen0_individual" + to_string(i + 1) + ".mid";

        midifile.write(outname);
        population[i] = midifile;

    }
    auto end_generation = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_generation - start;
    cout << "initial population generation completed in " << elapsed_seconds.count() << " seconds" << endl;
    log_writer << "initial population generation completed in " << elapsed_seconds.count() << " seconds" << endl;

    bool found = false;
    int pop_number = 0;

    while (!found && pop_number < 1000){


        calculate_fitness(target);

        sort(population, population+pop_size, sort_rule);

        for(int i = 0; i < pop_size; i++){
            cout << population[i].fit << " ";
            population[i].doTimeAnalysis();
            population[i].linkNotePairs();
        } cout << endl;

        int part = pop_size/4;

        for(int i = 0; i < part; i+=2){
            MidiFile &a = population[i];
            MidiFile &b = population[i+1];
            breed(a, b, i/2);
        }

        //mutation

        for(int i = 0; i < pop_size; ++i){
            string outname = "./population/gen_" + to_string(pop_number + 1) + "individual" + to_string(i + 1) + ".mid";
            population[i].write(outname);
            if(population[i].fit >= 9*getNotesCount(target)/10){
                found = true;
            }
        }
        auto end_iteration = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end_iteration - end_generation;
        cout << pop_number << "th iteration completed in " << elapsed_seconds.count() << " seconds with fitness " << population[0].fit << endl;
        log_writer << pop_number << "th iteration completed in " << elapsed_seconds.count() << " seconds with fitness " << population[0].fit << endl;
        end_generation = end_iteration;

        pop_number++;
    }

    log_writer.close();
    return 0;
}


int getNotesCount(const MidiFile &a){
    int count = 0;
    for (int i=0; i<a.getTrackCount(); i++) {
        for (int j=0; j<a[i].getEventCount(); j++) {
            if (a[i][j].isNoteOn()){
                count++;
            }
        }
    }
    return count;
}

void breed(const MidiFile &a, const MidiFile &b, int pair_n){
    MidiFile ab1 = a, ab2 = b;
    int size = a.getEventCount(1);
    for(int i = 0; i < size; i++){
        int beg_place = i, pos = 0, end_place = i;

        //search regression
        while(beg_place + pos + 1 < size && a[1][beg_place + pos].getKeyNumber() >= a[1][beg_place + pos + 1].getKeyNumber()){
            pos++;
        }
        if(pos > 3){
            pos-= 2;
            end_place += pos;
            //swap from beg_place to end_place
            for(pos = beg_place; pos < size && pos < end_place; pos++){
                ab1[1][pos] = b[1][pos];
                ab2[1][pos] = a[1][pos];
            }
            i = end_place;
        }

        //search progression
        while(beg_place + pos + 1 < size && a[1][beg_place + pos].getKeyNumber() <= a[1][beg_place + pos + 1].getKeyNumber()){
            pos++;
        }
        if(pos > 3){
            pos-= 2;
            end_place += pos;
            //swap from beg_place to end_place
            for(pos = beg_place; pos < size && pos < end_place; pos++){
                ab1[1][pos] = b[1][pos];
                ab2[1][pos] = a[1][pos];
            }
            i = end_place;
        }

    }

    ab1.deleteTrack(0);
    ab1.linkNotePairs();
    ab1.doTimeAnalysis();

    ab2.deleteTrack(0);
    ab2.linkNotePairs();
    ab2.doTimeAnalysis();

    population[pop_size - pair_n*2 - 1].erase();
    population [pop_size -(pair_n*2) - 1] = ab1;
    population[pop_size - (pair_n*2 + 1) - 1].erase();
    population[pop_size - (pair_n*2 + 1) - 1] = ab2;

}

int calculate_fitness(const MidiFile &target){

    for (int i = 0; i < pop_size; ++i){
        int len = 0, max_len = 0;
        int fit  = 0;
        int size = population[i][1].getEventCount();
        int dif = 0;


        for(int j = 1; j < size; j++){
            if(population[i][1][j].isNoteOn()){
                if(dif == population[i][1][j].getKeyNumber()  - target[0][j].getKeyNumber())
                    len++;
                else{
                    dif = population[i][1][j].getKeyNumber()  - target[0][j].getKeyNumber();
                    if(len > max_len){
                        max_len = len;
                        len = 0;
                    }
                }
            }
        }
        //fit = 100*max_len/(size);
       // population[i].fit = fit*fit;
        population[i].fit = max_len;
    } //possible intervals in same tone
    return 0;
}

int calculate_tonality(MidiFile a){
    return 0;
}

void mutate(MidiFile &a){
    // определение тональности -- отдельный файл?
    // изменение высоты в рамках тональности
    // треки в одной тональности
}