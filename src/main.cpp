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
#include <set>

using namespace std;
using namespace smf;


int pop_size;

MidiFile* population;

set <int> target_tonality = {6, 8, 10, 11, 1, 3, 5}; //D-dur
set <int> dominante = {1, 3, 5, 6, 8, 10, 12}; //A-dur
set <int> subdominante = {1, 3, 4, 6, 8, 10, 11}; //G-dur


int getNotesCount(const MidiFile &a);

void breed(const MidiFile &a, const MidiFile &b, int pair_n);

int calculate_fitness(const MidiFile &target);

int calculate_tonality(MidiFile &a);

void mutate(int place);


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
        midifile.addTrack();
        int track   = midifile.getTrackCount() - 1;
        int channel = 0;
        int instr   = options.getInteger("instrument");
        midifile.addTimbre(track, 0, channel, instr);

        int tpq     = target.getTPQ();
        int shift = random()%50 - 38;
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

    while (pop_number < 1000){


        calculate_fitness(target);

        sort(population, population+pop_size, sort_rule);

        int part = pop_size/4;

        for(int i = 0; i < part; i+=2){
            MidiFile &a = population[i];
            MidiFile &b = population[i+1];
            breed(a, b, i/2);
        }

        //mutation
//        int part_m = (pop_size + 9)/10;
//        for(int i = 0; i < part_m; i++){
//            mutate(i);
//        }

        for(int i = 0; i < pop_size; ++i){
            population[i].doTimeAnalysis();
            population[i].linkNotePairs();

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
    MidiFile ab1, ab2;
    ab1.addTrack();
    ab2.addTrack();
    int track   = ab1.getTrackCount() - 1;
    int channel = 0;
    int instr   = 0;
    ab1.addTimbre(track, 0, channel, instr);
    ab2.addTimbre(track, 0, channel, instr);

    int tpq     = a.getTPQ();
    int size = a.getEventCount(1);
    for(int i = 0; i < size; i++){

        if(a[1][i].isNoteOn()){
            int start = i, current_note = 0, next_note = 1, len = 0;
            while(start+next_note < size && !a[1][start+next_note].isNoteOn()) next_note++;

            //start search progression
            while(start+next_note < size && a[1][start+current_note].getKeyNumber() < a[1][start+next_note].getKeyNumber()){
                len++;
                current_note = next_note;
                next_note++;
                while(start+next_note < size && !a[1][start+next_note].isNoteOn()) next_note++;
            }
            //if has
            if(len > 2){
                int j;
                for(j = start; j < start + next_note; j++){
                    if(a[1][j].isNoteOn()){
                        ab1.addNoteOn(1, b[1][j].tick, 0, b[1][j].getKeyNumber(), b[1][j].getVelocity());
                        if(b[1][j].isLinked())
                            ab1.addNoteOff(1, b[1][j].getLinkedEvent()->tick, 0, b[1][j].getKeyNumber());

                        ab2.addNoteOn(1, a[1][j].tick, 0, a[1][j].getKeyNumber(), a[1][j].getVelocity());
                        if(a[1][j].isLinked())
                            ab2.addNoteOff(1, a[1][j].getLinkedEvent()->tick, 0, a[1][j].getKeyNumber());
                    }
                }
                i = j;
            }
                start = i, current_note = 0, next_note = 1, len = 0;
                while(start+next_note < size && !a[1][start+next_note].isNoteOn()) next_note++;
                //start search regression
                while(start+next_note < size && a[1][start+current_note].getKeyNumber() > a[1][start+next_note].getKeyNumber()){
                    len++;
                    current_note = next_note;
                    next_note++;
                    while(start+next_note < size && !a[1][start+next_note].isNoteOn()) next_note++;
                }//if has
                //swap regression
                if(len > 2){
                    int j;
                    for(j = start; j < start + next_note ; j++){
                        if(a[1][j].isNoteOn()){
                            ab1.addNoteOn(1, b[1][j].tick, 0, b[1][j].getKeyNumber(), b[1][j].getVelocity());
                            if(b[1][j].isLinked())
                                ab1.addNoteOff(1, b[1][j].getLinkedEvent()->tick, 0, b[1][j].getKeyNumber());

                            ab2.addNoteOn(1, a[1][j].tick, 0, a[1][j].getKeyNumber(), a[1][j].getVelocity());
                            if(a[1][j].isLinked())
                                ab2.addNoteOff(1, a[1][j].getLinkedEvent()->tick, 0, a[1][j].getKeyNumber());
                        }
                    }
                    i = j;
                }

                if(len <= 2 && b[1][i].isNoteOn()) {
                    ab2.addNoteOn(1, b[1][i].tick, 0, b[1][i].getKeyNumber(), b[1][i].getVelocity());
                    if(b[1][i].isLinked())
                        ab2.addNoteOff(1, b[1][i].getLinkedEvent()->tick, 0, b[1][i].getKeyNumber());

                    ab1.addNoteOn(1, a[1][i].tick, 0, a[1][i].getKeyNumber(), a[1][i].getVelocity());
                    if(a[1][i].isLinked())
                        ab1.addNoteOff(1, a[1][i].getLinkedEvent()->tick, 0, a[1][i].getKeyNumber());
                }

        }

    }

    ab1.sortTrack(1);
    ab1.setTPQ(tpq);
    ab1.linkNotePairs();
    ab1.doTimeAnalysis();

    ab2.sortTrack(1);
    ab2.setTPQ(tpq);
    ab2.linkNotePairs();
    ab2.doTimeAnalysis();

    population[pop_size - pair_n*2 - 1].erase();
    population [pop_size -(pair_n*2) - 1] = ab1;
    population[pop_size - (pair_n*2 + 1) - 1].erase();
    population[pop_size - (pair_n*2 + 1) - 1] = ab2;

}

int calculate_fitness(const MidiFile &target){

    for (int i = 0; i < pop_size; ++i){
        int fit  = 0, n1 = 0, n2 = 0, n3 = 0;
        int size = min(population[i][1].getEventCount(), target.getEventCount(0));


        for(int j = 1; j < size; j++){
            if(population[i][1][j].isNoteOn()){
                int pitch = population[i][1][j].getKeyNumber()%12;
                if(pitch == 0) pitch = 12;
                if(target_tonality.count(pitch)) n1++; else n1--;
                if(dominante.count(pitch)) n2++; else n2--;
                if(subdominante.count(pitch)) n3++; else n3--;
            }
        }
        //fit = 200*max_len/(size);
       // population[i].fit = fit*fit;
        fit = (n1*6/10 + n2*2/10 + n3*2/10);
        population[i].fit = fit;
    } //possible intervals in same tone
    return 0;
}

int calculate_tonality(MidiFile a){
    return 0;
}

void mutate(int place){
    srand(time(NULL));
    int size = population[place].getEventCount(1);
    int tpq = population[place].getTPQ();
    MidiFile a;
    a.addTrack();
    int tonality[] = {5, 6, 8, 10, 11, 13, 15};
    for(int j = 0; j < size; j++){
        if(population[place][1][j].isNoteOn()){
            int starttick = population[place][1][j].tick;
            int newKey = tonality[random()%7];

            newKey += ((population[place][1][j].getKeyNumber()+13)/14)*14;
            if(newKey < 0) newKey = 0;
            if(newKey > 127) newKey = 127;
            a.addNoteOn(1, starttick, 0, newKey, population[place][1][j].getVelocity());
            if(population[place][1][j].isLinked()){
                int endtick = population[place][1][j].getLinkedEvent()->tick;
                a.addNoteOff(1, endtick, 0, newKey);

            }

            //main notes tonika, dominanta, subdominanta - check them fitness function
        }
    }
    a.sortTrack(1);
    a.setTPQ(tpq);
    a.linkNotePairs();
    a.doTimeAnalysis();

    population[place].erase();
    population [place] = a;
}

//relative tonalities //difference in languages parallel
