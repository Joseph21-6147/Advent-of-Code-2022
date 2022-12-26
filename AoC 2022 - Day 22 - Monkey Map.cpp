// AoC 2022 - day 22 - Monkey Map
// ==============================

// date:  2022-12-22
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the input data consists of two parts: first part are 'map lines' modeled as strings
typedef string DatumType;
typedef vector<DatumType> DataStream;

// the input data consists of two parts:
//    * first part is a map layout modeled as a series of strings
//    * second part is 'walk instruction' modeled as a single string
// Both parts are separated by an empty line
typedef string DataStream2;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &mData, string &iData ) {
    mData.push_back( "        ...#"     );
    mData.push_back( "        .#.."     );
    mData.push_back( "        #..."     );
    mData.push_back( "        ...."     );
    mData.push_back( "...#.......#"     );
    mData.push_back( "........#..."     );
    mData.push_back( "..#....#...."     );
    mData.push_back( "..........#."     );
    mData.push_back( "        ...#...." );
    mData.push_back( "        .....#.." );
    mData.push_back( "        .#......" );
    mData.push_back( "        ......#." );

    iData = "10R5L5R10L4R5L5";
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &mData, string &iData ) {
    ifstream dataFileStream( sFileName );
    mData.clear();
    string sLine;
    DatumType datum;

    bool bPart2 = false;
    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            if (!bPart2) {
                mData.push_back( sLine );
            } else {
                iData = sLine;
            }
        } else {
            bPart2 = true;
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData, string &iData ) { ReadInputData( "input.test.txt"  , dData, iData ); }
void GetData_PUZZLE( DataStream &dData, string &iData ) { ReadInputData( "input.puzzle.txt", dData, iData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

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
void GetInput( DataStream &dData, string &iData, bool bDisplay = false ) {

    switch( glbProgPhase ) {
        case EXAMPLE: GetData_EXAMPLE( dData, iData ); break;
        case TEST:    GetData_TEST(    dData, iData ); break;
        case PUZZLE:  GetData_PUZZLE(  dData, iData ); break;
        default: cout << "ERROR: GetInput() --> unknown program phase: " << glbProgPhase << endl;
    }
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// put a belt around the original map to
// sync the coordinates with the puzzle coordinates and
// to easily discover wrap around situations
void ConvertMap( DataStream &mData ) {
    // make a copy of the original map
    DataStream tmp = mData;
    // clear the original map
    mData.clear();
    // determine sizes of the map
    int ySize = (int)tmp.size();
    int xSize;
    if (ySize == 0) {
        xSize = 0;
    } else {
        int longest = -1;
        for (int i = 0; i < (int)tmp.size(); i++) {
            if (longest < (int)tmp[i].length()) longest = tmp[i].length();
        }
        xSize = longest;
    }
    // fill the map with additional border characters
    for (int y = 0; y < ySize + 2; y++) {
        string sLineString = "";
        for (int x = 0; x < xSize + 2; x++) {
            sLineString.append( "=" );
        }
        mData.push_back( sLineString );
    }
    // now project the original map in it
    for (int y = 0; y < ySize; y++) {
        for (int x = 0; x < xSize; x++) {
            if (x < (int)tmp[y].length()) {
                mData[y+1][x+1] = tmp[y][x];
            } else {
                mData[y+1][x+1] = ' ';
            }
        }
    }
}

// instruction: walk a nr of steps and then rotate left or right
typedef struct sTokenStruct {
    int nrSteps;
    char rotDir;
} TokenType;
typedef vector<TokenType> TokenStream;

// convenience function for parsing
bool IsNumeric( char c ) { return '0' <= c && c <= '9'; }

// parse the instruction stream in iData into a stream of separate instructions in tData
void ParseInstructions( string &iData, TokenStream &tData ) {
    tData.clear();
    int cntr = 0;
    while (cntr < (int)iData.length()) {
        string nrString = "";
        while (cntr < (int)iData.length() && IsNumeric( iData[cntr] )) {
            nrString.append( string( 1, iData[cntr] ));
            cntr += 1;
        }
        char cDir = ' ';
        if (cntr < (int)iData.length()) {
            cDir = iData[cntr];
            cntr += 1;
        }

        TokenType curToken = { atoi( nrString.c_str()), cDir };
        tData.push_back( curToken );
    }
}

#define EAST  0
#define SOUTH 1
#define WEST  2
#define NORTH 3

string DirToString( int dir ) {
    switch( dir ) {
        case EAST:  return "EAST";
        case WEST:  return "WEST";
        case NORTH: return "NORTH";
        case SOUTH: return "SOUTH";
    }
    return "dir-shite!";
}

// display map with player position and direction in it
void ShowMap( DataStream &mData, int posX, int posY, int dir ) {
    DataStream tmp = mData;
    char cPlayer;
    switch (dir) {
        case EAST : cPlayer = '>'; break;
        case WEST : cPlayer = '<'; break;
        case NORTH: cPlayer = '^'; break;
        case SOUTH: cPlayer = 'v'; break;
        default: cout << "ERROR: ShowMap() --> unknown dir value: " << dir << endl;
    }
    tmp[posY][posX] = cPlayer;
    PrintDataStream( tmp );
}

// simulation for 1 instruction (part 1)
void MoveStep1( DataStream &mData, int &posX, int &posY, int &dir, int nrSteps, char cNewDir ) {

    // get initial variables
    int newX = posX, newY = posY;
    int sizeY = (int)mData.size();
    int sizeX = (sizeY == 0 ? 0 : (int)mData[0].length());

    // move nr of steps in direction
    bool isBlocked = false;
    for (int i = 0; i < nrSteps && !isBlocked; i++) {
        // check out to be position and direction
        switch (dir) {
            case EAST : newX += 1; break;
            case WEST : newX -= 1; break;
            case SOUTH: newY += 1; break;
            case NORTH: newY -= 1; break;
        }
        // what is to be position and what action is needed?
        switch (mData[newY][newX]) {
            case '=':
            case ' ': {  // wrap around is needed if possible
                    // find first valid map position after wrap
                    bool found = false;
                    switch (dir) {
                        case EAST : for (int k =         1; k <  sizeX && !found; k++) { found = mData[newY][k] != ' '; if (found) newX = k; } break;
                        case WEST : for (int k = sizeX - 2; k >=     0 && !found; k--) { found = mData[newY][k] != ' '; if (found) newX = k; } break;
                        case SOUTH: for (int k =         1; k <  sizeY && !found; k++) { found = mData[k][newX] != ' '; if (found) newY = k; } break;
                        case NORTH: for (int k = sizeY - 2; k >=     0 && !found; k--) { found = mData[k][newX] != ' '; if (found) newY = k; } break;
                        default: cout << "ERROR: MoveStep() --> unknown dir value: " << dir << endl;
                    }
                    if (mData[newY][newX] == '.') {
                        posX = newX;
                        posY = newY;
                    } else if (mData[newY][newX] == '#') {
                        isBlocked = true;
                    }
                }
                break;
            case '.': {  // regular move is possible
                    posX = newX;
                    posY = newY;
                }
                break;
            case '#':    // move is blocked: do nothing, ignore remainder of instruction
                isBlocked = true;
                break;
            default: cout << "ERROR: MoveStep1() --> unknown map value: " << mData[newY][newX] << endl;
        }
    }
    switch (cNewDir) {
        case 'R': dir = (dir     + 1) % 4; break;
        case 'L': dir = (dir + 4 - 1) % 4; break;
        case ' ': break;  // this is possible at the end of an instruction stream
        default: cout << "ERROR: MoveStep1() --> unknown cNewDir value: " << cNewDir << endl;
    }
}

// part 2 stuff ----------

#include "vector_types.h"

// defines a mapping from one point/direction combination to another point/direction combination
// this is needed for part 2
// The idea being ofc that if you are on the one cell, and taking 1 step into dir1, you will arrive at the
// other cell and your direction has changed to dir2
typedef struct sMappingStruct {
    vi2d pt1;
    int dir1;
    vi2d pt2;
    int dir2;
} MappingNode;
typedef vector<MappingNode> MappingType;

MappingType testMapping;

// adds mappings for all cells from one side to another, and the other way around.
// the first side is given by corner points pt1a and pt1b (and dir1) the other side is given by
// points pt2a and pt2b (and dir2)
void AddMapping( MappingType &mapping, vi2d pt1a, vi2d pt1b, int dir1, vi2d pt2a, vi2d pt2b, int dir2, int nrIterations ) {
    int dx1, dy1, dx2, dy2;
    if (pt1a.x == pt1b.x) dx1 = 0; else dx1 = (pt1a.x > pt1b.x ? -1 : +1);
    if (pt1a.y == pt1b.y) dy1 = 0; else dy1 = (pt1a.y > pt1b.y ? -1 : +1);
    if (pt2a.x == pt2b.x) dx2 = 0; else dx2 = (pt2a.x > pt2b.x ? -1 : +1);
    if (pt2a.y == pt2b.y) dy2 = 0; else dy2 = (pt2a.y > pt2b.y ? -1 : +1);

    int curx1 = pt1a.x, cury1 = pt1a.y;
    int curx2 = pt2a.x, cury2 = pt2a.y;
    MappingNode n;
    for (int i = 0; i < nrIterations; i++) {
        n = { vi2d( curx1, cury1 ),  dir1         , vi2d( curx2, cury2 ),  dir2          }; mapping.push_back( n );
        n = { vi2d( curx2, cury2 ), (dir2 + 2) % 4, vi2d( curx1, cury1 ), (dir1 + 2) % 4 }; mapping.push_back( n );
        curx1 += dx1; cury1 += dy1;
        curx2 += dx2; cury2 += dy2;
    }
}

// function to lookup the index of the mapping from pt/dir
int FindMappingIndex( MappingType &mapping, vi2d pt, int dir ) {
    int result = -1;
    for (int i = 0; i < (int)mapping.size() && result == -1; i++) {
        if (mapping[i].pt1 == pt && mapping[i].dir1 == dir) {
            result = i;
        }
    }
    return result;
}

// this is hardcoded from the test/example cube input
void InitTestMapping( MappingType &tstMapping ) {

    AddMapping( tstMapping, vi2d(  9, 1 ), vi2d( 12, 1 ), NORTH, vi2d(  4, 5 ), vi2d(  1 ,5 ), SOUTH, 4 );
    AddMapping( tstMapping, vi2d(  5, 5 ), vi2d(  8, 5 ), NORTH, vi2d(  9, 1 ), vi2d(  9, 4 ), EAST , 4 );
    AddMapping( tstMapping, vi2d(  1, 8 ), vi2d(  4, 8 ), SOUTH, vi2d( 12,12 ), vi2d(  9,12 ), NORTH, 4 );
    AddMapping( tstMapping, vi2d(  5, 8 ), vi2d(  8, 8 ), SOUTH, vi2d(  9,12 ), vi2d(  9, 9 ), EAST , 4 );
    AddMapping( tstMapping, vi2d( 13, 9 ), vi2d( 16, 9 ), NORTH, vi2d( 12, 8 ), vi2d( 12, 5 ), WEST , 4 );
    AddMapping( tstMapping, vi2d( 13,12 ), vi2d( 16,12 ), SOUTH, vi2d(  1, 8 ), vi2d(  1, 5 ), EAST , 4 );
    AddMapping( tstMapping, vi2d( 12, 1 ), vi2d( 12, 4 ), EAST , vi2d( 16,12 ), vi2d( 16, 9 ), WEST , 4 );

    sort( tstMapping.begin(), tstMapping.end(),
        []( MappingNode a, MappingNode b ) {
            return a.pt1 < b.pt1 || (a.pt1 == b.pt1 && a.dir1 < b.dir1);
        }
    );
}

// this is hardcoded from the puzzle cube input
void InitPuzzleMapping( MappingType &tstMapping ) {

    AddMapping( tstMapping, vi2d( 51, 150), vi2d(100, 150), SOUTH, vi2d( 50, 151), vi2d( 50, 200), WEST , 50 );
    AddMapping( tstMapping, vi2d(101,  50), vi2d(150,  50), SOUTH, vi2d(100,  51), vi2d(100, 100), WEST , 50 );
    AddMapping( tstMapping, vi2d(  1, 101), vi2d( 50, 101), NORTH, vi2d( 51,  51), vi2d( 51, 100), EAST , 50 );
    AddMapping( tstMapping, vi2d(100, 101), vi2d(100, 150), EAST , vi2d(150,  50), vi2d(150,   1), WEST , 50 );
    AddMapping( tstMapping, vi2d(  1, 101), vi2d(  1, 150), WEST , vi2d( 51,  50), vi2d( 51,   1), EAST , 50 );
    AddMapping( tstMapping, vi2d(  1, 200), vi2d( 50, 200), SOUTH, vi2d(101,   1), vi2d(150,   1), SOUTH, 50 );
    AddMapping( tstMapping, vi2d(  1, 151), vi2d(  1, 200), WEST , vi2d( 51,   1), vi2d(100,   1), SOUTH, 50 );

    sort( tstMapping.begin(), tstMapping.end(),
        []( MappingNode a, MappingNode b ) {
            return a.pt1 < b.pt1 || (a.pt1 == b.pt1 && a.dir1 < b.dir1);
        }
    );
}

// This function is a variant of MoveStep1(), that uses the mapping to determine how to walk over the cube
void MoveStep2( DataStream &mData, MappingType &mapping, int &posX, int &posY, int &dir, int nrSteps, char cNewDir ) {

    // get initial variables
    int newX = posX, newY = posY;

    // move nr of steps in direction (unless blocked)
    bool isBlocked = false;
    for (int i = 0; i < nrSteps && !isBlocked; i++) {
        // check out to be position and direction
        switch (dir) {
            case EAST : newX += 1; break;
            case WEST : newX -= 1; break;
            case SOUTH: newY += 1; break;
            case NORTH: newY -= 1; break;
        }
        // what is to be position and what action is needed?
        switch (mData[newY][newX]) {
            case '=':
            case ' ': {  // wrap around is needed if possible

                    // use the mapping to find associated cell and direction
                    int nIndex = FindMappingIndex( mapping, vi2d( posX, posY ), dir );
                    if (nIndex == -1) {
                        cout << "ERROR: MoveStep2() --> can't find cell associated with: " << vi2d( posX, posY).ToString() << " icw dir: " << DirToString( dir ) << endl;
                    } else {
                        newX = mapping[nIndex].pt2.x;
                        newY = mapping[nIndex].pt2.y;
                        if (mData[newY][newX] == '.') {
                            posX = newX;
                            posY = newY;
                            dir = mapping[nIndex].dir2;
                        } else if (mData[newY][newX] == '#') {
                            isBlocked = true;
                        }
                    }
                }
                break;
            case '.': {  // regular move is possible
                    posX = newX;
                    posY = newY;
                }
                break;
            case '#':    // move is blocked: do nothing, ignore remainder of instruction
                isBlocked = true;
                break;
            default: cout << "ERROR: MoveStep2() --> unknown map value: " << mData[newY][newX] << endl;
        }
    }
    switch (cNewDir) {
        case 'R': dir = (dir     + 1) % 4; break;
        case 'L': dir = (dir + 4 - 1) % 4; break;
        case ' ': break;  // this is possible at the end of an instruction stream
        default: cout << "ERROR: MoveStep2() --> unknown cNewDir value: " << cNewDir << endl;
    }
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE
    flcTimer tmr;

/* ========== */   tmr.StartTiming();   // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream mapData;
    string instData;
    GetInput( mapData, instData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << mapData.size() << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 0: " );   // =========================^^^^^vvvvv

    // part 1 code here

    // set the input in the correct map ds and get the instruction in a list (vector)
    ConvertMap( mapData );
    TokenStream instructions;
    ParseInstructions( instData, instructions );

    // set initial position and direction
    int posY =  1; // hardcoded line 1
    int posX = -1; // find first fit column
    for (int i = 0; i < (int)mapData[posY].length() && posX == -1; i++) {
        if (mapData[posY][i] == '.') posX = i;
    }
    int dir = EAST;

    // iterate over instructions list and perform simulation per instruction
    for (int i = 0; i < (int)instructions.size(); i++) {
        TokenType &curInstr = instructions[i];
        if (glbProgPhase != PUZZLE) {
            cout << "processing instruction " << i << endl;
        }
        MoveStep1( mapData, posX, posY, dir, curInstr.nrSteps, curInstr.rotDir );
    }
    // report the final row, column and direction, and derive the answer from it
    cout << endl << "Answer 1 - final row = " << posY << ", final column = " << posX << ", final facing = " << dir << " final password = " << 1000 * posY + 4 * posX + dir << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // create the additional (hardcoded) mappings to convert the 2d map in a 3d cube
    MappingType testMapping;
    if (glbProgPhase == PUZZLE) {
        InitPuzzleMapping( testMapping );
    } else {
        InitTestMapping( testMapping );
    }
    // output the mapping (for testing)
    if (glbProgPhase != PUZZLE) {
        for (int i = 0; i < (int)testMapping.size(); i++) {
            cout << "mapping [ " << i << " ]: from cell " << testMapping[i].pt1.ToString() << " dir " << DirToString( testMapping[i].dir1 )
                                           << " to cell " << testMapping[i].pt2.ToString() << " dir " << DirToString( testMapping[i].dir2 ) << endl;
        }
    }

    // set initial position and direction
    posY =  1; // hardcoded line 1
    posX = -1; // find first fit column
    for (int i = 0; i < (int)mapData[posY].length() && posX == -1; i++) {
        if (mapData[posY][i] == '.') posX = i;
    }
    dir = EAST;

    // iterate over instructions list and perform simulation per instruction
    for (int i = 0; i < (int)instructions.size(); i++) {
        TokenType &curInstr = instructions[i];
        if (glbProgPhase != PUZZLE) {
            cout << "processing instruction " << i << " nr steps= " << instructions[i].nrSteps << ", turn dir= " << instructions[i].rotDir << endl;
        }
        MoveStep2( mapData, testMapping, posX, posY, dir, curInstr.nrSteps, curInstr.rotDir );
    }
    // report the final row, column and direction, and derive the answer from it
    cout << endl << "Answer 2 - final row = " << posY << ", final column = " << posX << ", final facing = " << dir << " final password = " << 1000 * posY + 4 * posX + dir << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
