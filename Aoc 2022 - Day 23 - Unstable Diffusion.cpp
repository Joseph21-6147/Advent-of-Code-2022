// AoC 2022 - day 23 - Unstable Diffusion
// ======================================

// date:  2022-12-23
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the input data consists of a map modeled as a number of strings. The map has empty cells '.' and elves '#'
typedef string DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {

    dData.push_back( "....." );
    dData.push_back( "..##." );
    dData.push_back( "..#.." );
    dData.push_back( "....." );
    dData.push_back( "..##." );
    dData.push_back( "....." );
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;

    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            vData.push_back( sLine );
        }
    }
    dataFileStream.close();
}

// input.test.txt contains the larger example from the puzzle
void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << iData << endl;
}

// output to console for testing
void PrintDataStream( DataStream &dData ) {
    for (auto &e : dData) {
        PrintDatum( e );
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
        default: cout << "ERROR: GetInput() --> unknown program phase: " << glbProgPhase << endl;
    }
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// I expect the map to grow quite large, so I don't want a 2d data structure
// Instead I model the map as a list (vector) of elves that occupy a cell (pos)
// This also helps to keep track of the proposals and the cycle per elf.
typedef struct sElfStruct {
    vi2d pos;         // where is elf currently
    vi2d prop;        // what does it propose for next move
    int cntr = 0;     // keeps track of elf proposal cycle

    bool bIdle = false;
} ElfType;
typedef vector<ElfType> ElfStream;

// used for map printing
int glbMapX = 0, glbMapY = 0;

// converts the input data to the puzzle fit data structure
void ConvertInput( DataStream &iData, ElfStream &eData ) {
    glbMapY = (int)iData.size();
    glbMapX = (glbMapY == 0 ? 0 : iData[0].length());

    for (int y = 0; y < glbMapY; y++) {
        for (int x = 0; x < glbMapX; x++) {
            if (iData[y][x] == '#') {
                ElfType aux;
                aux.pos = vi2d( x, y );
                eData.push_back( aux );
            }
        }
    }
}

// convenience lookup function - returns index if found (-1 if not found)
int GetElfIndex( ElfStream &eData, vi2d location ) {
    int result = -1;
    for (int i = 0; i < (int)eData.size() && result == -1; i++) {
        if (eData[i].pos == location) result = i;
    }
    return result;
}

// returns true if location is not occupied
bool IsEmpty( ElfStream &eData, vi2d location ) {
    return (GetElfIndex( eData, location ) == -1);
}

// in the algo you need to check on different empty patterns. I programmed these out here.
bool IsClearAround( ElfStream &eData, vi2d location ) {
    return (
        IsEmpty( eData, location + vi2d( -1, -1 )) &&
        IsEmpty( eData, location + vi2d(  0, -1 )) &&
        IsEmpty( eData, location + vi2d( +1, -1 )) &&
        IsEmpty( eData, location + vi2d( -1,  0 )) &&
        IsEmpty( eData, location + vi2d( +1,  0 )) &&
        IsEmpty( eData, location + vi2d( -1, +1 )) &&
        IsEmpty( eData, location + vi2d(  0, +1 )) &&
        IsEmpty( eData, location + vi2d( +1, +1 ))
    );
}

bool IsClearNorth( ElfStream &eData, vi2d location ) {
    return (
        IsEmpty( eData, location + vi2d( -1, -1 )) &&
        IsEmpty( eData, location + vi2d(  0, -1 )) &&
        IsEmpty( eData, location + vi2d( +1, -1 ))
    );
}

bool IsClearSouth( ElfStream &eData, vi2d location ) {
    return (
        IsEmpty( eData, location + vi2d( -1, +1 )) &&
        IsEmpty( eData, location + vi2d(  0, +1 )) &&
        IsEmpty( eData, location + vi2d( +1, +1 ))
    );
}

bool IsClearWest(  ElfStream &eData, vi2d location ) {
    return (
        IsEmpty( eData, location + vi2d( -1, -1 )) &&
        IsEmpty( eData, location + vi2d( -1,  0 )) &&
        IsEmpty( eData, location + vi2d( -1, +1 ))
    );
}

bool IsClearEast(  ElfStream &eData, vi2d location ) {
    return (
        IsEmpty( eData, location + vi2d( +1, -1 )) &&
        IsEmpty( eData, location + vi2d( +1,  0 )) &&
        IsEmpty( eData, location + vi2d( +1, +1 ))
    );
}

// administers a proposed location for curElf, both in the elf itself and in the propList
void Propose( vector<vi2d> &propList, ElfType &curElf, vi2d location ) {
    propList.push_back( location );
    curElf.prop = location;
}

// attempts to do a proposal. Since this will not always succeed, (and might fail for all four
// directions) the function returns whether true if a proposal could be made
bool TryProposal( ElfStream &eData, vector<vi2d> &propList, ElfType &curElf ) {
    vi2d offsets[4] = {
        vi2d(  0, -1 ), // north
        vi2d(  0, +1 ), // south
        vi2d( -1,  0 ), // west
        vi2d( +1,  0 ), // east
    };
    int cntr = 0;
    int bSuccess = false;
    while (!bSuccess && cntr < 4) {
        switch ((curElf.cntr + cntr) % 4) {
            case 0: if (IsClearNorth( eData, curElf.pos )) { Propose( propList, curElf, curElf.pos + offsets[0] ); bSuccess = true; } break;
            case 1: if (IsClearSouth( eData, curElf.pos )) { Propose( propList, curElf, curElf.pos + offsets[1] ); bSuccess = true; } break;
            case 2: if (IsClearWest(  eData, curElf.pos )) { Propose( propList, curElf, curElf.pos + offsets[2] ); bSuccess = true; } break;
            case 3: if (IsClearEast(  eData, curElf.pos )) { Propose( propList, curElf, curElf.pos + offsets[3] ); bSuccess = true; } break;
        }
        cntr += 1;
    }
    return bSuccess;   // NOTE - it could be that none of these options succeed... [ this may well be the snag for today :) ]
}

// Re-arragnges the map (= eData structure) to let elves do their thing for one round.
// Returns number of elves that were active in this round.
int OneRound( ElfStream &eData ) {

    vector<vi2d> proposals;
    int nrActive = 0;

    // phase 1 - consider for all elves where to move
    // as a result - either the elf is declared idle or it has a proposal
    // all proposals are stored in the proposals list
    for (int i = 0; i < (int)eData.size(); i++) {
        ElfType &curElf = eData[i];

        // are there any neighbours close by?
        bool bIdle = IsClearAround( eData, curElf.pos );
        if (bIdle) {
            curElf.bIdle = true;
        } else {
            // Try to propose. If it fails, the elf is again idle
            curElf.bIdle = !TryProposal( eData, proposals, curElf );
        }
    }

    // phase 2 - actually move for all elves (unless idle)
    for (int i = 0; i < (int)eData.size(); i++) {
        ElfType &curElf = eData[i];
        if (!curElf.bIdle) {   // skip all idle elves
            if (count( proposals.begin(), proposals.end(), curElf.prop ) == 1) {
                // actually move to the proposed location
                curElf.pos = curElf.prop;
                // keep track of nr of active elves
                nrActive += 1;
            }
        }
        curElf.cntr = (curElf.cntr + 1) % 4;
    }
    return nrActive;
}

// for testing
void PrintElvesMap( ElfStream &eData ) {
    // create empty map
    vector<string> myMap;
    for (int y = 0; y < glbMapY; y++) {
        string oneLine;
        for (int x = 0; x < glbMapX; x++) {
            oneLine.append( "." );
        }
        myMap.push_back( oneLine );
    }
    // project elves onto it
    for (int i = 0; i < (int)eData.size(); i++) {
        ElfType &curElf = eData[i];
        myMap[ curElf.pos.y ][ curElf.pos.x ] = '#';
    }
    // display
    for (int i = 0; i < (int)myMap.size(); i++) {
        cout << myMap[i] << endl;
    }
}

// gets the bounding box coordinates upper left lower right from the
// elf map eData
void GetBoundingBox( ElfStream &eData, vi2d &ul, vi2d &lr ) {
    ul = vi2d( INT_MAX, INT_MAX );
    lr = vi2d( INT_MIN, INT_MIN );
    for (int i = 0; i < (int)eData.size(); i++) {
        vi2d &curPos = eData[i].pos;
        if (curPos.x < ul.x) ul.x = curPos.x;
        if (curPos.y < ul.y) ul.y = curPos.y;
        if (curPos.x > lr.x) lr.x = curPos.x;
        if (curPos.y > lr.y) lr.y = curPos.y;
    }
}

// works out the number of cells in the bounding box of eData
int GetBBoxNrOfCells( ElfStream &eData ) {
    vi2d ul, lr;
    GetBoundingBox( eData, ul, lr );
    int lenX = abs( ul.x - lr.x ) + 1;
    int lenY = abs( ul.y - lr.y ) + 1;
    return lenX * lenY;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE
    flcTimer tmr;

/* ========== */   tmr.StartTiming();   // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream inputData;
    GetInput( inputData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << inputData.size() << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

    // part 1 code here

    // put the input data in convenient puzzle ds
    ElfStream eMap1;
    ConvertInput( inputData, eMap1 );
    if (glbProgPhase != PUZZLE) {
        cout << "Initial situation" << endl;
        PrintElvesMap( eMap1 );
    }
    // calculate nr empty ground tiles from bounding box after 10 rounds
    for (int i = 0; i < 10; i++) {
        int nrActiveElves = OneRound( eMap1 );
        if (glbProgPhase != PUZZLE) {
            cout << endl << "After round: " << i + 1 << " there are " << nrActiveElves << " elves active" << endl;
            PrintElvesMap( eMap1 );
        }
    }
    // report the answer
    int nrTotalCells = GetBBoxNrOfCells( eMap1 );
    int nrElves = eMap1.size();
    cout << endl << "Answer 1 - after 10 rounds nr cells in bounding box = " << nrTotalCells << ", nr elves = " << nrElves << ", nr empty ground tiles = " << nrTotalCells - nrElves << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // put the input data in convenient puzzle ds
    ElfStream eMap2;
    ConvertInput( inputData, eMap2 );
    if (glbProgPhase != PUZZLE) {
        cout << "Initial situation" << endl;
        PrintElvesMap( eMap2 );
    }

    // run the simulation until all elves have become idle
    int nRoundAllIdle = -1;
    for (int i = 1; nRoundAllIdle == -1; i++) {
        int nrActiveElves = OneRound( eMap2 );
        if (nrActiveElves == 0)
            nRoundAllIdle = i;  // this will terminate the loop

        if (glbProgPhase != PUZZLE) {
            cout << endl << "After round: " << i << " there are " << nrActiveElves << " elves active" << endl;
            PrintElvesMap( eMap2 );
        } else {
            // for larger data do some form of progress indication
            if ((i % 100) == 0) {
                cout << "Round: " << i << ", active elves: " << nrActiveElves << endl;
            }
        }
    }
    // report the round that it happens the first time
    cout << endl << "Answer 2 - The first round where all elves were idle is : " << nRoundAllIdle << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
