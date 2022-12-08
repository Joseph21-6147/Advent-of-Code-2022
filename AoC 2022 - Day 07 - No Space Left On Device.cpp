// AoC 2022 - day 07 - No Space Left On Device
// ===========================================

// date:  2022-12-07
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

// the input data consists of 'commands' and 'command output' of various forms - both modeled as string
typedef string DatumType;
typedef vector<DatumType> DataStream;

// ==========   DATA INPUT FUNCTIONS

// hardcoded input - to focus on testing the solution
void GetData_EXAMPLE( DataStream &dData ) {
    DatumType aux;

    aux = "$ cd /";         dData.push_back( aux );
    aux = "$ ls";           dData.push_back( aux );
    aux = "dir a";          dData.push_back( aux );
    aux = "14848514 b.txt"; dData.push_back( aux );
    aux = "8504156 c.dat";  dData.push_back( aux );
    aux = "dir d";          dData.push_back( aux );
    aux = "$ cd a";         dData.push_back( aux );
    aux = "$ ls";           dData.push_back( aux );
    aux = "dir e";          dData.push_back( aux );
    aux = "29116 f";        dData.push_back( aux );
    aux = "2557 g";         dData.push_back( aux );
    aux = "62596 h.lst";    dData.push_back( aux );
    aux = "$ cd e";         dData.push_back( aux );
    aux = "$ ls";           dData.push_back( aux );
    aux = "584 i";          dData.push_back( aux );
    aux = "$ cd ..";        dData.push_back( aux );
    aux = "$ cd ..";        dData.push_back( aux );
    aux = "$ cd d";         dData.push_back( aux );
    aux = "$ ls";           dData.push_back( aux );
    aux = "4060174 j";      dData.push_back( aux );
    aux = "8033020 d.log";  dData.push_back( aux );
    aux = "5626152 d.ext";  dData.push_back( aux );
    aux = "7214296 k";      dData.push_back( aux );
}

// file input - this function reads text file one line at a time - adapt code to match your need for line parsing!
void ReadInputData( const string sFileName, DataStream &vData ) {

    ifstream dataFileStream( sFileName );
    vData.clear();
    string sLine;
    DatumType datum;

    while (getline( dataFileStream, sLine )) {
        if (sLine.length() > 0) {    // non empty line
            datum = sLine;
            vData.push_back( datum );
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
    // display to console if so desired (for debugging)
    if (bDisplay) {
        PrintDataStream( dData );
    }
}

// ==========   PUZZLE SPECIFIC SOLUTIONS


#define NONE -1    // constants for node types
#define DIR   0
#define FILE  1

// to build a "directory tree"
//   * each node has a type (can be either DIR or FILE)
//   * each FILE has a size
//   * each DIR has 0 or more children
//   * each node has a parent, only the root node has nullptr parent
typedef struct sNode {
    int type = NONE;
    string sName = "<none>";
    int size = 0;
    vector<struct sNode> kids;
    // NOTE - typically it's bad practice to use pointers to vector elements
    //        In this particular case it's know upfront that these vectors are
    //        filled once and don't change anymore.
    struct sNode *parent;
} NodeType;

NodeType glb_root = { DIR, "/", 0, {}, nullptr };   // the root is a global var
NodeType *cur_dir = &glb_root;                      // current dir is kept as a pointer

// convenience function
void InitNode( NodeType &node, int tpe, string name, int sze, NodeType *par ) {
    node.type   = tpe;
    node.sName  = name;
    node.size   = sze;
    node.parent = par;
    node.kids.clear();
    if (tpe == DIR) {
        node.kids.reserve( 100 );   // to mitigate risk that a node is reallocated with a growing vector
    }
}

// Cuts of and returns the front token from "input_to_be_adapted", using "delim" as delimiter.
// If delimiter is not found, the complete input string is passed as a token.
// The input string becomes shorter as a result, and may even become empty
string get_token_dlmtd( const string &delim, string &input_to_be_adapted ) {
    size_t splitIndex = input_to_be_adapted.find( delim );
    string token = input_to_be_adapted.substr( 0, splitIndex );
    input_to_be_adapted = (splitIndex == string::npos) ? "" : input_to_be_adapted.substr( splitIndex + 1 );

    return token;
}

bool IsNumeric( char c ) {
    return ('0' <= c && c <= '9');
}

// The current command output line (as indexed in dData by curCmd) is supposed to be
// the first output line after an ls command. It's output can span multiple lines, so
// parameter curCmd is typically increased as a result of this call.
void ProcessListCmd( DataStream &dData, int &curCmd ) {
    // get current line, but check on the size of the line vector
    string curLine = (curCmd < (int)dData.size()) ? dData[curCmd] : "";
    // process lines until either the buffer is empty or the next line is another command
    while (curCmd < (int)dData.size() && curLine[0] != '$') {
        // split line into 2 tokens
        string token1 = get_token_dlmtd( " ", curLine );
        string token2 = get_token_dlmtd( " ", curLine );

        if (IsNumeric( token1[0] )) {  // token1 denotes a size, token2 denotes a file name
            NodeType newNode;
            InitNode( newNode, FILE, token2, atoi( token1.c_str()), cur_dir );
            cur_dir->kids.push_back( newNode );

        } else {                       // token1 equals "dir", token2 denotes a dir name
            NodeType newNode;
            InitNode( newNode, DIR, token2, 0, cur_dir );
            cur_dir->kids.push_back( newNode );
        }
        curCmd += 1;
        curLine = (curCmd < (int)dData.size()) ? dData[curCmd] : "";
    }
}

void ProcessCdCmd( DataStream &dData, int &curCmd, string dirName ) {
    if (dirName.length() == 0) {
        cout << "ERROR: ProcessCdCmd() --> argument is empty " << endl;
    } else {
        if (dirName == "/") {          // set cur dir to root dir
            cur_dir = &glb_root;
        } else if (dirName == "..") {  // set cur dir to parent dir
            NodeType *aux = cur_dir;
            cur_dir = aux->parent;
        } else {                       // set cur dir to named child dir
            int  nFound = -1;
            for (int i = 0; i < (int)cur_dir->kids.size() && nFound == -1; i++) {
                if (cur_dir->kids[i].sName == dirName) {
                    nFound = i;
                }
            }
            if (nFound == -1) {
                cout << "ERROR: ProcessCdCmd() --> can't find subdir: " << dirName << endl;
            } else {
                cur_dir = &(cur_dir->kids[nFound]);
            }
        }
    }
}

void ProcessCommand( DataStream &dData, int &curCmd ) {
    string command = dData[curCmd];
    if (command[0] != '$') {
        cout << "ERROR: ProcessCommand() --> argument line isn't a command: " << command << endl;
    } else {
        string ignore = get_token_dlmtd( " ", command );   // get rid of '$'
        string token1 = get_token_dlmtd( " ", command );   // either "ls" or "cd"
        string token2 = get_token_dlmtd( " ", command );   // empty if "ls", contains dir name if "cd"
        if (token1 == "ls") {
            curCmd += 1;
            ProcessListCmd( dData, curCmd );
        } else if (token1 == "cd") {
            ProcessCdCmd( dData, curCmd, token2 );
            curCmd += 1;
        } else {
            cout << "ERROR: ProcessCommand() --> command token not recognized: " << token1 << endl;
        }
    }
}

int glbIndentCntr = 0;    // global var used in PrintDirTree() - stores indentation level

void PrintDirTree( NodeType *tree ) {
    if (tree == nullptr) {
        cout << "ERROR: PrintDirTree() --> nullptr argument" << endl;
    } else {
        if (tree->type == NONE) {
            cout << "ERROR: PrintDirTree() --> empty node encountered" << endl;
        } else {
            // first print the current node
            for (int i = 0; i < glbIndentCntr; i++) cout << " ";
            cout << "- " << tree->sName;
            if (tree->type == FILE) {
                cout << " (file, size=" << tree->size << ")" << endl;
            } else if (tree->type == DIR) {
                cout << " (dir)" << endl;
                // if the node is a directory type node, also print the kids by recursive calls
                glbIndentCntr += 2;
                for (int i = 0; i < (int)tree->kids.size(); i++) {
                    PrintDirTree( &(tree->kids[i]));
                }
                glbIndentCntr -= 2;
            }
        }
    }
}

// processes all the lines in dData
void ProcessCommandList( DataStream &dData ) {
    int cmdCounter = 0;
    while (cmdCounter < (int)dData.size()) {
        ProcessCommand( dData, cmdCounter );
    }
}

// ===== STUFF TO ANALYSE TREE SIZES

// returns the size of the (sub) tree pointed at by *tree
// Note I'm using long long to prevent risk of overflow or smth
long long DirSize( NodeType *tree ) {
    long long result = 0;
    // filter out obviouis error cases
    if (tree == nullptr) {
        cout << "ERROR: DirSize() --> nullptr argument" << endl;
    } else if (tree->type == NONE) {
        cout << "ERROR: DirSize() --> empty node encountered" << endl;
    } else {

        if (tree->type == FILE) {        // if node is a file, it's size is the size of the file
            result += tree->size;
        } else if (tree->type == DIR) {  // if node is a directory, it's size is the sum of the sizes of it's children
            for (int i = 0; i < (int)tree->kids.size(); i++) {
                result += DirSize( &(tree->kids[i]));
            }
        }
    }
    return result;
}

typedef struct sAnalyze {   // struct to contain dir name / size combinations
    string sDirName;
    long long llDirSize;
} AnalyseType;
vector<AnalyseType> vAnalyseData;

// walk the tree pointed at by *tree and fill the vAnalyseData list from the tree walk
void GatherDirSizes( NodeType *tree ) {
    // make a new analysis entry for each directory
    if (tree->type == DIR) {
        AnalyseType rec = { tree->sName, DirSize( tree ) };
        vAnalyseData.push_back( rec );
        // walk the tree recursively for all children of each directory
        for (int i = 0; i < (int)tree->kids.size(); i++) {
            GatherDirSizes( &(tree->kids[i]));
        }
    }
}

// ==========   MAIN()

int main()
{
    glbProgPhase = PUZZLE;     // program phase to EXAMPLE, TEST or PUZZLE

    flcTimer tmr;
    tmr.StartTiming(); // ============================================vvvvv

    // get input data, depending on the glbProgPhase (example, test, puzzle)
    DataStream cmdData;
    GetInput( cmdData, glbProgPhase != PUZZLE );
    cout << "Data stats - size of data stream " << cmdData.size() << endl << endl;

    tmr.TimeReport( "Timing data input: " );   // ====================^^^^^vvvvv

// ========== part 1

    // process all commands in the input
    ProcessCommandList( cmdData );
    if (glbProgPhase != PUZZLE) {
        PrintDirTree( &glb_root );
        cout << endl << endl;
    }
    // fill the vAnalyseData vector with sizes per directory
    GatherDirSizes( &glb_root );
    // accumulate sizes of all directories that have a size <= 100000
    long long answer1 = 0;
    for (int i = 0; i < (int)vAnalyseData.size(); i++) {
        if (glbProgPhase != PUZZLE) {
            cout << "directory: " << vAnalyseData[i].sDirName << " has size " << vAnalyseData[i].llDirSize << endl;
        }
        if (vAnalyseData[i].llDirSize <= 100000) {
            answer1 += vAnalyseData[i].llDirSize;
        }
    }
    cout << endl << "Answer 1 - accumulated dir size <= 100000: " << answer1 << endl << endl;

    tmr.TimeReport( "Timing 1: " );   // ==============================^^^^^

// ========== part 2

    // sort analysis data in descending order
    sort( vAnalyseData.begin(), vAnalyseData.end(),
         [](AnalyseType a, AnalyseType b) {
            return a.llDirSize > b.llDirSize;
         }
    );
    // work out what space we need to free
    long long spaceOccupied  = vAnalyseData[0].llDirSize;   // vAnalyse[0] contains root due to sorting
    long long spaceTotal     = 70000000;
    long long spaceAvailable = spaceTotal - spaceOccupied;
    long long spaceNeeded    = 30000000 - spaceAvailable;

    int foundIndex = -1;
    for (int i = 1; i < (int)vAnalyseData.size() && foundIndex == -1; i++) {
        if (vAnalyseData[i].llDirSize < spaceNeeded && vAnalyseData[i-1].llDirSize > spaceNeeded) {
            foundIndex = i - 1;
        }
    }
    cout << endl << "Answer 2 - size of smallest directory to free in order to get space needed: " << vAnalyseData[foundIndex].llDirSize
                 << " (this is directory: " << vAnalyseData[foundIndex].sDirName << ")" << endl << endl;

    tmr.TimeReport( "Timing 2: " );   // ==============================^^^^^

    return 0;
}
