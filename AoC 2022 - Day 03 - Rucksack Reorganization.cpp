// AoC 2022 - day 03 - Rucksack Reorganization
// ===========================================

// date:  2022-12-03
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

// the data consists of
//   * org          - original rucksack contents (characters)
//   * comp1, comp2 - the contents for compartments 1 and 2
//   * shared       - the one shared item in compartments 1 and 2
//   * prio         - the prio of that shared item
typedef struct sDatumType {
    string org, comp1, comp2;
    char   shared;
    int    prio;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;
    aux.org.clear(); aux.org.append( "vJrwpWtwJgWrhcsFMMfFFhFp"         ); dData.push_back( aux );
    aux.org.clear(); aux.org.append( "jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL" ); dData.push_back( aux );
    aux.org.clear(); aux.org.append( "PmmdzqPrVvPwwTWBwg"               ); dData.push_back( aux );
    aux.org.clear(); aux.org.append( "wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn"   ); dData.push_back( aux );
    aux.org.clear(); aux.org.append( "ttgJtRGJQctTZtZT"                 ); dData.push_back( aux );
    aux.org.clear(); aux.org.append( "CrZsJsPPZsGzwwsLwLmpwMDw"         ); dData.push_back( aux );
}

// file input

// this function assumes each non empty line to contain 1 char on position 0, and 1 on position 2
void ReadInputData( const string sFileName, DataStream &vData ) {

    ifstream dataFileStream( sFileName );
    vData.clear();
    string token;
    DatumType datum;

    while (getline( dataFileStream, token )) {
        if (token.length() > 0) {    // non empty line
            datum.org.clear();
            datum.org.append( token );
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
    cout << "original: " << iData.org << endl;
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

int GetPriority( char item ) {
    int result = -1;
    if ('a' <= item && item <= 'z') result = item - 'a' +  1;
    if ('A' <= item && item <= 'Z') result = item - 'A' + 27;
    return result;
}

// Cuts of and returns a token of size nSize from the front of input_to_be_adapted.
// This input string becomes shorter as a result of this cut off, and may even become empty.
string get_token_sized( const int nSize, string &input_to_be_adapted ) {
    string token;
    token = input_to_be_adapted.substr( 0, nSize );
    input_to_be_adapted = (nSize == (int)string::npos) ? "" : input_to_be_adapted.substr( nSize );
    return token;
}

// For part 1 - first split original content string in equal length compartments, then find
// the shared item in these two compartments and calc the priority of it
void ProcessRucksack( DatumType &rs ) {
    // first split the original string up in two equal length compartment
    string auxOrg = rs.org;
    int sLen = auxOrg.length();
    rs.comp1 = get_token_sized( sLen / 2, auxOrg );
    rs.comp2 = auxOrg;
    // now find the common item in first and second compartments
    bool bFound = false;
    char cCommon = ' ';
    for (int i = 0; i < (int)rs.comp1.length() && !bFound; i++) {
        bFound = (rs.comp2.find( rs.comp1[i] ) != string::npos);
        if (bFound)
            cCommon = rs.comp1[i];
    }
    // check if a shared item was found
    if (!bFound) {
        cout << "ERROR: ProcessRucksack() --> Couldn't find shared item..." << endl;
    } else {
        rs.shared = cCommon;
        rs.prio = GetPriority( cCommon );
    }
//    cout << "org: " << rs.org << " len: " << sLen << " comp1: " << rs.comp1 << " comp2: " << rs.comp2 << " shared: " << rs.shared << " priority value: " << rs.prio << endl;
}

// returns true if cBadge occurs in all three elf strings, false otherwise
bool IsBadge( char cBadge, string &elf1, string &elf2, string &elf3 ) {
    return (elf1.find( cBadge ) != string::npos) &&
           (elf2.find( cBadge ) != string::npos) &&
           (elf3.find( cBadge ) != string::npos);
}

// returns the one character that is the badge of all three elves
// (that is contained in all three strings)
char FindBadge( string &elf1, string &elf2, string &elf3 ) {
    char cFound = ' ';
    bool bFound = false;
    for (char c = 'a'; c <= 'z' && !bFound; c++) {
        bFound = IsBadge( c, elf1, elf2, elf3 );
        if (bFound) cFound = c;
    }
    for (char c = 'A'; c <= 'Z' && !bFound; c++) {
        bFound = IsBadge( c, elf1, elf2, elf3 );
        if (bFound) cFound = c;
    }
    return cFound;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream sackData;
    GetInput( sackData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << sackData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    int nCumulatePrios1 = 0;
    for (auto &elt : sackData) {
        ProcessRucksack( elt );
        nCumulatePrios1 += elt.prio;
    }
    cout << endl << "Answer 1 - accumulated priorities: " << nCumulatePrios1 << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    int nCumulatePrios2 = 0;
    for (int i = 0; i < (int)sackData.size(); i += 3) {
        char cFound = FindBadge( sackData[i].org, sackData[i + 1].org, sackData[i + 2].org );
        nCumulatePrios2 += GetPriority( cFound );
    }
    cout << endl << "Answer 2 - accumulated priorities: " << nCumulatePrios2 << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
