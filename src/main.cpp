#include <iostream>
#include "../include/individual.h"
#include <random>
#include <time.h>
#include <stdlib.h>
#include "../include/MidiFile.h"
#include "../include/Options.h"

using namespace std;
using namespace smf;

int pop_size;

void breed(Individual a, Individual b){

}

Individual* population;

int main(int argc, char** argv) {
    srand(time(NULL));
    // read

    Options options;
    options.define("n|note-count=i:10", "How many notes to randomly play");
    options.define("o|output-file=s",   "Output filename (stdout if none)");
    options.define("i|instrument=i:0",  "General MIDI instrument number");
    options.define("x|hex=b",           "Hex byte-code output");
    options.process(argc, argv);

    MidiFile target;
    string filename;
    cin >> filename;
    if (options.getArgCount() > 0) target.read(options.getArg(1));
    else target.read(filename);

    cout << target.getFileDurationInSeconds() << endl;
    // copy to buffer
    target.write("../new.mid");

    cin >> pop_size;

    // create initial population
    for (int i = 0; i < pop_size; ++i){
        //read
        int* instrument = nullptr;
        int* volume = nullptr;
        int* pitch = nullptr;
        int* note_value = nullptr;
        int temp = 0;
        int time_signature = 0;
        population[i] = *new Individual(instrument, volume, pitch, note_value, temp, time_signature);

    }

    //while fitness is not satisfied do{

        // calculate fitness of population

        // sort

        //selection

        //breeding

        //form new population

        //mutation

    //}

    // read buffer

    // create new track


    //random track generation just to understand how to work with library
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> starttime(0, 100);
    uniform_int_distribution<int> duration(1, 8);
    uniform_int_distribution<int> pitch(36, 84);
    uniform_int_distribution<int> velocity(40, 100);

    MidiFile midifile;
    int track   = 0;
    int channel = 0;
    int instr   = options.getInteger("instrument");
    midifile.addTimbre(track, 0, channel, instr);

    int tpq     = midifile.getTPQ();
    int count   = options.getInteger("note-count");
    for (int i=0; i<count; i++) {
        int starttick = int(starttime(mt) / 4.0 * tpq);
        int key       = pitch(mt);
        int endtick   = starttick + int(duration(mt) / 4.0 * tpq);
        midifile.addNoteOn (track, starttick, channel, key, velocity(mt));
        midifile.addNoteOff(track, endtick,   channel, key);
    }
    midifile.sortTracks();  // Need to sort tracks since added events are
    // appended to track in random tick order.
    string outname = "../output.mid";
    if (outname.empty()) {
        if (options.getBoolean("hex")) midifile.writeHex(cout);
        else cout << midifile;
    } else
        midifile.write(outname);

    cout << midifile.getFileDurationInSeconds() << " seconds" << endl;

    return 0;
}