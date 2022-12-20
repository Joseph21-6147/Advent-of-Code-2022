// AoC 2022 - day 18 - Boiling Boulders
// ====================================

// date:  2022-12-18
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../flcTimer.h"
#include "vector_types.h"   // for the vi3d type of vector

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the data consists of 'cubes' having a coordinate in a 3d space
typedef vi3d DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData.push_back( vi3d( 1, 1, 1 ) );
    dData.push_back( vi3d( 2, 1, 1 ) );
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
        if (sLine.length() > 0) {    // non empty line
            string sX = get_token_dlmtd( ",", sLine );
            string sY = get_token_dlmtd( ",", sLine );
            string sZ = get_token_dlmtd( ",", sLine );

            vData.push_back( vi3d( atoi( sX.c_str()), atoi( sY.c_str()), atoi( sZ.c_str())) );
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << iData.ToString() << endl;
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

// connectivity of every cube is modeled in an int[]
typedef struct sCubeStruct {
    vi3d coord;
    int covered[6] = { 0 };    // index 0 = x-, 1 = x+, 2 = y-, 3 = y+, 4 = z-, 5 = z+
    int totalCovered = 0;
    int blobID = -1;           // needed for part 2
} CubeType;
typedef vector<CubeType> CubeStream;

vi3d glbBB1, glbBB2;           // bounding box corner points

// for testing
void PrintCubeStream( CubeStream &cs ) {
    for (int i = 0; i < (int)cs.size(); i++) {
        cout << "Cube at coord: " << cs[i].coord.ToString();
        cout << " has " << cs[i].totalCovered << " faces covered: { ";
        for (int j = 0; j < 6; j++) cout << cs[i].covered[j] << ", ";
        cout << "}" << endl;
    }
}

// Puts the data in input data in the puzzle datastructure cubes - no processing is done
void TransformInputDataToCubeStream( DataStream &iData, CubeStream &cubes ) {
    // first copy the coordinates from the input stream to the cube stream
    cubes.clear();
    for (int i = 0; i < (int)iData.size(); i++) {
        CubeType tmp;
        tmp.coord = iData[i];
        cubes.push_back( tmp );
    }
}

// Searches cbs from begin to end to find c. If found, the index is returned (-1 otherwise)
int FindInCubeList( CubeStream &cbs, vi3d c ) {
    int result = -1;
    for (int i = 0; i < (int)cbs.size() && result < 0; i++) {
        if (cbs[i].coord == c) result = i;
    }
    return result;
}

// Analyses the dataset given in cubes, and sets the connectivity data in each of the elements therein.
void AnalyseCubeConnectivity( CubeStream &cubes ) {

    // sort the cube stream
    sort( cubes.begin(), cubes.end(), []( CubeType a, CubeType b ) {
            return a.coord < b.coord;
         }
    );

    // Convenience search lambda - Like FindInCubeList(), but this function starts at curIndex + 1 and moves forward
    auto find_forward = [&]( CubeStream &cs, int curIndex, vi3d c ) {
        int nFound = -1;
        for (int i = curIndex + 1; i < (int)cs.size() && nFound < 0; i++) {
            if (cs[i].coord == c) nFound = i;
        }
        return nFound;
    };

    // Convenience search lambda - Like FindInCubeList(), but this function starts at curIndex - 1 and moves backwards
    auto find_reverse = [&]( CubeStream &cs, int curIndex, vi3d c ) {
        int nFound = -1;
        for (int i = curIndex - 1; i >= 0 && nFound < 0; i--) {
            if (cs[i].coord == c) nFound = i;
        }
        return nFound;
    };

    // now analyse each cube and fill it's covered[] array (and keep track of the totalcovered value)
    for (int i = 0; i < (int)cubes.size(); i++) {
        // process only cubes that aren't fully clear yet
        if (cubes[i].totalCovered != 6) {
            for (int j = 0; j < 6; j++) {
                vi3d nghb;
                int  fst, scd;
                switch (j) {
                    case 0: nghb = vi3d( cubes[i].coord.x - 1, cubes[i].coord.y    , cubes[i].coord.z     ); fst = 0; scd = 1; break;
                    case 1: nghb = vi3d( cubes[i].coord.x + 1, cubes[i].coord.y    , cubes[i].coord.z     ); fst = 1; scd = 0; break;
                    case 2: nghb = vi3d( cubes[i].coord.x    , cubes[i].coord.y - 1, cubes[i].coord.z     ); fst = 2; scd = 3; break;
                    case 3: nghb = vi3d( cubes[i].coord.x    , cubes[i].coord.y + 1, cubes[i].coord.z     ); fst = 3; scd = 2; break;
                    case 4: nghb = vi3d( cubes[i].coord.x    , cubes[i].coord.y    , cubes[i].coord.z - 1 ); fst = 4; scd = 5; break;
                    case 5: nghb = vi3d( cubes[i].coord.x    , cubes[i].coord.y    , cubes[i].coord.z + 1 ); fst = 5; scd = 4; break;
                }

                if (cubes[i].covered[j] == 0) {
                    int cand = ((j % 2) == 0 ? find_reverse( cubes, i, nghb ) : find_forward( cubes, i, nghb ));
                    if (cand >= 0) {
                        cubes[i   ].covered[fst] = 1; cubes[i   ].totalCovered += 1;
                        cubes[cand].covered[scd] = 1; cubes[cand].totalCovered += 1;
                    }
                }
            }
        }
    }
}

// Accumulates the number of free faces in the cube stream cs.
// Note that the connectivity must be analysed before calling this function.
int GetSurfaceArea( CubeStream &cs ) {
    int nResult = 0;
    for (int i = 0; i < (int)cs.size(); i++) {
        nResult += (6 - cs[i].totalCovered);
    }
    return nResult;
}

// Analyses the bounding box for the set of cubes in cs. Result is
// passed back in dpt1 and dpt2 (diagonal points 1 and 2)
void GetBoundingBox( CubeStream &cs, vi3d &dpt1, vi3d &dpt2 ) {
    int x_min = INT_MAX;
    int y_min = INT_MAX;
    int z_min = INT_MAX;
    int x_max = INT_MIN;
    int y_max = INT_MIN;
    int z_max = INT_MIN;

    for (int i = 0; i < (int)cs.size(); i++) {
        vi3d &curCoord = cs[i].coord;
        if (curCoord.x < x_min) x_min = curCoord.x;
        if (curCoord.x > x_max) x_max = curCoord.x;
        if (curCoord.y < y_min) y_min = curCoord.y;
        if (curCoord.y > y_max) y_max = curCoord.y;
        if (curCoord.z < z_min) z_min = curCoord.z;
        if (curCoord.z > z_max) z_max = curCoord.z;
    }
    dpt1 = vi3d( x_min, y_min, z_min );
    dpt2 = vi3d( x_max, y_max, z_max );
}

// from the cubes that were given as input data, the pockets are derived:
//   1. a set of pockets is initialized that has the size of the bounding box of the cube data
//   2. all pockets are filtered out for which coordinate there is a cube in the cube data
void TransformCubesToPockets( CubeStream &cs, CubeStream &pockets ) {

    // create pocket volume according to bounding box size
    for (int z = glbBB1.z; z <= glbBB2.z; z++) {
        for (int y = glbBB1.y; y <= glbBB2.y; y++) {
            for (int x = glbBB1.x; x <= glbBB2.x; x++) {
                CubeType aux;
                aux.coord = vi3d( x, y, z );
                pockets.push_back( aux );
            }
        }
    }
    // filter out pocket cells that are present as cubes in the cube list
    pockets.erase(
        remove_if( pockets.begin(), pockets.end(), [&](CubeType &a) {
            return (FindInCubeList( cs, a.coord ) != -1);
        } ),
        pockets.end()
    );
}

// for part 2 I need a blob type, where a blob is just a CubeStreams
// this blob list type is used to partition the pocket space into separate blobs which are analysed
// on being interior or exterior blobs
typedef vector<CubeStream> BlobListType;

// Recursively adds cubes to an initial blob
void AddToBlob( CubeStream &pockets, int pocketIndex, int bId, CubeStream &blbList ) {
    CubeType &curPck = pockets[pocketIndex];
    // only process if not already processed
    if (curPck.blobID == -1) {
        // mark this pocket as 'belonging to this blob'
        curPck.blobID = bId;
        // grab current location (coordinate)
        vi3d &loc = curPck.coord;
        blbList.push_back( curPck );
        // try to process neighbouring pockets into this blob
        int nIx;
        // if the neighbour coordinate is in bounds, and a pocket can be found, call AddToBlob() recursively on it
        if (loc.x-1 >= glbBB1.x && ((nIx = FindInCubeList(pockets, loc + vi3d(-1, 0, 0))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
        if (loc.y-1 >= glbBB1.y && ((nIx = FindInCubeList(pockets, loc + vi3d( 0,-1, 0))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
        if (loc.z-1 >= glbBB1.z && ((nIx = FindInCubeList(pockets, loc + vi3d( 0, 0,-1))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
        if (loc.x+1 <= glbBB2.x && ((nIx = FindInCubeList(pockets, loc + vi3d(+1, 0, 0))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
        if (loc.y+1 <= glbBB2.y && ((nIx = FindInCubeList(pockets, loc + vi3d( 0,+1, 0))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
        if (loc.z+1 <= glbBB2.z && ((nIx = FindInCubeList(pockets, loc + vi3d( 0, 0,+1))) != -1)) AddToBlob( pockets, nIx, bId, blbList );
    }
}

// creates a new blob, having pockets[pocketindex] as the start coordinate of the blob
void CreateBlob( CubeStream &pockets, int pocketIndex, int blobId, BlobListType &blbs ) {
    CubeStream newBlob;
    AddToBlob( pockets, pocketIndex, blobId, newBlob );
    blbs.push_back( newBlob );
}

// create a list of blobs out of list pockets
void TransformPocketsToBlobs( CubeStream &pockets, BlobListType &blobs ) {

    int blobCntr = 0;
    for (int i = 0; i < (int)pockets.size(); i++) {
        if (pockets[i].blobID == -1) {
            // this call will create a blob using some kind of flood fill algo
            CreateBlob( pockets, i, blobCntr, blobs );
        }
        blobCntr += 1;
    }
}

// Returns true if coordinate loc is on the edge / border of the bounding box.
// In that case a pocket will not be an interior pocket, and the blob that contains
// that pocket will be an exterior blob
bool IsBorderLocation( vi3d &loc ) {
    return (
        loc.x == glbBB1.x || loc.x == glbBB2.x ||
        loc.y == glbBB1.y || loc.y == glbBB2.y ||
        loc.z == glbBB1.z || loc.z == glbBB2.z
    );
}

// This function works out the connectivity for each of the blobs internally, and then checks
// the cumulated surface for that blob, and whether it is an exerior pocket blob or an
// interior pocket blob. The function returns the sum of the interior blob surfaces.
int AnalyseBlobData( BlobListType &myBlobs ) {

    int nGlobalCumSurface = 0;
    for (int i = 0; i < (int)myBlobs.size(); i++) {
        CubeStream &local_cs = myBlobs[i];
        // analyse connectivity info for each blob
        AnalyseCubeConnectivity( local_cs );
        // check if this blob has any boundary coordinates
        bool bInteriorBlob = true;
        int nLocalCumSurface = 0;
        for (int j = 0; j < (int)local_cs.size() && bInteriorBlob; j++) {
            nLocalCumSurface += (6 - local_cs[j].totalCovered);
            // if the blob has any coordinate that is on the border of the bounding box
            // the whole blob cannot be interior and consequently is exterior
            if (IsBorderLocation( local_cs[j].coord )) {
                bInteriorBlob = false;
            }
        }
        // Accumulate the surfaces only from blobs that are interior
        if (bInteriorBlob)
            nGlobalCumSurface += nLocalCumSurface;
    }
    return nGlobalCumSurface;
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
    int nAnswer1;
    CubeStream cubeData;
    TransformInputDataToCubeStream( inputData, cubeData );
    GetBoundingBox( cubeData, glbBB1, glbBB2 );
    // after some initialisation, collect the cnnectivity data for all cubes, and
    // work out and report the answer
    AnalyseCubeConnectivity( cubeData );
    nAnswer1 = GetSurfaceArea( cubeData );
    cout << endl << "Answer 1 - Total surface area is: " << nAnswer1 << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here
    int nAnswer2;
    // Transform the cube data into the complementary set of pockets
    CubeStream pocketData;
    TransformCubesToPockets( cubeData, pocketData );
    // transform the pocket data into a partitioning of blobs (sets of connected pockets)
    BlobListType blobData;
    TransformPocketsToBlobs( pocketData, blobData );
    // analyse the blobs to get the interior surface data out of it
    int nInteriorSurface = AnalyseBlobData( blobData );
    // note that nAnswer1 contains the total surface - so nAnswer2 is given as the difference
    // between answer 1 and the interior surface
    nAnswer2 = nAnswer1 - nInteriorSurface;

    cout << endl << "Answer 2 - total surface area = " << nAnswer1;
    cout << " interior surface area = " << nInteriorSurface;
    cout << " exterior surface area = " << nAnswer2 << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
