// AoC 2022 - day 12 - Hill Climbing
// =================================

// date:  2022-12-12
// by:    Joseph21 (Joseph21-6147)

// Courtesy of Javidx9 for re-using his A* code

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"  // needed for vi2d coordinate type

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of a 'map' having elevation values (a-z) and S(tart) and E(nd) values
typedef string DatumType;
typedef vector<DatumType> DataStream;

int nMapWidth  = -1;       // sizes are set after reading in the data
int nMapHeight = -1;

vi2d glbStart, glbEnd;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData.push_back( "Sabqponm" );
    dData.push_back( "abcryxxl" );
    dData.push_back( "accszExk" );
    dData.push_back( "acctuvwj" );
    dData.push_back( "abdefghi" );
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;
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
    // initialize map size variables - they are known AFTER getting the data
    nMapHeight = dData.size();
    nMapWidth = (nMapHeight == 0 ? 0 : dData[0].length());
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS

// for convenient addressing
char GetElevation( DataStream &dData, int x, int y ) {
    return dData[y][x];
}

// search the map for the 'S' and 'E' characters that denote start and end point
// put them coordinates in references strt and stop
// NOTE - to make the path finding work correctly, the 'E' is replaced by a 'z' elevation
//        as stated in the puzzle
void GetRoutePoints( DataStream &dData, vi2d &strt, vi2d &stop ) {
    for (int y = 0; y < nMapHeight; y++) {
        for (int x = 0; x < nMapWidth; x++) {
            switch (GetElevation( dData, x, y )) {
                case 'S': strt = { x, y }; break;
                case 'E': stop = { x, y }; break;
            }
        }
    }
    // set Start and End points to correct height values in the map
    // (otherwise the path finding doesn't work correctly)
    dData[glbEnd.y][glbStart.x] = 'a';
    dData[glbEnd.y][glbEnd.x]   = 'z';
}

// ==========   A* SPECIFIC STUFF ( THANKS @JAVIDX9 !! )   =============

struct sNode
{
    bool bObstacle = false;			// Is the node an obstruction?
    bool bVisited = false;			// Have we searched this node before?
    float fGlobalGoal;				// Distance to goal so far
    float fLocalGoal;				// Distance to goal if we took the alternative route
    int x;							// Nodes position in 2D space
    int y;
    std::vector<sNode*> vecNeighbours;	// Connections to neighbours
    sNode* parent;					// Node connecting to this node that offers shortest parent

    // added this char field for displaying purposes
    char cDisplay = '.';
};

sNode *nodes     = nullptr;
sNode *nodeStart = nullptr;
sNode *nodeEnd   = nullptr;

// These four tester functions are used in building the A* graph, and check for:
//   * boundary of maps
//   * hight difference not too large (step up of +1 is allowed, step down is unlimited)
// NOTE - there's an assumption that 'S' has height 'a' and 'E' has height 'z'. This should be set in the height map!!
bool CanMoveUp( DataStream &dData, int x, int y ) { return (y >              0 && GetElevation( dData, x    , y - 1 ) <= GetElevation( dData, x, y ) + 1); }
bool CanMoveDn( DataStream &dData, int x, int y ) { return (y < nMapHeight - 1 && GetElevation( dData, x    , y + 1 ) <= GetElevation( dData, x, y ) + 1); }
bool CanMoveLt( DataStream &dData, int x, int y ) { return (x >              0 && GetElevation( dData, x - 1, y     ) <= GetElevation( dData, x, y ) + 1); }
bool CanMoveRt( DataStream &dData, int x, int y ) { return (x < nMapWidth  - 1 && GetElevation( dData, x + 1, y     ) <= GetElevation( dData, x, y ) + 1); }

// In Javids video on A* this stuff is done in OnUserCreate()
void BuildHeightMap( DataStream &dData ) {
    // initialize start and end points from input
    GetRoutePoints( dData, glbStart, glbEnd );

    // Create a 2D array of nodes - this is for convenience of rendering and construction
    // and is not required for the algorithm to work - the nodes could be placed anywhere
    // in any space, in multiple dimensions...
    nodes = new sNode[nMapWidth * nMapHeight];
    for (int x = 0; x < nMapWidth; x++) {
        for (int y = 0; y < nMapHeight; y++) {
            nodes[y * nMapWidth + x].x = x; // ...because we give each node its own coordinates
            nodes[y * nMapWidth + x].y = y;
            nodes[y * nMapWidth + x].bObstacle = false;
            nodes[y * nMapWidth + x].parent = nullptr;
            nodes[y * nMapWidth + x].bVisited = false;
        }
    }
    // Create connections - in this case nodes are on a regular grid
    for (int x = 0; x < nMapWidth; x++) {
        for (int y = 0; y < nMapHeight; y++) {
            if (CanMoveUp( dData, x, y )) nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x + 0)]);
            if (CanMoveDn( dData, x, y )) nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x + 0)]);
            if (CanMoveLt( dData, x, y )) nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x - 1)]);
            if (CanMoveRt( dData, x, y )) nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x + 1)]);
        }
    }
    // Manually position the start and end markers
    nodeStart = &nodes[ glbStart.y * nMapWidth + glbStart.x ];
    nodeEnd   = &nodes[ glbEnd.y   * nMapWidth + glbEnd.x   ];
}

// returns the length of the shortest path found from nodeStart to nodeEnd (0 if not found)
int Solve_AStar( DataStream &dData ) {
    // Reset Navigation Graph - default all node states
    for (int x = 0; x < nMapWidth; x++)
        for (int y = 0; y < nMapHeight; y++) {
            nodes[y * nMapWidth + x].bVisited = false;
            nodes[y * nMapWidth + x].fGlobalGoal = INFINITY;
            nodes[y * nMapWidth + x].fLocalGoal = INFINITY;
            nodes[y * nMapWidth + x].parent = nullptr;	// No parents
            // added for display purposes
            nodes[y * nMapWidth + x].cDisplay = '.';
        }

    auto distance = [](sNode *a, sNode *b) { // For convenience
        return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
    };

    auto heuristic = [distance](sNode *a, sNode *b) { // So we can experiment with heuristic
        return distance(a, b);
    };

    // Setup starting conditions
    sNode *nodeCurrent = nodeStart;
    nodeStart->fLocalGoal = 0.0f;
    nodeStart->fGlobalGoal = heuristic(nodeStart, nodeEnd);

    // Add start node to not tested list - this will ensure it gets tested.
    // As the algorithm progresses, newly discovered nodes get added to this
    // list, and will themselves be tested later
    list<sNode*> listNotTestedNodes;
    listNotTestedNodes.push_back(nodeStart);

    // if the not tested list contains nodes, there may be better paths
    // which have not yet been explored. However, we will also stop
    // searching when we reach the target - there may well be better
    // paths but this one will do - it wont be the longest.
    while (!listNotTestedNodes.empty() && nodeCurrent != nodeEnd) { // Find absolutely shortest path // && nodeCurrent != nodeEnd)
        // Sort Untested nodes by global goal, so lowest is first
        listNotTestedNodes.sort(
            [](const sNode * lhs, const sNode * rhs) {
                return lhs->fGlobalGoal < rhs->fGlobalGoal;
            }
        );

        // Front of listNotTestedNodes is potentially the lowest distance node. Our
        // list may also contain nodes that have been visited, so ditch these...
        while(!listNotTestedNodes.empty() && listNotTestedNodes.front()->bVisited)
            listNotTestedNodes.pop_front();

        // ...or abort because there are no valid nodes left to test
        if (listNotTestedNodes.empty())
            break;

        nodeCurrent = listNotTestedNodes.front();
        nodeCurrent->bVisited = true; // We only explore a node once

        // Check each of this node's neighbours...
        for (auto nodeNeighbour : nodeCurrent->vecNeighbours) {
            // ... and only if the neighbour is not visited and is
            // not an obstacle, add it to NotTested List
            if (!nodeNeighbour->bVisited && nodeNeighbour->bObstacle == 0)
                listNotTestedNodes.push_back(nodeNeighbour);

            // Calculate the neighbours potential lowest parent distance
            float fPossiblyLowerGoal = nodeCurrent->fLocalGoal + distance(nodeCurrent, nodeNeighbour);

            // If choosing to path through this node is a lower distance than what
            // the neighbour currently has set, update the neighbour to use this node
            // as the path source, and set its distance scores as necessary
            if (fPossiblyLowerGoal < nodeNeighbour->fLocalGoal) {
                nodeNeighbour->parent = nodeCurrent;
                nodeNeighbour->fLocalGoal = fPossiblyLowerGoal;

                // The best path length to the neighbour being tested has changed, so
                // update the neighbour's score. The heuristic is used to globally bias
                // the path algorithm, so it knows if its getting better or worse. At some
                // point the algo will realise this path is worse and abandon it, and then go
                // and search along the next best path.
                nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, nodeEnd);
            }
        }
    }

    // finalise the outcome of the A* algo - create the path and report its length
    deque<vi2d> path;
    bool bPathFound = (nodeCurrent == nodeEnd);
    if (bPathFound) {
        // push the end node's coordinates to the *front* of the deque
        sNode *current = nodeEnd;
        path.push_front( { current->x, current->y } );
        // walk the path backwards, from end node to start node, pushing the coordinates into
        // the deque, and setting the appropriate display characters
        while (current != nodeStart) {
            vi2d cur = { current->x, current->y };
            vi2d par = { current->parent->x, current->parent->y };
            if (cur.x == par.x) {
                if (cur.y < par.y) current->parent->cDisplay = '^';
                else               current->parent->cDisplay = 'v';
            } else {
                if (cur.x < par.x) current->parent->cDisplay = '<';
                else               current->parent->cDisplay = '>';
            }

            current = current->parent;
            path.push_front( { current->x, current->y } );
        }
        // for display purposes explicitly mark start and end point
        nodeStart->cDisplay = 'S';
        nodeEnd->cDisplay = 'E';

        bool bDebugDisplay = false;   // set to true for debugging output
        if (bDebugDisplay) {
            for (int y = 0; y < nMapHeight; y++) {
                for (int x = 0; x < nMapWidth; x++) {
                    cout << nodes[ y * nMapWidth + x ].cDisplay;
                }
                cout << endl;
            }
            cout << endl << "path length = " << path.size() - 1 << endl;
        }
    }
    // return length of shortest path found (or 0 if not found)
    return (bPathFound ? path.size() - 1 : 0);
}

// ==========   MAIN()

// little struct needed for part 2
typedef struct sPuzzleStruct {
    vi2d startpoint = { -1, -1 };
    int pathlen = -1;
} PuzzleStruct;

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

    // build the height map that is needed by the A* algorithm
    BuildHeightMap( inputData );
    // call A* function to get the pathlen for the path from nodeStart to nodeEnd
    int nPathLen1 = Solve_AStar( inputData );
    cout << "Shortest path found = " << nPathLen1 << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // first build a list of all start points (i.e points having elevation 'a')
    vector<vi2d> vStartPoints;
    for (int y = 0; y < nMapHeight; y++) {
        for (int x = 0; x < nMapWidth; x++) {
            if (GetElevation( inputData, x, y ) == 'a') {
                vStartPoints.push_back( { x, y } );
            }
        }
    }
    // then apply A* on all these start points keeping the same end point
    // store the result in a proper list
    vector<PuzzleStruct> vSolutions;
    for (int i = 0; i < (int)vStartPoints.size(); i++) {
        vi2d elt = vStartPoints[i];
        nodeStart = &nodes[ elt.y * nMapWidth + elt.x ];

        int nLenFound = Solve_AStar( inputData );
        if (nLenFound > 0) {
            vSolutions.push_back( { elt, nLenFound } );
        }
    }
    // then sort that list w.r.t pathlen
    sort( vSolutions.begin(), vSolutions.end(),
        [](PuzzleStruct a, PuzzleStruct b){
            return a.pathlen < b.pathlen;
        }
    );
    // finally report the shortest one
    cout << "Shortest path found = " << vSolutions[0].pathlen << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
