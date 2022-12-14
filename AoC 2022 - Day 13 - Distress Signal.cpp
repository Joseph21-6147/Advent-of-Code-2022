// AoC 2022 - day 13 - Distress Signal
// ===================================

// date:  2022-12-13
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

#define EMPTY 0    // constants for nType field of SignalType
#define INTGR 1
#define LIST  2

string TypeToString( int type ) {
    string result;
    switch (type) {
        case EMPTY: result = "EMPTY"; break;
        case INTGR: result = "INTGR"; break;
        case LIST : result = "LIST";  break;
    }
    return result;
}

typedef struct signalStruct {
    int nType = EMPTY;                    // one of EMPTY, INTGR or LIST
    int nValue;                           // used in case of INTGR
    vector<struct signalStruct *> vList;  // used in case of LIST
    struct signalStruct *pParent = nullptr;
} SignalType;

// the input data consists of 'pairs of signals' having a first and second selection
typedef struct datumStruct {
    string sSig1, sSig2;
    SignalType *pSig1 = nullptr;
    SignalType *pSig2 = nullptr;
} DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;
    aux.sSig1 = "[1,1,3,1,1]";                 aux.sSig2 = "[1,1,5,1,1]";                  dData.push_back( aux );
    aux.sSig1 = "[[1],[2,3,4]]";               aux.sSig2 = "[[1],4]";                      dData.push_back( aux );
    aux.sSig1 = "[9]";                         aux.sSig2 = "[[8,7,6]]";                    dData.push_back( aux );
    aux.sSig1 = "[[4,4],4,4]";                 aux.sSig2 = "[[4,4],4,4,4]";                dData.push_back( aux );
    aux.sSig1 = "[7,7,7,7]";                   aux.sSig2 = "[7,7,7]";                      dData.push_back( aux );
    aux.sSig1 = "[]";                          aux.sSig2 = "[3]";                          dData.push_back( aux );
    aux.sSig1 = "[[[]]]";                      aux.sSig2 = "[[]]";                         dData.push_back( aux );
    aux.sSig1 = "[1,[2,[3,[4,[5,6,7]]]],8,9]"; aux.sSig2 = "[1,[2,[3,[4,[5,6,0]]]],8,9]";  dData.push_back( aux );
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;
    int nLineNr = 0;
    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            nLineNr += 1;
            switch (nLineNr) {
                case  1: datum.sSig1 = sLine;                           break;
                case  2: datum.sSig2 = sLine; vData.push_back( datum ); break;
                default: cout << "ERROR - ReadInputData() --> unknown nLineNr: " << nLineNr << endl;
            }
        } else {                     // reset line number on empty lines
            nLineNr = 0;
        }
    }
    dataFileStream.close();
}

void GetData_TEST(   DataStream &dData ) { ReadInputData( "input.test.txt", dData ); }
void GetData_PUZZLE( DataStream &dData ) { ReadInputData( "input.puzzle.txt", dData ); }

// ==========   CONSOLE OUTPUT FUNCTIONS

// output to console for testing
void PrintDatum( DatumType &iData ) {
    cout << iData.sSig1 << " " << iData.sSig2 << endl;
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

// Cuts of and returns the front token from "input_to_be_adapted", using "delim" as delimiter.
// If delimiter is not found, the complete input string is passed as a token.
// The input string becomes shorter as a result, and may even become empty
string get_token_dlmtd( const string &delim, string &input_to_be_adapted ) {
    size_t splitIndex = input_to_be_adapted.find( delim );
    string token = input_to_be_adapted.substr( 0, splitIndex );
    input_to_be_adapted = (splitIndex == string::npos) ? "" : input_to_be_adapted.substr( splitIndex + 1 );

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

// for convenience
bool IsNumeric( char c ) { return ('0' <= c && c <= '9'); }

// for convenience - create a new node using the parameters and return a pointer to it
SignalType *CreateNode( int type, int val, SignalType *par ) {
    SignalType *node = new SignalType;
    node->nType = type;
    node->nValue = val;
    node->pParent = par;
    return node;
}

// Parse the signal modeled in sSig, and create a tree from it.
// The function is recursive. Using pPar you can pass a pointer of the parent of this tree
// (use nullptr when creating from the root up).
// NOTE - the sSig parameter is consumed by the parsing process!
SignalType *SignalParse( string &sSig, SignalType *pPar ) {

    SignalType *root = nullptr;
    if (sSig.length() > 0) {
        if (IsNumeric( sSig[0] )) {
            // integer encountered - isolate string that represent it and build INTGR type node from it
            int sLen;
            for (sLen = 1; sLen < (int)sSig.length() && IsNumeric( sSig[sLen] ); sLen++) /* count length of numeric substring */;
            string sInteger = get_token_sized( sLen, sSig );
            int    nInteger = atoi( sInteger.c_str());
            root = CreateNode( INTGR, nInteger, pPar );
        }
        if (sSig[0] == '[') {
            // list encountered - isolate string that is within the matching brackets and parse it - building a LIST type node from it
            // the function is called recursively to create the subtrees that go within the LIST type node
            int nrLeftBrackets = 1;
            int sLen;
            for (sLen = 1; sLen < (int)sSig.length() && nrLeftBrackets > 0; sLen++) {
                switch (sSig[sLen]) {
                    case '[': nrLeftBrackets += 1; break;
                    case ']': nrLeftBrackets -= 1; break;
                }
            }
            sLen -= 2;  // compensate string length for both bracket characters
            string sIgnore1 = get_token_sized( 1, sSig );   // remove leading '['
            string sListToken = get_token_sized( sLen, sSig );
            string sIgnore2 = get_token_sized( 1, sSig );   // remove trailing ']'

            // now process the list node and it's children by recursive calls
            root = CreateNode( LIST, 0, pPar );
            while (sListToken.length() > 0) {

                SignalType *pNode = SignalParse( sListToken, root );
                root->vList.push_back( pNode );
                if (sListToken.length() > 0 && sListToken[0] == ',')
                    string sIgnore3 = get_token_sized( 1, sListToken );   // remove ','
            }
        }
    }
    return root;
}

void DisposeSignalTree( SignalType *tree ) {
    for (auto &e : tree->vList) {
        DisposeSignalTree( e );
    }
    delete tree;
}

// for pretty printing
int glbIndentLevel = 0;
void Indent() { for (int i = 0; i < glbIndentLevel; i++) { cout << " "; } }

void PrintSignalTree( SignalType *tree ) {
    if (tree != nullptr) {
        Indent();
        // print info on current node
        cout << "type = " << TypeToString( tree->nType ) << ", value = " << tree->nValue << ", nr children = " << tree->vList.size() << endl;
        // print info on children (if any)
        glbIndentLevel += 2;
        for (int i = 0; i < (int)tree->vList.size(); i++) {
            PrintSignalTree( tree->vList[i] );
        }
        glbIndentLevel -= 2;
    }
}

#define RIGHTORDER 0   // possible outcome (result) of a comparison
#define WRONGORDER 1
#define UNDECIDED  2

string ResultToString( int nRes ) {
    string sRes;
    switch (nRes) {
        case RIGHTORDER: sRes = "RIGHTORDER"; break;
        case WRONGORDER: sRes = "WRONGORDER"; break;
        case UNDECIDED : sRes = "UNDECIDED";  break;
        default:         sRes = "UNKNOWN";
    }
    return sRes;
}

int CompareSignalTrees( SignalType *pLeft, SignalType *pRight ) {

//compIndent(); cout << "comparing ";
//compIndent(); PrintSignalTree( pLeft );
//compIndent(); cout << "and       ";
//compIndent(); PrintSignalTree( pRight );

    int result;
    if (pLeft == nullptr && pRight == nullptr) {
        result = UNDECIDED;
    } else if (pLeft == nullptr && pRight != nullptr) {
        result = RIGHTORDER;
    } else if (pLeft != nullptr && pRight == nullptr) {
        result = WRONGORDER;
    } else {
        // both pointers are not nullptr, compare content
        if (pLeft->nType == INTGR) {
            if (pRight->nType == INTGR) {
                // both are integers
                if (pLeft->nValue < pRight->nValue) { result = RIGHTORDER; } else
                if (pLeft->nValue > pRight->nValue) { result = WRONGORDER; } else
                                                    { result = UNDECIDED;  }
            } else {
                // left is integer, right is list - convert left to list and compare again
                SignalType *newNode = CreateNode( LIST, 0, nullptr );
                newNode->vList.push_back( pLeft );
                glbIndentLevel += 2;
                result = CompareSignalTrees( newNode, pRight );
                glbIndentLevel -= 2;

                delete newNode;
            }
        } else {
            if (pRight->nType == LIST) {
                // both are lists

                result = UNDECIDED;
                // try to find outcome by comparing elements
                int nCounter = 0;
                bool bDone = false;
                int leftListLen  = (int)pLeft->vList.size();
                int rightListLen = (int)pRight->vList.size();
                while (nCounter < leftListLen && nCounter < rightListLen && !bDone) {

                    glbIndentLevel += 2;
                    result = CompareSignalTrees( pLeft->vList[nCounter], pRight->vList[nCounter] );
                    glbIndentLevel -= 2;

                    if (result != UNDECIDED) {
                        bDone = true;
                    } else {
                        nCounter += 1;
                    }
                }
                // result could still be undecided, check the length of the lists
                if (result == UNDECIDED) {
                    if (leftListLen < rightListLen) {
                        result = RIGHTORDER;
                    } else if (leftListLen > rightListLen) {
                        result = WRONGORDER;
                    } else {
                        result = UNDECIDED;
                    }
                }
            } else {
                // left is list, right is integer - convert right to list and compare again
                SignalType *newNode = CreateNode( LIST, 0, nullptr );
                newNode->vList.push_back( pRight );

                glbIndentLevel += 2;
                result = CompareSignalTrees( pLeft, newNode );
                glbIndentLevel -= 2;

                delete newNode;
            }
        }
    }

//compIndent(); cout << "result of comparison: " << result_to_string( result ) << endl;

    return result;
}

// ==========   MAIN()

typedef struct sPacketStruct {
    string sSig;
    SignalType *pSig = nullptr;
} PacketType;

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

    // I'm using an aux string variable because the parsing "eats up" the signal string
    // and I need it for part 2
    string aux;

    int nCumulateIndices = 0;
    for (int i = 0; i < (int)inputData.size(); i++) {
        DatumType &curRec = inputData[i];
        aux = curRec.sSig1; curRec.pSig1 = SignalParse( aux, nullptr );
        aux = curRec.sSig2; curRec.pSig2 = SignalParse( aux, nullptr );
        int nCompRes = CompareSignalTrees( curRec.pSig1, curRec.pSig2 );

        if (nCompRes == RIGHTORDER) {
            nCumulateIndices += i + 1;
        }
    }
    cout << endl << "Answer 1 - accumulated indices: " << nCumulateIndices << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // get all packets into a vector, including their pointers to the packet trees that were
    // parsed in part 1
    vector<PacketType> vPackets;

    for (int i = 0; i < (int)inputData.size(); i++) {

        string sInput;
        sInput = inputData[i].sSig1;
        SignalType *pInput = inputData[i].pSig1;
        vPackets.push_back( { sInput, pInput } );

        sInput = inputData[i].sSig2;
        pInput = inputData[i].pSig2;
        vPackets.push_back( { sInput, pInput } );
    }
    // add the divider packets
    string sDivider1 = "[[2]]";
    SignalType *pInput;
    aux = sDivider1; pInput = SignalParse( aux, nullptr );
    vPackets.push_back( { sDivider1, pInput } );
    string sDivider2 = "[[6]]";
    aux = sDivider2; pInput = SignalParse( aux, nullptr );
    vPackets.push_back( { sDivider2, pInput } );

    // sort the packets vector using the compare function on the parsed signal trees
    sort( vPackets.begin(), vPackets.end(), []( PacketType a, PacketType b ) {
            return (CompareSignalTrees( a.pSig, b.pSig ) == RIGHTORDER);
        }
    );
    // collect dividers indices
    int nIndex1 = -1, nIndex2 = -1;
    for (int i = 0; i < (int)vPackets.size(); i++) {
        if (vPackets[i].sSig == sDivider1) { nIndex1 = i + 1; }
        if (vPackets[i].sSig == sDivider2) { nIndex2 = i + 1; }
    }
    cout << endl << "Answer 2 - index first divider: " << nIndex1 << " index second divider: " << nIndex2 << " decoder key = " << nIndex1 * nIndex2 << endl << endl;

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
