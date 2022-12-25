// AoC 2022 - Day 20 - Grove Positioning System
// ============================================

// date:  2022-12-20
// by:    Joseph21 (Joseph21-6147)

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>

#include "../flcTimer.h"

using namespace std;

// ==========   PROGRAM PHASING

enum eProgPhase {     // what programming phase are you in - set at start of main()
    EXAMPLE = 0, TEST, PUZZLE
} glbProgPhase;

// ==========   INPUT DATA STRUCTURES          <<<<< ========== adapt to match columns of input file

// the input data consists of 'numbers' having a first and second selection
typedef int DatumType;
typedef vector<DatumType> DataStream;

// ==========   INPUT DATA FUNCTIONS

// hardcoded input - just to get the solution tested
void GetData_EXAMPLE( DataStream &dData ) {
    dData = { 1, 2, -3, 3, -2, 0, 4 };
}

// file input - this function reads text file content one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {
    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;
    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            datum = atoi( sLine.c_str() );
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

// The puzzle data structure is a std::list type container, which contains pointers to list nodes.
// These listnodes contain the actual data (int for part 1, changed to long long for part 2).
// By storing pointers in the list I can add a std::vector which also contains nodes to listpointers,
// and which is used to retain the original ordering of the list.
// So all the mixing is done in the list of MListType, while the original ordering is retained in
// the OListTYpe.
typedef struct sListNodeStruct {
    long long theNumber;
} NodeType;
typedef vector<NodeType *> OListType;     // this one is to preserve the initial ordering
typedef list<NodeType *>   MListType;     // this is the mixed version
typedef list<NodeType *>::iterator  MListItrType;     // this is the iterator type of the mixed version list


// Reads the input data (from file) into the combined lists datastructure.
// No processing of any kind.
void InitList_part1( DataStream &iData, OListType &oList, MListType &mList ) {
    // put all initial data in the list combination structure
    for (int i = 0; i < (int)iData.size(); i++) {
        NodeType *nodePtr = new NodeType;
        nodePtr->theNumber = (long long)iData[i];
        oList.push_back( nodePtr );
        mList.push_back( nodePtr );
    }
}

// Since we're working with dynamic allocation of list nodes, there must be a way to clear
// the alloc'd memory.
void DisposeList( OListType &oList, MListType &mList ) {
    for (int i = 0; i < (int)oList.size(); i++) {
        delete oList[i];
    }
    mList.clear();
    oList.clear();
}

// Convenience function - since I'm not sure how integer division behaves with type
// long long in combination with negative quotient.
long long myRemainder( long long a, long long b ) {
    long long result = a;
    if (result >= 0) {
        long long llDiv = result / b;
        result -= llDiv * b;
    } else {
        result = -result;
        long long llDiv = result / b;
        result -= llDiv * b;
        result = -result;
    }
    return result;
}

// eltIndex is the index in the ordered list (oList) that must be "mixed" in the mixed list mList.
void MoveElement( OListType &oList, MListType &mList, int eltIndex ) {
    // find pointer to the element to be moved, its value and an iterator to it
    NodeType *curElt = oList[eltIndex];
    long long curEltVal = curElt->theNumber;
    MListItrType itr = find( mList.begin(), mList.end(), curElt );
    // check for error in the call to find()
    if (itr == mList.end()) {
        cout << "ERROR: MoveElement() --> can't find element containing " << curEltVal << endl;
    } else {
        // found succesfully - remove element under iterator and set iterator to next element
        itr = mList.erase( itr );
        if (itr == mList.end()) itr = mList.begin();

        // note that you have to work with the length of mList, since it is 1 shorter than oList atm
        long long nListSize = (long long)mList.size();
        long long nrIterations = curEltVal;
        // bring back number of iterations between the range of list size
        nrIterations = myRemainder( curEltVal, nListSize );

        if (nrIterations >= 0) {
            // do the actual moving (with wrap around)
            for (int i = 0; i < nrIterations; i++) {
                itr++;
                if (itr == mList.end()) itr = mList.begin();
            }
        } else {
            // do the actual moving (with wrap around)
            for (int i = 0; i > nrIterations; i--) {
                if (itr == mList.begin()) {
                    itr = mList.end();
                }
                itr--;
            }
        }
        // after the moving is done, insert the element at the right location in mList
        mList.insert( itr, oList[eltIndex] );
    }
}

// Returns the value of the node that is "indexAfterValue0" positions away (with wrapping around)
// the element with value 0 in mList
long long GetElement( OListType &oList, MListType &mList, int indexAfterValue0 ) {
    int listSize = (int)mList.size();
    int clippedIndex = indexAfterValue0 % listSize;
    // search for the element with value 0
    int findIndex = -1;
    for (int i = 0; i < (int)oList.size() && findIndex < 0; i++) {
        if (oList[i]->theNumber == 0) findIndex = i;
    }
    // check for errors in call to find()
    if (findIndex == -1) {
        cout << "ERROR: GetElement() --> can't find element containing [1] " << 0 << endl;
    }

    // set iterator on the element with value 0 in the mList
    MListItrType itr = find( mList.begin(), mList.end(), oList[findIndex] );
    // check for errors in call to find()
    if (itr == mList.end()) {
        cout << "ERROR: GetElement() --> can't find element containing [2] " << 0 << endl;
    } else {
        // iterator is succesfully set to element with value 0 in mList. Now advance it
        // indexAfterValue0 elements
        for (int i = 0; i < clippedIndex; i++) {
            itr++;
            if (itr == mList.end()) itr = mList.begin();
        }
    }
    // finally return the contents of the list node
    return (*itr)->theNumber;
}

// for testing
void PrintList( OListType &oList, MListType &mList, bool bAlsoOrderedList = false ) {

    if (bAlsoOrderedList) {
        cout << "ordered list = ";
        for (int i = 0; i < (int)oList.size(); i++) {
            cout << oList[i]->theNumber << ", ";
        }
        cout << endl;
    }
    cout << "mixed list   = ";
    for (MListItrType itr = mList.begin(); itr != mList.end(); itr++) {
        cout << (*itr)->theNumber << ", ";
    }
    cout << endl;
}

// For part 2 a little processing is needed on the input data
void InitList_part2( DataStream &iData, OListType &oList, MListType &mList ) {
    long long decryptionKey = 811589153;
    // put all initial data in the list combination structure
    for (int i = 0; i < (int)iData.size(); i++) {
        NodeType *nodePtr = new NodeType;
        nodePtr->theNumber = (long long)iData[i] * decryptionKey;
        oList.push_back( nodePtr );
        mList.push_back( nodePtr );
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

    // Init the data structure from the input data
    OListType olist1;
    MListType mlist1;
    InitList_part1( inputData, olist1, mlist1 );

    // move all elements according to the puzzle rules
    for (int i = 0; i < (int)olist1.size(); i++) {
        MoveElement( olist1, mlist1, i );
    }
    // find the denoted values from the list
    long long nr1000 = GetElement( olist1, mlist1, 1000 );
    long long nr2000 = GetElement( olist1, mlist1, 2000 );
    long long nr3000 = GetElement( olist1, mlist1, 3000 );
    // report the answer
    cout << endl << "Answer 1 - numbers are: " << nr1000 << ", " << nr2000 << " and " << nr3000 << ". Accumulated score: " << nr1000+nr2000+nr3000 << endl << endl;

    DisposeList( olist1, mlist1 );

/* ========== */   tmr.TimeReport( "Timing 1: " );   // =========================^^^^^vvvvv

    // part 2 code here

    // Init the data structure from the input data
    OListType olist2;
    MListType mlist2;
    InitList_part2( inputData, olist2, mlist2 );

    if (glbProgPhase != PUZZLE) {
        cout << "initial arrangement: " << endl;
        PrintList( olist2, mlist2 );
    }

    // iterate ten rounds of mixing
    for (int j = 1; j <= 10; j++) {
        // mix all the elements from the list once
        for (int i = 0; i < (int)olist2.size(); i++) {
            MoveElement( olist2, mlist2, i );
        }
        if (glbProgPhase != PUZZLE) {
            cout << endl << "after finishing round: " << j << endl;
            PrintList( olist2, mlist2 );
        }
    }
    // find the denoted values from the list
    nr1000 = GetElement( olist2, mlist2, 1000 );
    nr2000 = GetElement( olist2, mlist2, 2000 );
    nr3000 = GetElement( olist2, mlist2, 3000 );
    // report the answer
    cout << endl << "Answer 2 - numbers are: " << nr1000 << ", " << nr2000 << " and " << nr3000 << ". Accumulated score: " << nr1000+nr2000+nr3000 << endl << endl;

    DisposeList( olist2, mlist2 );

/* ========== */   tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
