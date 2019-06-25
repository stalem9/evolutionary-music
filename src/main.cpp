#include <iostream>
#include "../include/individual.h"
#include <random>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include "../include/MidiFile.h"
#include "../include/Options.h"
#include <chrono>
#include <ctime>

using namespace std;
using namespace smf;

int pop_size;

MidiFile* population;

int getNotesCount(MidiFile a);

void breed(MidiFile a, MidiFile b, int place, int pair_n);

int calculate_fitness(MidiFile target);

void mutate();

bool sortRule(MidiFile a, MidiFile b);


int main(int argc, char** argv) {
    srand(time(NULL));
    auto start = chrono::system_clock::now();
    Options options;
    options.process(argc, argv);

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
    uniform_int_distribution<int> pitch(36, 84);

    for (int i = 0; i < pop_size; ++i){
        //read and copy data from target track
        target.write("./population/individual" + to_string(i + 1) + ".mid");

        random_device rd;
        mt19937 mt(rd());

        MidiFile midifile;
        midifile.read("./population/individual" + to_string(i + 1) + ".mid");
        midifile.addTrack();
        int track   = midifile.getTrackCount() - 1;
        int channel = 0;
        int instr   = options.getInteger("instrument");
        midifile.addTimbre(track, 0, channel, instr);

        int tpq     = target.getTPQ();
        //cout << tpq;
        for(int j = 0; j < target[0].getEventCount(); ++j){
            if(target[0][j].isNoteOn()){
                //int key = pitch(mt);
                int key = target[0][j].getKeyNumber() + random()%14-7;
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
        midifile.doTimeAnalysis();
        midifile.linkNotePairs();

        string outname = "./population/gen0_individual" + to_string(i + 1) + ".mid";

        if (outname.empty()) {
            if (options.getBoolean("hex")) midifile.writeHex(cout);
            else cout << midifile;
        } else
            midifile.write(outname);
        population[i] = midifile;

    }
    auto end_generation = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end_generation-start;
    cout << "initial population generation completed in " << elapsed_seconds.count() << endl;

    bool found = false;
    int pop_number = 0;

    while (!found && pop_number < 1000){

        calculate_fitness(target);

        std::sort(population, population+pop_size, sortRule);

        int part = pop_size/4;
        int place = target[0].getEventCount()/2;
        while(!target[0][place].isNoteOff()) place--;

        for(int i = 0; i < part; i+=2){
            MidiFile a = population[i];
            MidiFile b = population[i+1];

            breed(a, b, place, i/2);
        }

        //mutation

        for(int i = 0; i < pop_size; ++i){
            string outname = "./population/gen_" + to_string(pop_number) + "individual" + to_string(i + 1) + ".mid";
            population[i].write(outname);
            if(population[i].fit > 900){
                found = true;
            }
        }
        cout << pop_number << " " << population[0].fit << endl;
        auto end_iteration = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end_iteration-end_generation;
        cout << pop_number << "th iteration complete in " << elapsed_seconds.count() << endl;
        end_generation = end_iteration;

        pop_number++;
    }

    return 0;
}


int getNotesCount(MidiFile a){
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

void breed(MidiFile a, MidiFile b, int place, int pair_n){
    MidiFile ab1 = a, ab2 = b;
    for(int i = 0; i < place; ++i){
        ab2[1][i] = a[1][i];
        ab1[1][i] = b[1][i];

    }
    population[pop_size -(pair_n*2) - 1] = ab1;
    population[pop_size - (pair_n*2 + 1) - 1] = ab2;

}

int calculate_fitness(MidiFile target){

    for (int i = 0; i < pop_size; ++i){
        int count = 0;
        int fit  = population[i].fit;
        for(int j = 0; j < population[i][1].getEventCount(); j++){
            if(population[i][1][j].isNote()){
                if(population[i][1][j].getKeyNumber() == target[0][j].getKeyNumber())
                    count++;
                //cout << "count! " << count << endl;
            }
        }
        fit += 3000*count/(getNotesCount(population[i]));
        population[i].fit = fit*fit/2000;
    }

    return 0;
}

void mutate(){

}

bool sortRule(MidiFile a, MidiFile b){
    return a.fit > b.fit;
}
