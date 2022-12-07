// AoC 2022 - day 06 - Tuning Trouble
// ==================================

// date:  2022-12-06
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

// the data consists of 'signals' modeled by characters
// put them in a vector since I'm unsure of string length boundaries getting in the way
typedef char DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    string auxData = "mjqjpqmgbljsphdztnvjfqwrcgsmlb";     // the puzzle provides multiple examples
//    string auxData = "bvwbjplbgvbhsrlpgdmjqwftvncz";
//    string auxData = "nppdvjthqldpwncqszvftbrmjlhg";
//    string auxData = "nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg";
//    string auxData = "zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw";

    // convert string to vector of char's
    for (int i = 0; i < (int)auxData.length(); i++) {
        dData.push_back( auxData[i] );
    }
}

// file input - reads 1 line at a time from .txt file, and process it into vData
// adapt the parsing and processing of the lines to match the puzzle input
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;

    while (getline( dataFileStream, sLine )) {    // for this particular puzzle I expect only 1 line of data
        if (sLine.length() > 0) {    // non empty line
            for (int i = 0; i < (int)sLine.length(); i++) {
                vData.push_back( sLine[i] );
            }
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << iData;
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

// Returns true if the datastream segment of length nrDifferent and ending with index 'index'
// contains all different characters. Returns false otherwise.
bool NrDifferentFound( DataStream &dData, int index, int nrDifferent ) {
    bool bUnEqual = true;
    for (int j = nrDifferent - 1; j > 0 && bUnEqual; j--) {
        for (int k = j - 1; k >= 0 && bUnEqual; k--) {
            bUnEqual = ( dData[index - j] != dData[index - k] );
        }
    }
    return bUnEqual;
}

// A regular marker is 4 different characters. A Start Of Message marker is 14 different characters
bool IsMarkerFound( DataStream &dData, int i ) { return NrDifferentFound( dData, i,  4 ); }
bool IsSOMFound(    DataStream &dData, int i ) { return NrDifferentFound( dData, i, 14 ); }

// return the first end index of the segment in Datastream that is of length nrDiff and contains
// only different characters
int FindFirstPattern( DataStream &dData, int nrDiff ) {
    int result = -1;
    bool bFound = false;
    for (int i = nrDiff - 1; i < (int)dData.size() && !bFound; i++) {
        if (NrDifferentFound( dData, i, nrDiff )) {
            result = i;
            bFound = true;
        }
    }
    return result;
}

// A regular marker is 4 different characters. A Start Of Message marker is 14 different characters
int FindFirstMarker( DataStream &dData ) { return FindFirstPattern( dData,  4 ); }
int FindFirstSOM(    DataStream &dData ) { return FindFirstPattern( dData, 14 ); }

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream signalData;
    GetInput( signalData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << signalData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    int nAnswer1 = FindFirstMarker( signalData );
    cout << endl << "Answer 1 - first marker found at index: " << nAnswer1 << " (which is " << nAnswer1+1 << "th letter!)" << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    int nAnswer2 = FindFirstSOM( signalData );
    cout << endl << "Answer 2 - first start of message marker found at index: " << nAnswer2 << " (which is " << nAnswer2+1 << "th letter!)" << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
