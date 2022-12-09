// AoC 2022 - day 08 - Treetop Tree House
// ======================================

// date:  2022-12-08
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

// the data comprises a map of trees, modelled as a vector of strings
typedef string DatumType;
typedef vector<DatumType> DataStream;

int glbMapSizeX, glbMapSizeY;      // to capture map sizes after initialisation

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - focus on getting the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData.push_back( "30373" );
    dData.push_back( "25512" );
    dData.push_back( "65332" );
    dData.push_back( "33549" );
    dData.push_back( "35390" );
}

// file input - this function reads text file one line at a time - adapt code to match your need for line parsing!
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

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   OUTPUT FUNCTIONS

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
        default: cout << "ERROR: GetInput() --> program phase not recognized: " << glbProgPhase << endl;
    }
    // derive dimensions of the map from the - now filled - datastructure
    glbMapSizeY = (int)dData.size();
    glbMapSizeX = (glbMapSizeY > 0) ? (int)dData[0].length() : 0;
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// for convenient addressing of the map
char GetTree( DataStream &dData, int mapX, int mapY ) { return dData[mapY][mapX]; }

// a tree is visible if it can be seen from any of it's four sides. It can be seen if all the adjacent trees in
// that direction are lower. In other words it is invisible when it's blocked from four all sides
bool IsVisible( DataStream &dData, int mapX, int mapY ) {
    // every tree along the edges of the map is visible (by definition)
    bool result = mapX == 0 || mapY == 0 || mapX == glbMapSizeX - 1 || mapY == glbMapSizeY - 1;
    if (!result) {
        char treeHeight = GetTree( dData, mapX, mapY );
                // check for each direction whether the view is blocked
        bool BlckLt = false; for (int x = mapX - 1; x >=           0 && !BlckLt; x--) { BlckLt = GetTree( dData,    x, mapY ) >= treeHeight; }   // check West
        bool BlckRt = false; for (int x = mapX + 1; x <  glbMapSizeX && !BlckRt; x++) { BlckRt = GetTree( dData,    x, mapY ) >= treeHeight; }   //       East
        bool BlckUp = false; for (int y = mapY - 1; y >=           0 && !BlckUp; y--) { BlckUp = GetTree( dData, mapX,    y ) >= treeHeight; }   //       North
        bool BlckDn = false; for (int y = mapY + 1; y <  glbMapSizeY && !BlckDn; y++) { BlckDn = GetTree( dData, mapX,    y ) >= treeHeight; }   //       South
        // trees are invisible only if blocked by *all* sides
        result = !(BlckLt && BlckRt && BlckUp && BlckDn);
    }
    return result;
}

// The scenic score is the product of the view distances of all four sides. A view distance is the nr of tree's that are visible, including any blocking tree.
// Trees at the edge of the map have at least one view distance of 0, so their scenic score will be 0.
int ScenicScore( DataStream &dData, int mapX, int mapY ) {
    int nScore = -1;
    if (mapX == 0 || mapY == 0 || mapX == glbMapSizeX - 1 || mapY == glbMapSizeY - 1) {
        nScore = 0; // since one of the viewing distances is 0, the scenic score is 0
    } else {
        char treeHeight = GetTree( dData, mapX, mapY );
        // increase viewing distance (per direction) until you find that the view is blocked
        bool BlckLt = false; int VDLt = 0; for (int x = mapX - 1; x >=           0 && !BlckLt; x--) { BlckLt = GetTree( dData,    x, mapY ) >= treeHeight; VDLt += 1; }
        bool BlckRt = false; int VDRt = 0; for (int x = mapX + 1; x <  glbMapSizeX && !BlckRt; x++) { BlckRt = GetTree( dData,    x, mapY ) >= treeHeight; VDRt += 1; }
        bool BlckUp = false; int VDUp = 0; for (int y = mapY - 1; y >=           0 && !BlckUp; y--) { BlckUp = GetTree( dData, mapX,    y ) >= treeHeight; VDUp += 1; }
        bool BlckDn = false; int VDDn = 0; for (int y = mapY + 1; y <  glbMapSizeY && !BlckDn; y++) { BlckDn = GetTree( dData, mapX,    y ) >= treeHeight; VDDn += 1; }
        // scenic score is product of all viewing distances
        nScore = VDLt * VDRt * VDUp * VDDn;
    }
    return nScore;
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream mapData;
    GetInput( mapData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << mapData.size() << endl << endl;

    tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

// ========== part 1

    // check for each tree if it's visible and accumulate the nr of visible trees
    int nrVisible = 0;
    for (int y = 0; y < glbMapSizeY; y++) {
        for (int x = 0; x < glbMapSizeX; x++) {
            bool bIsVisible = IsVisible( mapData, x, y );
            if (glbProgPhase != PUZZLE)
                cout << "Tree at " << x << ", " << y << " with height: " << GetTree( mapData, x, y ) << " is " << (bIsVisible ? "" : "NOT") << " visible" << endl;
            if (bIsVisible)
                nrVisible += 1;
        }
    }
    cout << endl << "Answer 1 - nr of visible trees: " << nrVisible << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

// ========== part 2

    // scan the map to find the tree with the highest scenic score
    int foundX = -1, foundY = -1, maxScore = -1;
    for (int y = 0; y < glbMapSizeY; y++) {
        for (int x = 0; x < glbMapSizeX; x++) {
            int localScore = ScenicScore( mapData, x, y );
            if (localScore > maxScore) {
                maxScore = localScore;
                foundX = x;
                foundY = y;
            }
        }
    }
    cout << endl << "Answer 2 - max score is " << maxScore << " and is found at: " << foundX << ", " << foundY << endl << endl;

    return 0;
}
