// AoC 2022 - day 15 - Beacon Exclusion Zone
// =========================================

// date:  2022-12-15
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"   // for all the coordinate (vllong2d type)

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of pairs of 'sensor and beacon' coordinates
typedef struct datumStruct {
    vllong2d sensor, beacon;
    long long dist = -1;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {

    dData.push_back( { vllong2d(  2, 18 ), vllong2d( -2, 15 ) } );
    dData.push_back( { vllong2d(  9, 16 ), vllong2d( 10, 16 ) } );
    dData.push_back( { vllong2d( 13,  2 ), vllong2d( 15,  3 ) } );
    dData.push_back( { vllong2d( 12, 14 ), vllong2d( 10, 16 ) } );
    dData.push_back( { vllong2d( 10, 20 ), vllong2d( 10, 16 ) } );
    dData.push_back( { vllong2d( 14, 17 ), vllong2d( 10, 16 ) } );
    dData.push_back( { vllong2d(  8,  7 ), vllong2d(  2, 10 ) } );
    dData.push_back( { vllong2d(  2,  0 ), vllong2d(  2, 10 ) } );
    dData.push_back( { vllong2d(  0, 11 ), vllong2d(  2, 10 ) } );
    dData.push_back( { vllong2d( 20, 14 ), vllong2d( 25, 17 ) } );
    dData.push_back( { vllong2d( 17, 20 ), vllong2d( 21, 22 ) } );
    dData.push_back( { vllong2d( 16,  7 ), vllong2d( 15,  3 ) } );
    dData.push_back( { vllong2d( 14,  3 ), vllong2d( 15,  3 ) } );
    dData.push_back( { vllong2d( 20,  1 ), vllong2d( 15,  3 ) } );
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

// Cuts of and returns a token of size nSize from the front of input_to_be_adapted.
// This input string becomes shorter as a result of this cut off, and may even become empty.
string get_token_sized( const int nSize, string &input_to_be_adapted ) {
    string token;
    token = input_to_be_adapted.substr( 0, nSize );
    input_to_be_adapted = (nSize == (int)string::npos) ? "" : input_to_be_adapted.substr( nSize );
    return token;
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;
    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            string sIgnore  = get_token_sized( 12, sLine );      // just cut of the "Sensor at x=" part
            string sSensorX = get_token_dlmtd( ", y=", sLine );
            string sSensorY = get_token_dlmtd( ": closest beacon is at x=", sLine );
            string sBeaconX = get_token_dlmtd( ", y=", sLine );
            string sBeaconY = sLine;
            datum.sensor = vllong2d( atoll( sSensorX.c_str()), atoll( sSensorY.c_str()));
            datum.beacon = vllong2d( atoll( sBeaconX.c_str()), atoll( sBeaconY.c_str()));
            vData.push_back( datum );
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << "Sensor at: " << iData.sensor.ToString() << ", close beacon at: " << iData.beacon.ToString();
    cout << " distance from sensor to close beacon: " << iData.dist << endl;
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

void InitDistances( DataStream &dData ) {

    auto manhattan_distance = [=]( vllong2d a, vllong2d b ) {
        return abs( a.x - b.x ) + abs( a.y - b.y );
    };

    for (int i = 0; i < (int)dData.size(); i++) {
        dData[i].dist = manhattan_distance( dData[i].sensor, dData[i].beacon );
    }
}

// a line part is an interval along a (horizontal) line
typedef struct sLinePartStruct {
    long long x1, x2;  // and including x2
} LinePartType;

// a line has a constant y coordinate, and can contain multiple line parts
typedef struct sLineStruct {
    long long y;
    vector<LinePartType> lineParts;
} LineType;

// overlapping and contiguous line parts are resolved in such a way that the minimal
// number of line parts remains
void OptimizeLine( LineType &l ) {
    // first sort the line parts for increasing x1
    sort( l.lineParts.begin(), l.lineParts.end(),
        []( LinePartType a, LinePartType b ) {
            return a.x1 < b.x1;
        }
    );
    // then check if lineparts can be combined
    vector<LinePartType> newVector;
    LinePartType curLP = l.lineParts[0];
    for (int i = 1; i < (int)l.lineParts.size(); i++) {
        if (l.lineParts[i].x1 <= curLP.x2 + 1) {
            if (l.lineParts[i].x2 > curLP.x2)
            // overlap, so combine
                curLP.x2 = l.lineParts[i].x2;
        } else {
            newVector.push_back( curLP );
            curLP = l.lineParts[i];
        }
    }
    newVector.push_back( curLP );
    // now replace the old line parts vector with the new one
    l.lineParts = newVector;
}

// Returns true if the line modelled by y intersects with the manhattan distance area modeled by sensor and dist.
// If it intersects, x1 and x2 will have the horizontal intersection coordinates
bool Line_ManhattanArea_Intersection( vllong2d sensor, long long dist, long long y, long long &x1, long long &x2 ) {

    bool bResult = false;
    x1 = -1;
    x2 = -1;
    if (sensor.y - dist <= y && y <= sensor.y + dist) {
        bResult = true;
        if (y <= sensor.y) {
            x1 = sensor.x - (y - (sensor.y - dist));
            x2 = sensor.x + (y - (sensor.y - dist));
        } else {
            x1 = sensor.x - ((sensor.y + dist) - y);
            x2 = sensor.x + ((sensor.y + dist) - y);
        }
    }
    return bResult;
}

// Creates a list of unique beacons from the input data stream iData
void GetUniqueBeaconList( DataStream &iData, vector<vllong2d> &beacons ) {
    beacons.clear();
    // build up beacon list from input data stream
    for (auto e : iData) {
        beacons.push_back( e.beacon );
    }
    // sort is needed to have unique() function correctly
    sort( beacons.begin(), beacons.end());
    // remove duplicates
    beacons.erase( unique( beacons.begin(), beacons.end()), beacons.end());
}

// Returns true if point b is on any of the line parts of line l
bool Overlap( LineType &l, vllong2d b ) {
    bool bResult = false;
    if (l.y == b.y) {
        for (int i = 0; i < (int)l.lineParts.size() && !bResult; i++) {
            bResult = (l.lineParts[i].x1 <= b.x && b.x <= l.lineParts[i].x2);
        }
    }
    return bResult;
}

// Line parts are truncated so that they are guaranteed to be witin interval [ a, b ]
void TruncateLine( LineType &l, long long a, long long b ) {
    vector<LinePartType> newVector;

    int cntr = 0;
    // skip all line parts that are completely out of bounds to the left
    while (cntr < (int)l.lineParts.size() && l.lineParts[cntr].x2 < a) {
        cntr += 1;
    }
    // adapt line part that overlaps left boundary
    if (cntr < (int)l.lineParts.size() && l.lineParts[cntr].x1 < a) {
        l.lineParts[cntr].x1 = a;
    }
    if (cntr < (int)l.lineParts.size() && l.lineParts[cntr].x2 > b) {
        l.lineParts[cntr].x2 = b;
    }
    newVector.push_back( l.lineParts[cntr] );
    cntr += 1;
    // copy all line parts that are completely within bounds a - b
    while (cntr < (int)l.lineParts.size() && l.lineParts[cntr].x2 <= b) {
        newVector.push_back( l.lineParts[cntr] );
        cntr += 1;
    }
    // adapt line part that overlaps right boundary
    if (cntr < (int)l.lineParts.size() && l.lineParts[cntr].x1 < b) {
        l.lineParts[cntr].x2 = b;
        newVector.push_back( l.lineParts[cntr] );
    }

    // now replace line parts vector with new
    l.lineParts = newVector;
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
    InitDistances( inputData );

    long long lineY = (glbProgPhase != PUZZLE ? 10 : 2000000);
    LineType curLine;
    curLine.y = lineY;
    // 1. determine all intersections for all input sensors with the line of interest,
    // and collect the data (intersection line parts) in curLine variable
    for (int i = 0; i < (int)inputData.size(); i++) {
        long long tmpX1, tmpX2;
        if (Line_ManhattanArea_Intersection( inputData[i].sensor, inputData[i].dist, lineY, tmpX1, tmpX2 )) {
            // an intersection is found, tmpX1 and tmpX2 contain the horizontal intersection coordinates
            curLine.lineParts.push_back( { tmpX1, tmpX2 } );
        }
    }
    // 2. combine lineparts where possible
    OptimizeLine( curLine );
    // 3. work out the cumulated length of all lineparts
    long long nTotalLength = 0;
    for (int i = 0; i < (int)curLine.lineParts.size(); i++) {
        long long nX1 = curLine.lineParts[i].x1;
        long long nX2 = curLine.lineParts[i].x2;
        long long nLocalLength = nX2 - nX1 + 1;
        nTotalLength += nLocalLength;
    }
    // 4. now get a list of unique beacons
    vector<vllong2d> vBeacons;
    GetUniqueBeaconList( inputData, vBeacons );
    // 5. check how many of them overlap with the line parts under consideration
    long long nOverlappingBeacons = 0;
    for (auto e : vBeacons) {
        if (Overlap( curLine, e )) {
            nOverlappingBeacons += 1;
        }
    }
    // 6. report the answer
    cout << endl << "Answer 1 - accumulated positions: " << nTotalLength;
    cout << " - overlapping beacons: " << nOverlappingBeacons;
    cout << " - final answer = " << nTotalLength - nOverlappingBeacons << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    long long findX = -1;
    long long findY = -1;
    long long boundaryValue = (glbProgPhase == PUZZLE ? 4000000 : 20);

    // iterate over all the lines in the search area
    for (long long lineY = 0; lineY < boundaryValue; lineY++) {
        LineType curLine;
        curLine.y = lineY;
        // determine all intersections for all input sensors with the current line
        // and collect the data (intersection line parts) in curLine variable
        for (int i = 0; i < (int)inputData.size(); i++) {
            long long tmpX1, tmpX2;
            if (Line_ManhattanArea_Intersection( inputData[i].sensor, inputData[i].dist, lineY, tmpX1, tmpX2 )) {
                // an intersection is found, tmpX1 and tmpX2 contain the intersection points
                curLine.lineParts.push_back( { tmpX1, tmpX2 } );
            }
        }
        // combine lineparts where possible
        OptimizeLine( curLine );
        // make the current line fit in [ 0 - boundaryValue ] interval
        TruncateLine( curLine, 0, boundaryValue );
        // cumulate the length of all lineparts
        long long nLineLength = 0;
        for (int i = 0; i < (int)curLine.lineParts.size(); i++) {
            long long nX1 = curLine.lineParts[i].x1;
            long long nX2 = curLine.lineParts[i].x2;
            long long nLocalLength = nX2 - nX1 + 1;
            nLineLength += nLocalLength;
        }
        // check if line is incomplete - in that case there is an empty cell
        if (nLineLength != boundaryValue + 1) {
            findY = lineY;
            findX = curLine.lineParts[0].x2 + 1;
        }
    }
    // calculate and report tuning frequency
    long long tuningFreq = findX * 4000000 + findY;
    cout << endl << "Answer 2 - non occupied cell found at: (" << findX << ", " << findY << ") - ";
    cout << "Tuning Frequency = " << tuningFreq << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
