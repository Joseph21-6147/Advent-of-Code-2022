// AoC 2022 - day 09 - Rope Bridge
// ===============================

// date:  2022-12-09
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"   // needed for vi2d type coordinates

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0,
    TEST,
    PUZZLE
} glbProgPhase;

// ==========   DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

#define NN '?'
#define UP 'U'
#define DN 'D'
#define LT 'L'
#define RT 'R'

// the data consists of 'movements of the head' having a direction (L, R, U, D) and nr of steps
typedef struct datumStruct {
    char cDir;
    int  nSteps;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    bool bExample1 = true;
    if (bExample1) {
        // example data for part 1
        aux.cDir = 'R'; aux.nSteps = 4; dData.push_back( aux );
        aux.cDir = 'U'; aux.nSteps = 4; dData.push_back( aux );
        aux.cDir = 'L'; aux.nSteps = 3; dData.push_back( aux );
        aux.cDir = 'D'; aux.nSteps = 1; dData.push_back( aux );
        aux.cDir = 'R'; aux.nSteps = 4; dData.push_back( aux );
        aux.cDir = 'D'; aux.nSteps = 1; dData.push_back( aux );
        aux.cDir = 'L'; aux.nSteps = 5; dData.push_back( aux );
        aux.cDir = 'R'; aux.nSteps = 2; dData.push_back( aux );
    } else {
        // example data for part 2
        aux.cDir = 'R'; aux.nSteps =  5; dData.push_back( aux );
        aux.cDir = 'U'; aux.nSteps =  8; dData.push_back( aux );
        aux.cDir = 'L'; aux.nSteps =  8; dData.push_back( aux );
        aux.cDir = 'D'; aux.nSteps =  3; dData.push_back( aux );
        aux.cDir = 'R'; aux.nSteps = 17; dData.push_back( aux );
        aux.cDir = 'D'; aux.nSteps = 10; dData.push_back( aux );
        aux.cDir = 'L'; aux.nSteps = 25; dData.push_back( aux );
        aux.cDir = 'U'; aux.nSteps = 20; dData.push_back( aux );
    }
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
            string sDirToken = get_token_dlmtd( " ", sLine );
            datum.cDir = sDirToken[0];
            datum.nSteps = atoi( sLine.c_str());
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
    cout << iData.cDir << " " << iData.nSteps << endl;
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
        default: cout << "ERROR: GetInput() --> program phase not recognized: " << glbProgPhase << endl;
    }
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

vi2d glbHead = { 0, 0 };    // for part 1 - head and tail are explicit variables
vi2d glbTail = { 0, 0 };

vector<vi2d> glbRope = {    // for part 2 - head and tail are elements [0] and [9] of glbRope
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};

vector<vi2d> glbTrail;     // to store the trail of the tail :)

// for part 1 this was hardcoded using glbHead variable, for part 2 I need to have it work on glbRope[0]
void MoveHead( char cDirection, vi2d &curHead = glbHead ) {
    switch (cDirection) {
        case LT: curHead.x -= 1; break;
        case RT: curHead.x += 1; break;
        case UP: curHead.y += 1; break;
        case DN: curHead.y -= 1; break;
        default: cout << "ERROR: MoveHead() --> unknown direction: " << cDirection << endl;
    }
}

void MoveSegment( vi2d pred, vi2d &cur ) {

    int dX = pred.x - cur.x;
    int dY = pred.y - cur.y;

    if (abs(dX) >= 2 && dY == 0) {                     // horizontal movement
        cur.x += (dX > 0 ? +1 : -1);
    } else if (abs(dY) >= 2 && dX == 0) {              // vertical movement
        cur.y += (dY > 0 ? +1 : -1);
    } else if (!(abs( dX ) <= 1 && abs( dY ) <= 1)) {  // if not touching

        cur.x += (dX > 0 ? +1 : -1);                   // diagonal movement
        cur.y += (dY > 0 ? +1 : -1);
    }
}

// After writing MoveSegment() for part 2 I rewrote MoveTail() for part 1 to be a specific case of MoveSegment()
void MoveTail() { MoveSegment( glbHead, glbTail ); }

// convenience function to output vi2d type coordinate
string CoordToString( vi2d coord ) {
    string s;
    s.append( "( " );
    s.append( to_string( coord.x ));
    s.append( ", " );
    s.append( to_string( coord.y ));
    s.append( " )" );
    return s;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream moveData;
    GetInput( moveData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << moveData.size() << endl << endl;

    tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

// ========== part 1

    glbTrail.push_back( glbTail );  // start position counts as visited !!

    // iterate all move instructions
    for (auto &curMove : moveData) {
        // iterate all steps of one instruction
        for (int j = 0; j < curMove.nSteps; j++) {

            if (glbProgPhase != PUZZLE) {
                cout << "Moving step " << j + 1 << " of " << curMove.nSteps << " in direction: " << curMove.cDir;
                cout << " Head before: " << CoordToString( glbHead ) << " Tail before: " << CoordToString( glbTail );
            }

            MoveHead( curMove.cDir );
            MoveTail();
            glbTrail.push_back( glbTail );

            if (glbProgPhase != PUZZLE) {
                cout << " Head after: " << CoordToString( glbHead ) << " Tail after: " << CoordToString( glbTail ) << endl;
            }
        }
    }
    int nTrailSizeBefore = glbTrail.size();
    // remove duplicates using sort and erase/unique idiom
    sort( glbTrail.begin(), glbTrail.end() );
    glbTrail.erase( unique( begin( glbTrail ), end( glbTrail )), end( glbTrail ));
    int nTrailSizeAfter = glbTrail.size();
    cout << "trail size before and after duplicate removal: " << nTrailSizeBefore << " --> " << nTrailSizeAfter << endl;

    cout << endl << "Answer 1 - nr of visited locations: " << nTrailSizeAfter << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

// ========== part 2

    glbTrail.clear();
    glbTrail.push_back( glbRope[9] );  // start position counts as visited !!

    for (auto &curMove : moveData) {
        for (int j = 0; j < curMove.nSteps; j++) {
            MoveHead( curMove.cDir, glbRope[0] );
            // after moving the head, move all the segments in turn
            for (int k = 1; k < 10; k++) {
                MoveSegment( glbRope[k - 1], glbRope[k] );
            }
            // store the location of the tail
            glbTrail.push_back( glbRope[9] );
        }
    }

    nTrailSizeBefore = glbTrail.size();
    // remove duplicates using sort and erase/unique idiom
    sort( glbTrail.begin(), glbTrail.end() );
    glbTrail.erase( unique( glbTrail.begin(), glbTrail.end()), glbTrail.end());
    nTrailSizeAfter = glbTrail.size();
    cout << "trail size before and after duplicate removal: " << nTrailSizeBefore << " --> " << nTrailSizeAfter << endl;

    cout << endl << "Answer 2 - nr of visited locations: " << nTrailSizeAfter << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
