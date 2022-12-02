// AoC 2022 - day 01 - Calorie Counting
// ====================================

// date:  2022-12-01
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

//typedef vector<int>       DatumType;
typedef struct elfStruct {       // total calorie value was practical for part 2 (sorting)
    vector<int> vCalList;
    int nTotal;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    aux.vCalList.clear();
    aux.vCalList.push_back(1000);
    aux.vCalList.push_back(2000);
    aux.vCalList.push_back(3000);
    dData.push_back( aux );

    aux.vCalList.clear();
    aux.vCalList.push_back(4000 );
    dData.push_back( aux );

    aux.vCalList.clear();
    aux.vCalList.push_back(5000 );
    aux.vCalList.push_back(6000 );
    dData.push_back( aux );

    aux.vCalList.clear();
    aux.vCalList.push_back(7000 );
    aux.vCalList.push_back(8000 );
    aux.vCalList.push_back(9000 );
    dData.push_back( aux );

    aux.vCalList.clear();
    aux.vCalList.push_back(10000 );
    dData.push_back( aux );
}

// this function assumes 1 line to contain 1 token
void ReadInputData( const string sFileName, DataStream &vData ) {

    ifstream dataFileStream( sFileName );
    vData.clear();
    string token;
    DatumType datum;

    while (getline( dataFileStream, token )) {
        if (token.length() == 0) { // empty line
            vData.push_back( datum );
            datum.vCalList.clear();
        } else {
            int nVal = atoi( token.c_str() );
            datum.vCalList.push_back( nVal );
        }
    }
    if (datum.vCalList.size() > 0)
        vData.push_back( datum );

    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    for (auto e : iData.vCalList) {
        cout << e << endl;
    }
    cout << endl;
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
        default: cout << "ERROR: GetInput() --> program phase not recognized: " << glbProgPhase;
    }

    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// none here - it's all implemented in main

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream elfData;
    GetInput( elfData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << elfData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    // for all elves in the list, accumulate the calorie weight in it's nTotal field
    // at the same time, select the max calorie weight from the list
    int maxNrCalories = -1;
    int elfIndex = -1;
    for (int i = 0; i < (int)elfData.size(); i++) {
        int sum = 0;
        for (int j = 0; j < (int)elfData[i].vCalList.size(); j++) {
            sum += elfData[i].vCalList[j];
        }
        elfData[i].nTotal = sum;
        if (sum > maxNrCalories) {
            maxNrCalories = sum;
            elfIndex = i;
        }
    }
    cout << endl << "Answer 1 - elf: " << elfIndex << " carries most calories, being: " << maxNrCalories << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    // sort the list of elves in decreasing order of accumulated calorie weight
    sort( elfData.begin(), elfData.end(),
        []( DatumType d1, DatumType d2 ) {
            return d1.nTotal > d2.nTotal;
        }
    );
    // calc. and report the weight of the three heaviest elves
    int nAnswer2 = 0; for (int i = 0; i < 3; i++) nAnswer2 += elfData[i].nTotal;
    cout << endl << "Answer 2 - total nr of calories for heaviest three elves: " << nAnswer2 << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}





