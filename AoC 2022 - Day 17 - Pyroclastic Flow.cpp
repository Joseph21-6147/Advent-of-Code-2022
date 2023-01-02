// AoC 2022 - day 17b - Pyroclastic Flow
// ========================

// date:  2022-12-23
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"

using namespace std;

// for testing
void WaitForKey( char key ) {
    char c;
    do {
        cout << "PRESS <" << key << "> TO CONTINUE ";
        cin >> c;
    } while (c != key);
}

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of one large 'jet pattern' pointing left or right
typedef string DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS


// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData.push_back( ">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>" );
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
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

int glbClock = 0;         // controls alternating jet pushing and moving around

int glbJetIndex = 0;      // enables cyclic jet flow

void NextJetIndex( DataStream &iData ) {
    glbJetIndex = (glbJetIndex + 1) % (int)iData[0].length();
}

char GetJetDir( DataStream &iData ) {
    return iData[0][glbJetIndex];
}

typedef vector<vllong2d> RockPattern;
typedef vector<RockPattern> RockStream;

RockStream rocks = {
    { vllong2d( 0, 0 ), vllong2d( 1, 0 ), vllong2d( 2, 0 ), vllong2d( 3, 0 ) },                // - shape
    { vllong2d( 1, 0 ), vllong2d( 1, 1 ), vllong2d( 1, 2 ), vllong2d( 0, 1 ), vllong2d( 2, 1 ) },  // + shape
    { vllong2d( 0, 0 ), vllong2d( 1, 0 ), vllong2d( 2, 0 ), vllong2d( 2, 1 ), vllong2d( 2, 2 ) },  // L shape
    { vllong2d( 0, 0 ), vllong2d( 0, 1 ), vllong2d( 0, 2 ), vllong2d( 0, 3 ) },                // | shape
    { vllong2d( 0, 0 ), vllong2d( 1, 0 ), vllong2d( 0, 1 ), vllong2d( 1, 1 ) }                 // . shape
};

// the simulation contains one dynamic rock tumbling down ...
int glbCurRock;           // current type of rock - cycles over [0-4]
vllong2d glbCurPos;       // position of current rock
// ... and the board having a floor and possibly previously frozen rocks on it
RockPattern glbBoard;

long long glbFloorHeight = -1;
// calculates the highest (= maximum y coordinate) point in the glbBoard RockPattern
// (which contains the frozen rocks on the board)
long long GetHighPoint() {
    long long result = glbFloorHeight;
    if (!glbBoard.empty()) {
        long long localMax = INT_MIN;
        for (int i = 0; i < (int)glbBoard.size(); i++) {
            if (localMax < glbBoard[i].y) {
                localMax = glbBoard[i].y;
            }
        }
        result = localMax;
    }
    return result;
}

// The board is modeled as a list of obstacles. Most of them are rock components,
// but at init the floor is put into the board.
void InitBoard() {
    for (int i = 0; i < 7; i++) {
        glbBoard.push_back( vllong2d( i, -1 ));
    }
}

// Check if the current rock (modeled in global variables glbCurRock and glbCurPos) collides with
// board boundaries or with other frozen rocks when it is moved with offset.
// Returns true if it does not collide, and then sets the glbCurPos to the new position.
// Returns false otherwise
bool AttemptMoveRock( vllong2d offset ) {
    bool bInBounds = true;
    bool bNoCollide = true;

    for (int i = 0; i < (int)rocks[glbCurRock].size() && bInBounds && bNoCollide; i++) {
        vllong2d aux = rocks[glbCurRock][i] + glbCurPos + offset;
        // shifted rock is in bounds if x coordinate is in [0-6]
        bInBounds = (0 <= aux.x && aux.x < 7);
        // shifted rock doesn't collide with frozen rocks on board if overlap is 0
        bNoCollide = (count( glbBoard.begin(), glbBoard.end(), aux ) == 0);
    }
    // move the rock if possible
    if (bInBounds && bNoCollide) {
        glbCurPos += offset;
    }
    return bInBounds && bNoCollide;
}

long long countNrRocks = 0;

typedef struct sFrozenRockStruct {
    int nType;
    vllong2d freezePos;
} FrozenRock;
typedef vector<FrozenRock> FrozenStream;

FrozenStream frozenRocks;

// returns true if the rocks at indices index1 and index2 in the frozenRocks global list
// have the same horizontal (x-) coordinate
bool SameHorOffset( int index1, int index2 ) {
    bool bResult = false;
    FrozenRock &r1 = frozenRocks[index1];
    FrozenRock &r2 = frozenRocks[index2];
    if (r1.nType != r2.nType) {
        cout << "ERROR: SameHorOffset() --> types don't match: " << r1.nType << " vs: " << r2.nType << endl;
    } else {
        bResult = r1.freezePos.x == r2.freezePos.x;
    }
    return bResult;
}

// This function helps to determine what the circularity pattern is in the frozen rock stack
long long CheckCircularity( int type, vllong2d fPos ) {
    long long llResult = -1;
    // assumption - cur element is last element in vector
    int curIndex = (int)frozenRocks.size() - 1;

    // check all frozen rocks for the same type and same x coordinate
    for (int i = (int)frozenRocks.size() - 6; i >= 0 && llResult == -1; i -= 5) {
        FrozenRock &otherRock = frozenRocks[i];
        // check if any rocks of the same type were frozen before at the exact same x coordinate
        if (type != otherRock.nType) {
            cout << "ERROR: CheckCircularity() --> type mismatch: " << type << " vs: " << frozenRocks[i].nType << endl;
        } else {
            if (SameHorOffset( curIndex, i )) {
                // frozenRocks curIndex and i have the same x value. Check for the four rocks below each of them

                long long tmpYoffset = (fPos.y - otherRock.freezePos.y);
                bool bChecks[4] = { false };
                // make sure the indices are within the range of the frozenRocks vector
                if (i - 4 >= 0) {
                    for (int j = 1; j < 5; j++) {
                        // there's only a match if the 4 other frozen rocks also correspond in x-offset, AND have the same y-difference
                        bChecks [j - 1] = SameHorOffset( curIndex - j, i - j ) &&
                                          (tmpYoffset == (frozenRocks[curIndex - j].freezePos.y - frozenRocks[i - j].freezePos.y));
                    }
                }

                if (bChecks[0] && bChecks[1] && bChecks[2] && bChecks[3]) {
                    cout << "Match found !! - THIS rock index: " << curIndex << ", type: " <<            type << ", position: " << fPos.ToString()
                         <<               ", OTHER rock index: " << i        << ", type: " << otherRock.nType << ", position: " << otherRock.freezePos.ToString()
                         << ", vertical distance = " << tmpYoffset << endl;

                    // set return value to vertical distance
                    llResult = tmpYoffset;
                }
            }
        }
    }
    return llResult;
}


// return true if a new block was inited
bool OneMove( DataStream &iData ) {
    bool bResult = false;

    if (glbClock % 2 == 0) {

        // if clock tick is even, the current rock gets pushed by a jet of hot gas
        char cDir = GetJetDir( iData );
        switch (cDir) {
            case '<': AttemptMoveRock( vllong2d( -1, 0 )); break;
            case '>': AttemptMoveRock( vllong2d( +1, 0 )); break;
            default: cout << "ERROR: OneMove() --> shit here " << endl;
        }
        // advance jet counter
        NextJetIndex( iData );
    } else {

        // if clock tick is not even, attempt to move the rock one step down
        bool bBlocked = !AttemptMoveRock( vllong2d( 0, -1 ));
        // if the block can't move further down, freeze it in the board and init a new block
        if (bBlocked) {
            bResult = true;
            for (int i = 0; i < (int)rocks[glbCurRock].size(); i++) {
                glbBoard.push_back( rocks[glbCurRock][i] + glbCurPos );
            }
            frozenRocks.push_back( { glbCurRock, glbCurPos } );

            CheckCircularity( glbCurRock, glbCurPos );

            // advance rock counter
            countNrRocks += 1;
            glbCurRock = (glbCurRock + 1) % 5;
            // init rock position
            glbCurPos = vllong2d( 2, GetHighPoint() + 4 );
        }
    }
    // keep glbClock modular to prevent overflow
    glbClock = (glbClock + 1) % 10000000;

    return bResult;
}

// for testing - output the board and current rock in a visual way
void PrintBoard() {
    // 1. create an empty board to display
    vector<string> display;
    for (int i = 0; i < 24; i++) {
        display.push_back( "......." );
    }
    // 2. project all board cells on it
    for (int i = 0; i < (int)glbBoard.size(); i++) {
        vllong2d curPos = glbBoard[i];
        if (curPos.y >= 0) {
            display[curPos.y][curPos.x] = '#';
        }
    }
    // 3. project dynamic rock on it
    for (int i = 0; i < (int)rocks[glbCurRock].size(); i++) {
        vllong2d curPos = rocks[glbCurRock][i];
        curPos += glbCurPos;
        display[curPos.y][curPos.x] = '@';
    }
    // 4. now display it
    int startHeight = (int)display.size() - 1;
    for (int i = startHeight; i >= 0; i--) {
        cout << "|" << display[i] << "|" << endl;
    }
    cout << "+-------+" << endl;
}

// returns the largest y coordinate in the glbBoard modeled board
long long TowerHeight() {
    long long nResult = INT_MIN;
    for (int i = 0; i < (int)glbBoard.size(); i++) {
        if (glbBoard[i].y > nResult)
            nResult = glbBoard[i].y;
    }
    return nResult + 1;
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

    // init the board, clock and jet index
    InitBoard();
    glbClock = 0;
    glbJetIndex = 0;
    // init the dynamic rock
    glbCurRock = 0;
    glbCurPos = vllong2d( 2, GetHighPoint() + 4 );

//    cout << endl << "Initial situation: " << endl;
//    PrintBoard();

    long long idiotNumber = 1000000000000;

// this code was used to (manually) assess cyclicity of the input data
// ===================================================================
//    for (long long i = 0; countNrRocks < idiotNumber; i++) {
//        OneMove( inputData );
////        cout << "After move: " << i + 1 << " height = " << TowerHeight() << " nr of rocks in board: " << countNrRocks << endl;
////        PrintBoard();
////        WaitForKey( 'n' );
//        if (countNrRocks % 1000 == 0) {
//            cout << countNrRocks << endl;
//        }
//    }


// this code was used to (manually) assess height of stack after x rocks
// =====================================================================
//    for (long long i = 0; countNrRocks < idiotNumber; i++) {
//        OneMove( inputData );
//        cout << "After move: " << i + 1 << " height = " << TowerHeight() << " nr of rocks in board: " << countNrRocks << endl;
//        if (countNrRocks % 1000 == 0) {
//            cout << countNrRocks << endl;
//        }
//    }

/*  Specific for part 2 - After discovering a cycle pattern with
        base (start) of pattern
        cycle - the number of rocks in the cycle
        yield - the height of the stack of rocks in a cycle

    The formula becomes
        answer =
            (10^12 div cycle) * yield +
            height( 10^12 mod cycle)

    I worked out these values separately, so there's not integral answer coming out of this code
 */

    // The values I found for cycle and yield (using the above outcommented snippets of code) are hardcoded here to get the answers
    long long firstPart, secndPart;
    firstPart = (idiotNumber / 35) * 53;
    secndPart = (idiotNumber % 35);
    cout << "Answer 1 (for TEST  ) should be somewhere around: " << firstPart << ", where height of: " << secndPart << " rocks must still be added" << endl;

    firstPart = (idiotNumber / 1745) * 2752;
    secndPart = (idiotNumber % 1745);
    cout << "Answer 1 (for PUZZLE) should be somewhere around: " << firstPart << ", where height of: " << secndPart << " rocks must still be added" << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // Accidentally overwrote the part 1 code when solving part 2, so there's no part 2 code... :\

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
