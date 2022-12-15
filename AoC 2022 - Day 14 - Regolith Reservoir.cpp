// AoC 2022 - day 14 - Regolith Reservoir
// ======================================

// date:  2022-12-14
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"    // need this for vi2d (coordinate) type

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of 'lines' having a first and second point (coordinates)
typedef struct datumStruct {
    vi2d point1, point2;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData.push_back( { vi2d( 498,4 ), vi2d( 498,6 ) } );
    dData.push_back( { vi2d( 498,6 ), vi2d( 496,6 ) } );

    dData.push_back( { vi2d( 503,4 ), vi2d( 502,4 ) } );
    dData.push_back( { vi2d( 502,4 ), vi2d( 502,9 ) } );
    dData.push_back( { vi2d( 502,9 ), vi2d( 494,9 ) } );
}

// Cuts of and returns the front token from "input_to_be_adapted", using "delim" as delimiter.
// If delimiter is not found, the complete input string is passed as a token.
// The input string becomes shorter as a result, and may even become empty
string get_token_dlmtd( const string &delim, string &input_to_be_adapted ) {
    size_t splitIndex = input_to_be_adapted.find( delim );
    string token = input_to_be_adapted.substr( 0, splitIndex );
    input_to_be_adapted = (splitIndex == string::npos) ? "" : input_to_be_adapted.substr( splitIndex + delim.length());

    return token;
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;

    while (getline( dataFileStream, sLine )) {

        // 1. read all coords into a temp. vector
        vector<vi2d> coords;
        while (sLine.length() > 0) {
            string sCurCoord = get_token_dlmtd( " -> ", sLine );
            string sCurX = get_token_dlmtd( ",", sCurCoord );
            string sCurY =                       sCurCoord;
            vi2d aux = { atoi( sCurX.c_str()), atoi( sCurY.c_str()) };
            coords.push_back( aux );
        }
        // 2. process the vector making lines of each pair of coordinates
        for (int i = 1; i < (int)coords.size(); i++) {
            vData.push_back( { coords[i - 1], coords[i] } );
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << "point 1: " << iData.point1.ToString() << ", point 2: " << iData.point2.ToString() << endl;
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

#define EMPTY '.'     // different kinds of material
#define SRCE  '+'
#define SAND  'o'
#define ROCK  '#'

// Because I expect the map to be huge, I modeled it as a list of cells that are not empty.
// Each cell has a coordinate, and an associated material
typedef struct sCellStruct {
    vi2d coord;
    char material = EMPTY;    // one of EMPTY, SRCE, SAND or ROCK
} CellType;
typedef vector<CellType> CellsType;

CellsType mapCells;

int lowRockLevel;    // to contain the max (= lowest) y coordinate of all blockades
int floorLevel;      // needed for part 2

// Searches the map (in global variable mapCells) for the lowest rock cell.
// Note that this is the highest y value since the y increases going downwards.
int GetLowestRock() {
    int curMax = INT_MIN;
    for (int i = 0; i < (int)mapCells.size(); i++) {
        if (mapCells[i].coord.y > curMax) {
            curMax = mapCells[i].coord.y;
        }
    }
    return curMax;
}

// Uses dData input data stream to create map information. This map info has the form of a list
// (vector) of cells that are occupied by some material (typically ROCK to start with
// later also SAND)
void ProcessRockInputData( DataStream &dData ) {
    for (int i = 0; i < (int)dData.size(); i++) {
        // process lines in turn
        vi2d p1 = dData[i].point1;
        vi2d p2 = dData[i].point2;
        if (p1.x == p2.x) {     // vertical line
            int increment = (p1.y < p2.y ? +1 : -1);
            bool bDone = false;
            for (int y = p1.y; !bDone; y += increment) {
               mapCells.push_back( { vi2d( p1.x, y ), ROCK } );
               bDone = (y == p2.y);
            }
            mapCells.push_back( { vi2d( p1.x, p2.y ), ROCK } );
        } else {               // horizontal line
            int increment = (p1.x < p2.x ? +1 : -1);
            bool bDone = false;
            for (int x = p1.x; !bDone; x += increment) {
                mapCells.push_back( { vi2d( x, p1.y ), ROCK } );
                bDone = (x == p2.x);
             }
        }
    }
}

// ==========  the code below is for displaying the map in a visual way  ========

vector<string> tstMap;
vi2d tstMapOrg = { 494, 0 };
vi2d sandSource = { 500, 0 };

char GetMap( int x, int y ) {
    return tstMap[ y - tstMapOrg.y ][ x - tstMapOrg.x ];
}

void SetMap( int x, int y, char c ) {
    tstMap[ y - tstMapOrg.y ][ x - tstMapOrg.x ] = c;
}

char GetMap( vi2d coord ) { return GetMap( coord.x, coord.y ); }
void SetMap( vi2d coord, char c ) { SetMap( coord.x, coord.y, c ); }

void DisplayTestMap() {

    // 1. first create an empty map in tstMap
    tstMap.clear();
    for (int i = 0; i < 10; i++) {
        tstMap.push_back( ".........." );
    }
    // 2. place the materials from the list of cells
    for (int i = 0; i < (int)mapCells.size(); i++) {
        SetMap( mapCells[i].coord, mapCells[i].material );
    }
    // 3. place the source of the sand
    SetMap( sandSource, '+' );
    // 4. display the map
    for (int y = 0; y < (int)tstMap.size(); y++) {
        for (int x = 0; x < (int)tstMap[y].length(); x++) {
            cout << tstMap[y][x];
        }
        cout << endl;
    }
}

// Returns true if the coord position is "occupied"
bool IsOccupied( vi2d coord ) {
    bool result = false;
    for (int i = 0; i < (int)mapCells.size() && !result; i++) {
        result = (coord == mapCells[i].coord);
    }
    return result;
}
bool IsOccupied( int x, int y ) { return IsOccupied( vi2d( x, y )); }

// run one new unit of sand from the source, following the rules of sand movement,
// until the unit of sand "comes to rest" or "goes into the void".
// Returns true if the sand unit goes into the void.
bool RunAdditionalSandUnit() {

    vi2d unit = sandSource;
    bool done = false;
    bool bVoid = false;
    while (!done && !bVoid) {
        // check if straight down is possible
        if (!IsOccupied( unit + vi2d( 0, 1 ))) {
            unit += vi2d( 0, 1 );
        } else if (!IsOccupied( unit + vi2d( -1, 1 ))) {   // try diagonal left
            unit += vi2d( -1, 1 );
        } else if (!IsOccupied( unit + vi2d( +1, 1 ))) {   // try diagonal right
            unit += vi2d( +1, 1 );
        } else {  // all three options are blocked - unit comes to rest
            done = true;
            mapCells.push_back( { unit, SAND } );
        }
        // if sand unit is below (y value is larger than) lowest blockade, it's in the void
        bVoid = (unit.y > lowRockLevel);
    }
    return bVoid;
}

// Very similar to previous function, but this time a check is done (and true is returned)
// if the sand source location is blocked.
bool RunAdditionalSandUnit2() {

    vi2d unit = sandSource;
    bool done = false;
    bool bSourceBlocked = false;
    while (!done && !bSourceBlocked) {
        // check if straight down is possible
        if (!IsOccupied( unit + vi2d( 0, 1 ))) {
            unit += vi2d( 0, 1 );
        } else if (!IsOccupied( unit + vi2d( -1, 1 ))) {   // try diagonal left
            unit += vi2d( -1, 1 );
        } else if (!IsOccupied( unit + vi2d( +1, 1 ))) {   // try diagonal right
            unit += vi2d( +1, 1 );
        } else {  // all three options are blocked - unit comes to rest
            done = true;
            mapCells.push_back( { unit, SAND } );
        }
        if (unit.y == floorLevel - 1) {    // unit gets to rest on virtual floor
            done = true;
            mapCells.push_back( { unit, SAND } );
        }
        bSourceBlocked = (unit == sandSource);
    }
    return bSourceBlocked;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = EXAMPLE;     // program phase to EXAMPLE, TEST or PUZZLE
    flcTimer tmr;

/* ========== */   tmr.StartTiming();   // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream inputData;
    GetInput( inputData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << inputData.size() << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

    // part 1 code here

    ProcessRockInputData( inputData );
    lowRockLevel = GetLowestRock();

    mapCells.push_back( { sandSource, SRCE } );
    if (glbProgPhase != PUZZLE)
        DisplayTestMap();

    int nAnswer1 = -1;
    bool bStop = false;
    for (int i = 1; !bStop; i++) {
        bool GoesToVoid = RunAdditionalSandUnit();
        if (GoesToVoid) {
            cout << "This unit (and all following ones) disappears into void: " << i << endl;
            nAnswer1 = i - 1;
            bStop = true;
        }
        if (glbProgPhase != PUZZLE) {
            if (i == 1 || i == 2 || i == 5 || i == 22 || i == 24) {
                cout << "after adding units: " << i << endl << endl;
                DisplayTestMap();
            }
        }
    }
    cout << endl << "Answer 1 - sand units getting to rest: " << nAnswer1 << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here
    mapCells.clear();
    ProcessRockInputData( inputData );
    lowRockLevel = GetLowestRock();
    floorLevel = lowRockLevel + 2;

    mapCells.push_back( { sandSource, SRCE } );
    if (glbProgPhase != PUZZLE)
        DisplayTestMap();

    int nAnswer2 = -1;
    bStop = false;
    for (int i = 1; !bStop; i++) {
        bool bSourceBlock = RunAdditionalSandUnit2();
        if (bSourceBlock) {
            cout << "This unit has blocked the sand source: " << i << endl;
            nAnswer2 = i;
            bStop = true;
        }
    }
    cout << endl << "Answer 2 - units getting to rest: " << nAnswer2 << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
