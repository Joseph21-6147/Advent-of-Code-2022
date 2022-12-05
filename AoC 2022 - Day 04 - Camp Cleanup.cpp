// AoC 2022 - day 04 - Camp Cleanup
// ================================

// date:  2022-12-04
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

// the data consists of pairs (f - first, s - second) of elves each having a range of sections to clean up
typedef struct datumStruct {
    int f1, f2, s1, s2;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    aux = { 2, 4, 6, 8 }; dData.push_back( aux );
    aux = { 2, 3, 4, 5 }; dData.push_back( aux );
    aux = { 5, 7, 7, 9 }; dData.push_back( aux );
    aux = { 2, 8, 3, 7 }; dData.push_back( aux );
    aux = { 6, 6, 4, 6 }; dData.push_back( aux );
    aux = { 2, 6, 4, 8 }; dData.push_back( aux );
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

// file input - this function assumes each non empty line to be of format "xx-xx,xx-xx" where each
// xx is an integer number
void ReadInputData( const string sFileName, DataStream &vData ) {

    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;

    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            string aux;
            aux = get_token_dlmtd( "-", sLine ); datum.f1 = atoi( aux.c_str() );
            aux = get_token_dlmtd( ",", sLine ); datum.f2 = atoi( aux.c_str() );
            aux = get_token_dlmtd( "-", sLine ); datum.s1 = atoi( aux.c_str() );
            aux = get_token_dlmtd( ",", sLine ); datum.s2 = atoi( aux.c_str() );
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
    cout << "first: " << iData.f1 << "-" << iData.f2 << " second: " << iData.s1 << "-" << iData.s2 << endl;
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

bool RangesLeftOverlap(  DatumType &iData ) { return (iData.f2 >= iData.s1) && (iData.f1 <= iData.s2); }
bool RangesRightOverlap( DatumType &iData ) { return (iData.s2 >= iData.f1) && (iData.s1 <= iData.f2); }

bool RangesOverlap( DatumType &iData ) {
    return RangesLeftOverlap( iData ) || RangesRightOverlap( iData );
}

bool RangeLeftContained(  DatumType &iData ) { return (iData.f1 >= iData.s1) && (iData.f2 <= iData.s2); }
bool RangeRightContained( DatumType &iData ) { return (iData.s1 >= iData.f1) && (iData.s2 <= iData.f2); }

bool RangesContained( DatumType &iData ) {
    return RangeLeftContained( iData ) || RangeRightContained( iData );
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream elfPairData;
    GetInput( elfPairData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << elfPairData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    int nNrContainments = 0;
    for (auto e : elfPairData) {
        if (RangesContained( e )) nNrContainments += 1;
    }
    cout << endl << "Answer 1 - nr contained ranges: " << nNrContainments << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    int nNrOverlaps = 0;
    for (auto e : elfPairData) {
        if (RangesOverlap( e )) nNrOverlaps += 1;
    }
    cout << endl << "Answer 2 - nr overlapping ranges: " << nNrOverlaps << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
