// AoC 2022 - day 02 - Rock Paper Scissors
// =======================================

// date:  2022-12-02
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

#define F_ROCK 'A'   // first select, 'A' is interpreted as rock, etc
#define F_PAPR 'B'
#define F_SCRS 'C'

#define S_ROCK 'X'   // second select, 'X' is interpreted as rock, etc
#define S_PAPR 'Y'
#define S_SCRS 'Z'

#define S_LOSE 'X'   // second alternative select interpretation, 'X' is interpreted as must lose, etc
#define S_DRAW 'Y'
#define S_WIN  'Z'

// the data consists of 'turns' having a first and second selection
typedef struct datumStruct {
    char f, s;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    aux.f = 'A'; aux.s = 'Y'; dData.push_back( aux );
    aux.f = 'B'; aux.s = 'X'; dData.push_back( aux );
    aux.f = 'C'; aux.s = 'Z'; dData.push_back( aux );
}

// this function assumes each non empty line to contain 1 char on position 0, and 1 char on position 2
void ReadInputData( const string sFileName, DataStream &vData ) {

    ifstream dataFileStream( sFileName );
    vData.clear();
    string token;
    DatumType datum;

    while (getline( dataFileStream, token )) {
        if (token.length() > 0) {    // non empty line
            datum.f = token[0];
            datum.s = token[2];
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
    cout << iData.f << " " << iData.s << endl;
}

// output to console for testing
void PrintDataStream( DataStream &dData ) {
    for (auto &e : dData) {
        PrintDatum( e );
    }
    cout << endl;
}

// ==========   PROGRAM PHASING

// populates input data
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

// determines and returns points if player loses (0 points), wins (6 points) or its a draw (3 points)
int OutcomeScore( char fst, char scd ) {
    int result = -1;
    switch (fst) {
        case F_ROCK:
            switch (scd) {
                case S_ROCK: result = 3; break;  // draw
                case S_PAPR: result = 6; break;  // win
                case S_SCRS: result = 0; break;  // lost
            }
            break;
        case F_PAPR:
            switch (scd) {
                case S_ROCK: result = 0; break;
                case S_PAPR: result = 3; break;
                case S_SCRS: result = 6; break;
            }
            break;
        case F_SCRS:
            switch (scd) {
                case S_ROCK: result = 6; break;
                case S_PAPR: result = 0; break;
                case S_SCRS: result = 3; break;
            }
            break;
    }
    return result;
}

// put score to chosen player form (rock = 1, paper = 2, scissors = 3)
int SelectionScore( char scd ) {
    int result = -1;
    switch (scd) {
        case S_ROCK: result = 1; break;
        case S_PAPR: result = 2; break;
        case S_SCRS: result = 3; break;
    }
    return result;
}

// determines which form to chose to get desired outcome if elf choses form fst
char SelectSecond( char fst, char outcome ) {
    char result = ' ';
    switch (fst) {
        case F_ROCK:
            switch (outcome) {
                case S_LOSE: result = S_SCRS; break;
                case S_DRAW: result = S_ROCK; break;
                case S_WIN : result = S_PAPR; break;
            }
            break;
        case F_PAPR:
            switch (outcome) {
                case S_LOSE: result = S_ROCK; break;
                case S_DRAW: result = S_PAPR; break;
                case S_WIN : result = S_SCRS; break;
            }
            break;
        case F_SCRS:
            switch (outcome) {
                case S_LOSE: result = S_PAPR; break;
                case S_DRAW: result = S_SCRS; break;
                case S_WIN : result = S_ROCK; break;
            }
            break;
    }
    return result;
}

// for part 1 - score calculation for 1 turn: just assign score for selection and for the outcome
int TotalScore1( char fst, char scd ) {
    return SelectionScore( scd ) + OutcomeScore( fst, scd );
}

// for part 2 - score calculation for 1 turn: first determine the selection from the
// first selection and the desired outcome then assign points based on that selection and outcome
int TotalScore2( char fst, char outcome ) {
    char newSelect = SelectSecond( fst, outcome );
    return SelectionScore( newSelect ) + OutcomeScore( fst, newSelect );
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream turnData;
    GetInput( turnData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << turnData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    int nAccumulateScore = 0;
    for (auto elt : turnData ) {
//        PrintDatum( elt );
        int nLocalScore = TotalScore1( elt.f, elt.s );
//        cout << "Local score: " << nLocalScore << endl;
        nAccumulateScore += nLocalScore;
    }

    cout << endl << "Answer 1 - accumulated score: " << nAccumulateScore << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    nAccumulateScore = 0;
    for (auto elt : turnData ) {
//        PrintDatum( elt );
        int nLocalScore = TotalScore2( elt.f, elt.s );
//        cout << "Local score: " << nLocalScore << endl;
        nAccumulateScore += nLocalScore;
    }

    cout << endl << "Answer 2 - accumulated score: " << nAccumulateScore << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
