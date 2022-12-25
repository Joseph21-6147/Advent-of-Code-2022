// AoC 2022 - day 11c - Monkey in the Middle
// =========================================

// date:  2022-12-25
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <algorithm>

#include "../flcTimer.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of 'turns' having a first and second selection
typedef struct datumStruct {
    int nID         = -1;     // the id of the monkey
    char cOperation = ' ';    // can be '*', '+' or '^' (for square)
    int nOperand    = -1;     // belonging to the operation
    int nDivider    = -1;     // for the monkey test
    int nUponTrue   = -1;     // throw towards this monkey if test success
    int nUponFalse  = -1;     // throw towards this monkey if test failure
    deque<long long> vItems;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// convenience function for code formatting purposes
void MakeDatum( DatumType &datum, int id, char op, int opernd, int divr, int nTrue, int nFalse, deque<long long> items ) {
    datum.nID        = id;
    datum.cOperation = op;
    datum.nOperand   = opernd;
    datum.nDivider   = divr;
    datum.nUponTrue  = nTrue;
    datum.nUponFalse = nFalse;
    datum.vItems     = items;
}

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    // id, operation, operand, divider, iftrue, iffalse, items
    MakeDatum( aux, 0, '*', 19, 23, 2, 3, { 79, 98         } ); dData.push_back( aux );
    MakeDatum( aux, 1, '+',  6, 19, 2, 0, { 54, 65, 75, 74 } ); dData.push_back( aux );
    MakeDatum( aux, 2, '^',  0, 13, 1, 3, { 79, 60, 97     } ); dData.push_back( aux );
    MakeDatum( aux, 3, '+',  3, 17, 0, 1, { 74             } ); dData.push_back( aux );
}

// hardcoded input - just to get the solution tested
void GetData_TEST( DataStream &dData ) {
    DatumType aux;

    // id, operation, operand, divider, iftrue, iffalse, items
    MakeDatum( aux, 0, '*', 13, 11, 3, 2, { 57                             } ); dData.push_back( aux );
    MakeDatum( aux, 1, '+',  2,  7, 6, 7, { 58, 93, 88, 81, 72, 73, 65     } ); dData.push_back( aux );
    MakeDatum( aux, 2, '+',  6, 13, 3, 5, { 65, 95                         } ); dData.push_back( aux );
    MakeDatum( aux, 3, '^',  0,  5, 4, 5, { 58, 80, 81, 83                 } ); dData.push_back( aux );
    MakeDatum( aux, 4, '+',  3,  3, 1, 7, { 58, 89, 90, 96, 55             } ); dData.push_back( aux );
    MakeDatum( aux, 5, '*',  7, 17, 4, 1, { 66, 73, 87, 58, 62, 67         } ); dData.push_back( aux );
    MakeDatum( aux, 6, '+',  4,  2, 2, 0, { 85, 55, 89                     } ); dData.push_back( aux );
    MakeDatum( aux, 7, '+',  7, 19, 6, 0, { 73, 80, 54, 94, 90, 52, 69, 58 } ); dData.push_back( aux );
}

// There's no file reading in this puzzle - the puzzle input is also hardcoded
void GetData_PUZZLE( DataStream &dData ) { GetData_TEST( dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << "Monkey ID: " << iData.nID;
    cout << ", operation: " << iData.cOperation << " " << iData.nOperand;
    cout << ", divider: " << iData.nDivider << " ? " << iData.nUponTrue << " : " << iData.nUponFalse;
    cout << ", nr of items: " << iData.vItems.size();
    cout << " = { ";
    for (int i = 0; i < (int)iData.vItems.size(); i++) {
        cout << iData.vItems[i] << " ";
    }
    cout << " } " << endl;
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
        case TEST   : GetData_TEST(    dData ); break;
        case PUZZLE : GetData_PUZZLE(  dData ); break;
        default: cout << "ERROR: GetInput() --> unknown program phase: " << glbProgPhase << endl;
    }
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// for the puzzle you need to record how many times
// each monkey makes an inspect of an item
typedef struct sPuzzleStruct {
    int monkeyID;
    long long nrInspects;
} PuzzleType;
vector<PuzzleType> vInspects;

// simulates on turn for one monkey (with index curM)
void MonkeyTurn1( DataStream &mData, int curM ) {
    DatumType &curMonkey = mData[curM];

    while (!curMonkey.vItems.empty()) {
        // 1. inspect next item
        long long curItemVal = curMonkey.vItems.front();
        curMonkey.vItems.pop_front();

        vInspects[curMonkey.nID].nrInspects += 1;

        // 2. apply operation on item
        long long newItemVal;
        switch (curMonkey.cOperation) {
            case '*': newItemVal = curItemVal * curMonkey.nOperand; break;
            case '+': newItemVal = curItemVal + curMonkey.nOperand; break;
            case '^': newItemVal = curItemVal * curItemVal; break;
            default : newItemVal = -1;
        }

        // 3. get bored with item
        newItemVal = (long long)( float( newItemVal ) / 3.0f );

        // 4. perform test
        bool bTestResult = (newItemVal % curMonkey.nDivider == 0);

        // 5. throw item to another monkey
        int nMonkeyToThrowTo = (bTestResult ? curMonkey.nUponTrue : curMonkey.nUponFalse);
        mData[ nMonkeyToThrowTo ].vItems.push_back( newItemVal );
    }
}

// let each monkey - in order - have it's turn
void MonkeyRound1( DataStream &mData ) {
    for (int i = 0; i < (int)mData.size(); i++) {
        MonkeyTurn1( mData, i );
    }
}

// part 2 ----------

// the idea is to encapsulate the calculation in a modular space, by applying a global
// modulus factor on each outcome. This factor is determined by multiplying all dividers for
// all monkeys. This way the numbers are kept within a reasonable bound, while all the division
// tests are not impacted... :)
long long glbModulusFactor = -1;

void InitModulusFactor( DataStream &iData ) {
    glbModulusFactor = 1;
    for (int i = 0; i < (int)iData.size(); i++) {
        glbModulusFactor *= iData[i].nDivider;
    }
}

// this is a variant of MonkeyTurn1() where all calculation are kept in modular space
void MonkeyTurn2( DataStream &mData, int curM ) {
    DatumType &curMonkey = mData[curM];

    while (!curMonkey.vItems.empty()) {
        // 1. inspect next item
        long long curItemVal = curMonkey.vItems.front();
        curMonkey.vItems.pop_front();

        vInspects[curMonkey.nID].nrInspects += 1;

        // 2. apply operation on item
        long long newItemVal;
        switch (curMonkey.cOperation) {
            case '*': newItemVal = curItemVal * curMonkey.nOperand; break;
            case '+': newItemVal = curItemVal + curMonkey.nOperand; break;
            case '^': newItemVal = curItemVal * curItemVal; break;
            default : newItemVal = -1;
        }
        // make sure to stay within modular math
        newItemVal = newItemVal % glbModulusFactor;

        // 4. perform test
        bool bTestResult = (newItemVal % curMonkey.nDivider == 0);

        // 5. throw item to another monkey
        int nMonkeyToThrowTo = (bTestResult ? curMonkey.nUponTrue : curMonkey.nUponFalse);
        mData[ nMonkeyToThrowTo ].vItems.push_back( newItemVal );
    }
}

// let each monkey - in order - have it's turn
void MonkeyRound2( DataStream &mData ) {
    for (int i = 0; i < (int)mData.size(); i++) {
        MonkeyTurn2( mData, i );
    }
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

    // initialize / clear the inspect count list
    vInspects.resize( inputData.size());
    for (int i = 0; i < (int)vInspects.size(); i++) {
        vInspects[i].monkeyID = i;
        vInspects[i].nrInspects = 0;
    }

    for (int i = 0; i < 20; i++) {
        MonkeyRound1( inputData );
//        cout << "Results after round " << i + 1 << endl << endl;
//        PrintDataStream( inputData );
    }

    // output and then sort all inspection counts per monkey
//    for (int i = 0; i < (int) vInspects.size(); i++) {
//        cout << "Monkey " << i << " inspected items " << vInspects[i].nrInspects << " times." << endl;
//    }
    sort( vInspects.begin(), vInspects.end(),
        []( PuzzleType a, PuzzleType b ) {
            return a.nrInspects > b.nrInspects;
        }
    );
    // report the answer - the two highest inspection counts multiplied
    cout << endl << "Answer 1 - level of monkey business: " << vInspects[0].nrInspects * vInspects[1].nrInspects << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // re-read inputdata
    inputData.clear();
    GetInput( inputData, false );
    // init glbModulusFactor to enable working in modular space
    InitModulusFactor( inputData );
//    cout << "Factor to work within modular math = " << glbModulusFactor << endl;

    // clear vInspects vector
    for (int i = 0; i < (int)vInspects.size(); i++) {
        vInspects[i].monkeyID = i;
        vInspects[i].nrInspects = 0;
    }

    // output only at these triggers
//    vector<int> triggers = { 1, 20, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000 };
    vector<int> triggers = {  };
    // collect inspection counts per monkey for 10000 rounds
    for (int i = 1; i <= 10000; i++) {
        MonkeyRound2( inputData );

        if (find( triggers.begin(), triggers.end(), i ) != triggers.end()) {
            cout << "Results after round " << i << endl;
            cout << "=================== " <<      endl;
//            PrintDataStream( inputData );
            for (int j = 0; j < (int) vInspects.size(); j++) {
                cout << "Monkey " << j << " inspected items " << vInspects[j].nrInspects << " times." << endl;
            }
        }
    }
    // sort the inspection count list to get the highest two values
    sort( vInspects.begin(), vInspects.end(),
        []( PuzzleType a, PuzzleType b ) {
            return a.nrInspects > b.nrInspects;
        }
    );
    // report the answer - the two highest inspection counts multiplied
    cout << endl << "Answer 2 - level of monkey business: " << vInspects[0].nrInspects * vInspects[1].nrInspects << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
