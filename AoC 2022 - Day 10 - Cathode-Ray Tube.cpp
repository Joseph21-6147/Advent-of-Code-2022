// AoC 2022 - day 10 Cathode-Ray Tube
// ==================================

// date:  2022-12-10
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of 'instructions' having an opcode (name) and optional integer value
// and a number of cycles
typedef struct datumStruct {
    string sOpcode  = "empty";
    int    nOperand = 0;
    int    nCycles  = 0;
} DatumType;
typedef vector<DatumType> DataStream;

// there's one register X that initially holds value 1
// a state vector keeps track of what X's value is at any cycle
typedef struct registerStruct {
    int nCycle;
    int nValue;
} RegisterType;
vector<RegisterType> glbX;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;
    aux.sOpcode = "noop";                    aux.nCycles = 1; dData.push_back( aux );
    aux.sOpcode = "addx"; aux.nOperand =  3; aux.nCycles = 2; dData.push_back( aux );
    aux.sOpcode = "addx"; aux.nOperand = -5; aux.nCycles = 2; dData.push_back( aux );
}

// Cuts of and returns the front token from "input_to_be_adapted", using "delim" as delimiter.
// If delimiter is not found, the complete input string is passed as a token.
// The input string becomes shorter as a result, and may even become empty
string get_token_dlmtd( const string &delim, string &input_to_be_adapted ) {
    size_t splitIndex = input_to_be_adapted.find( delim );
    string token = input_to_be_adapted.substr( 0, splitIndex );
    input_to_be_adapted = (splitIndex == string::npos) ? "" : input_to_be_adapted.substr( splitIndex + 1 );
    return token;
}

// file input - this function reads text file one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;
    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            string sToken = get_token_dlmtd( " ", sLine );
            if (sToken == "noop") {
                datum.sOpcode = sToken;
                datum.nCycles = 1;
            } else if (sToken == "addx") {
                datum.sOpcode = sToken;
                datum.nOperand = atoi( sLine.c_str());
                datum.nCycles = 2;
            } else {
                cout << "ERROR: ReadInputData() --> unknown opcode: " << sToken << endl;
            }
            vData.push_back( datum );
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << "(cycles: " << iData.nCycles << "):    " << iData.sOpcode << " " << iData.nOperand << endl;
}

// output to console for testing
void PrintDataStream( DataStream &dData ) {
    for (int i = 0; i < (int)dData.size(); i++) {
        cout << "Instr. " << i << " ";
        PrintDatum( dData[i] );
    }
    cout << endl;
}

// ==========   PROGRAM PHASING

// populates input data, by calling the appropriate input function that is associated
// with the global program phase var
void GetInput( DataStream &dData, bool bDisplay = false ) {

    switch( glbProgPhase ) {
        case EXAMPLE: GetData_EXAMPLE( dData ); break;
        case TEST:    GetData_TEST(    dData ); break;
        case PUZZLE:  GetData_PUZZLE(  dData ); break;
        default: cout << "ERROR: GetInput() --> program phase not recognized: " << glbProgPhase << endl;
    }
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

void RunProgram( DataStream &dData ) {
    // start with clean x register state
    glbX.clear();
    int xValue = 1;
    int nCycleCount = 0;
    glbX.push_back( { nCycleCount, xValue } );     // register X starts with value 1 at cycle 0

    // iterate over all instructions in the program
    for (int i = 0; i < (int)dData.size(); i++) {
        DatumType &curInstr = dData[i];

        if (curInstr.sOpcode == "noop") {          // push one cycle and leave xValue unchanged
            nCycleCount += 1;
            glbX.push_back( { nCycleCount, xValue } );

        } else if (curInstr.sOpcode == "addx") {   // push two cycles and alter xValue
            nCycleCount += 1;
            glbX.push_back( { nCycleCount, xValue } );
            nCycleCount += 1;
            glbX.push_back( { nCycleCount, xValue } );
            xValue += curInstr.nOperand;
        } else {
            cout << "ERROR: RunProgram() --> unknown opcode: " << curInstr.sOpcode << endl;

        }
    }
    // make sure to add last xValue to the glbX vector to make it apparent
    nCycleCount += 1;
    glbX.push_back( { nCycleCount, xValue } );
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream progData;
    GetInput( progData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << progData.size() << endl << endl;

    tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

// ========== part 1

    // trigger at selected cycles
    auto select_condition = [=]( int cycle ) {
        return (cycle % 40 == 20);
    };

    RunProgram( progData );

    int nCumulatedSignalStrenght = 0;
    for (int i = 0; i < (int)glbX.size(); i++) {

        // debug output in EXAMPLE or TEST phases
        if (glbProgPhase != PUZZLE)
            cout << "During " << glbX[i].nCycle << "-th cycle X = " << glbX[i].nValue;

        if (select_condition( glbX[i].nCycle )) {

            // debug output in EXAMPLE or TEST phases
            if (glbProgPhase != PUZZLE)
                cout << " --> signal strenght: " << glbX[i].nCycle << " * " << glbX[i].nValue << " = " << glbX[i].nCycle * glbX[i].nValue;

            nCumulatedSignalStrenght += glbX[i].nCycle * glbX[i].nValue;
        }

        // debug output in EXAMPLE or TEST phases
        if (glbProgPhase != PUZZLE)
            cout << endl;
    }
    cout << endl << "Answer 1 - accumulated signal strength: " << nCumulatedSignalStrenght << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

// ========== part 2

    // check if denoted column (nCol) is masked by value nVal
    auto sprite_mask = [=]( int nVal, int nCol ) {
        return (nVal - 1 <= nCol && nCol <= nVal + 1);
    };

    // start at cycle 1
    for (int i = 1; i < (int)glbX.size(); i++) {
        int nCol = (i - 1) % 40;
        if (nCol == 0)
            cout << endl;
        // using a space instead of a dot makes better readability
        cout << (sprite_mask( glbX[i].nValue, nCol ) ? "#" : " ");
    }
    cout << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
