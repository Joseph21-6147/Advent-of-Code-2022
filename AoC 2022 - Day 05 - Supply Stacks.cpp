// AoC 2022 - day 05 - Supply Stacks
// =================================

// date:  2022-12-05
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0,
    TEST,
    PUZZLE
} glbProgPhase;

// ==========   DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// there are two data structures in this puzzle - the crate stacks and the rearrange input

// crate stacks are represented as strings. The character at 0 is the top of the stack
vector<string> glbStacks;

// rearrange data consists of 'moves' having a nr of crates, and the stacks to get crates from and to
typedef struct datumStruct {
    int nr, from, to;
} DatumType;
typedef vector<DatumType> DataStream;

// NOTE - in the puzzle the stacks are indexed from 1, I index them from zero

// ==========   DATA INPUT FUNCTIONS

void SetStacks_EXAMPLE() {

    //     [D]
    // [N] [C]
    // [Z] [M] [P]
    //  1   2   3

    glbStacks = {
        "NZ",
        "DCM",
        "P"
    };
}

void SetStacks_PUZZLE() {

    //             [C]         [N] [R]
    // [J] [T]     [H]         [P] [L]
    // [F] [S] [T] [B]         [M] [D]
    // [C] [L] [J] [Z] [S]     [L] [B]
    // [N] [Q] [G] [J] [J]     [F] [F] [R]
    // [D] [V] [B] [L] [B] [Q] [D] [M] [T]
    // [B] [Z] [Z] [T] [V] [S] [V] [S] [D]
    // [W] [P] [P] [D] [G] [P] [B] [P] [V]
    //  1   2   3   4   5   6   7   8   9

    glbStacks = {
        "JFCNDBW",
        "TSLQVZP",
        "TJGBZP",
        "CHBZJLTD",
        "SJBVG",
        "QSP",
        "NPMLFDVB",
        "RLDBFMSP",
        "RTDV",
    };
}

void SetStacks_TEST() { SetStacks_PUZZLE(); }

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {

    // first init the initial crate stack arrangement
    SetStacks_EXAMPLE();

    // then init the rearrange steps
    DatumType aux;

    aux.nr = 1; aux.from = 2 - 1; aux.to = 1 - 1; dData.push_back( aux );
    aux.nr = 3; aux.from = 1 - 1; aux.to = 3 - 1; dData.push_back( aux );
    aux.nr = 2; aux.from = 2 - 1; aux.to = 1 - 1; dData.push_back( aux );
    aux.nr = 1; aux.from = 1 - 1; aux.to = 2 - 1; dData.push_back( aux );
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

// file input - this function assumes each non empty line to contain 1 char on position 0, and 1 on position 2
void ReadInputData( const string sFileName, DataStream &vData ) {
    // first init the initial crate stack arrangement
    SetStacks_PUZZLE();
    // then read the rearrange instructions from file
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;

    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            string aux;
            aux = get_token_dlmtd( " ", sLine ); // ignore "move"
            aux = get_token_dlmtd( " ", sLine ); datum.nr   = atoi( aux.c_str());
            aux = get_token_dlmtd( " ", sLine ); // ignore "from"
            aux = get_token_dlmtd( " ", sLine ); datum.from = atoi( aux.c_str()) - 1;
            aux = get_token_dlmtd( " ", sLine ); // ignore "to"
            aux = get_token_dlmtd( " ", sLine ); datum.to   = atoi( aux.c_str()) - 1;
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
    cout << "Nr: " << iData.nr << " From stack: " << iData.from << " To stack: " << iData.to << endl;
}

// output to console for testing
void PrintStacks() {
    for (int i = 0; i < (int)glbStacks.size(); i++) {
        cout << "Stack[ " << i << " ] = " << glbStacks[i] << endl;
    }
    cout << endl;
}

// output to console for testing
void PrintDataStream( DataStream &dData ) {
    PrintStacks();
    for (auto &e : dData) {
        PrintDatum( e );
    }
    cout << endl;
}

// ==========   PROGRAM PHASING

void InitCrates() {
    switch( glbProgPhase ) {
        case EXAMPLE: SetStacks_EXAMPLE(); break;
        case TEST:    SetStacks_TEST();    break;
        case PUZZLE:  SetStacks_PUZZLE();  break;
        default: cout << "ERROR: InitCrates() --> program phase unknown: " << glbProgPhase << endl;
    }
}

// populates input data, by calling the appropriate input function that is associated
// with the global program phase var
void GetInput( DataStream &dData, bool bDisplay = false ) {

    switch( glbProgPhase ) {
        case EXAMPLE: GetData_EXAMPLE( dData ); break;
        case TEST:    GetData_TEST(    dData ); break;
        case PUZZLE:  GetData_PUZZLE(  dData ); break;
        default: cout << "ERROR: GetInput() --> program phase unknown: " << glbProgPhase << endl;
    }

    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// Cuts of and returns a token of size nSize from the front of input_to_be_adapted.
// This input string becomes shorter as a result of this cut off, and may even become empty.
string get_token_sized( const int nSize, string &input_to_be_adapted ) {
    string token;
    token = input_to_be_adapted.substr( 0, nSize );
    input_to_be_adapted = (nSize == (int)string::npos) ? "" : input_to_be_adapted.substr( nSize );
    return token;
}

// rearrange function for one instruction - part 1: crates are rearranged one at a time
void Rearrange1( int nr, int from, int to ) {
    for (int i = 0; i < nr; i++) {
        string sCrate = get_token_sized( 1, glbStacks[from] );
        string newStack = sCrate;
        newStack.append( glbStacks[to] );
        glbStacks[to] = newStack;
    }
}

// rearrange function for one instruction - part 2: crates are rearranged in stacks
void Rearrange2( int nr, int from, int to ) {
    string sCrates = get_token_sized( nr, glbStacks[from] );
    string newStack = sCrates;
    newStack.append( glbStacks[to] );
    glbStacks[to] = newStack;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream rearrData;
    GetInput( rearrData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << rearrData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    for (int i = 0; i < (int)rearrData.size(); i++) {
//        cout << "step: " << i << ": ";
//        PrintDatum( rearrData[i] );
        Rearrange1( rearrData[i].nr, rearrData[i].from, rearrData[i].to );
//        PrintStacks();
    }
    cout << endl << "Answer 1 - accumulated stack tops: ";
    for (int i = 0; i < (int)glbStacks.size(); i++) {
        cout << glbStacks[i][0];
    }
    cout << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

// IMPORTANT NOTE: outcomment part 1 (or reinit the crates) to start with the right initial setup!!

    InitCrates();

    for (int i = 0; i < (int)rearrData.size(); i++) {
//        cout << "step: " << i << ": ";
//        PrintDatum( rearrData[i] );
        Rearrange2( rearrData[i].nr, rearrData[i].from, rearrData[i].to );
//        PrintStacks();
    }
    cout << endl << "Answer 2- accumulated stack tops: ";
    for (int i = 0; i < (int)glbStacks.size(); i++) {
        cout << glbStacks[i][0];
    }
    cout << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
