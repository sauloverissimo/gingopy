// Gingo — Music Theory Library
// Implementation of LookupTree singleton — harmonic tree lookup tables.
//
// SPDX-License-Identifier: MIT

#include "gingo/internal/lookup_tree.hpp"

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Singleton (Meyer's pattern — thread-safe in C++11 and later)
// ---------------------------------------------------------------------------

const LookupTree& LookupTree::instance() {
    static LookupTree tree;
    return tree;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

LookupTree::LookupTree() {
    init_branches();
    init_paths();
}

// ---------------------------------------------------------------------------
// Tree branches — 68 branch definitions
// ---------------------------------------------------------------------------

void LookupTree::init_branches() {
    branches_ = Table({"scaleIndex", "branch"});

    branches_.data = {
        // ---------------------------------------------------------------
        // Major scale branches (scaleIndex = 0) — 29 entries
        // ---------------------------------------------------------------
        // Diatonic chords
        {0, std::string("I")},                   // 0

        // Applied chords (tonicization)
        {0, std::string("IIm / IV")},            // 1
        {0, std::string("IIm7(b5) / IIm")},      // 2  [ADVANCED: rare progression]
        {0, std::string("IIm7(11) / IV")},       // 3  [ADVANCED: jazz voicing]
        {0, std::string("SUBV7 / IV")},          // 4
        {0, std::string("V7 / IV")},             // 5
        {0, std::string("VIm")},                 // 6
        {0, std::string("V7 / IIm")},            // 7
        {0, std::string("Idim")},                // 8
        {0, std::string("#Idim")},               // 9
        {0, std::string("bIIIdim")},             // 10
        {0, std::string("IV#dim")},              // 11
        {0, std::string("IV")},                  // 12
        {0, std::string("V7 / V")},              // 13  [CORRECTED: was "V7/V7"]
        {0, std::string("IIm")},                 // 14
        // Modal interchange (borrowed from parallel minor)
        {0, std::string("IVm")},                 // 15
        {0, std::string("bVI")},                 // 16
        {0, std::string("bVII")},                // 17
        {0, std::string("IIm7(b5)")},            // 18  [ADVANCED: borrowed from harmonic minor]
        {0, std::string("II#dim")},              // 19
        {0, std::string("SUBV7")},               // 20
        {0, std::string("V7")},                  // 21
        {0, std::string("V7 / VI")},             // 22
        {0, std::string("V7 / Im")},             // 23  [ADVANCED: secondary dominant of borrowed chord]
        {0, std::string("V7 / V")},              // 24
        {0, std::string("V7 / III")},            // 25
        {0, std::string("V7 / IV")},             // 26
        {0, std::string("V7 / bIII")},           // 27  [ADVANCED: secondary dominant of borrowed chord]
        {0, std::string("V7 / bVI")},            // 28  [ADVANCED: secondary dominant of borrowed chord]

        // ---------------------------------------------------------------
        // Natural minor branches (scaleIndex = 1) — 13 entries
        // ---------------------------------------------------------------
        {1, std::string("Im")},                  // 29
        {1, std::string("IIm7(b5) / Ivm")},      // 30
        {1, std::string("IVm7 / IVm")},          // 31
        {1, std::string("V / IVm")},             // 32
        {1, std::string("V / V")},               // 33
        {1, std::string("bVI / Im")},            // 34
        {1, std::string("II / IIm")},            // 35
        {1, std::string("IVm")},                 // 36
        {1, std::string("bV / V")},              // 37
        {1, std::string("IV#dim")},              // 38
        {1, std::string("V7 / I")},              // 39
        {1, std::string("V7 / III")},            // 40
        {1, std::string("V7 / V")},              // 41
        {1, std::string("bVII")},                // 42  [NEW: common in rock/jazz]
        {1, std::string("bIII")},                // 43  [NEW: relative major]
        {1, std::string("Vm")},                  // 44  [NEW: diatonic minor V]

        // ---------------------------------------------------------------
        // Harmonic minor branches (scaleIndex = 2) — 16 entries
        // ---------------------------------------------------------------
        {2, std::string("Im")},                  // 45
        {2, std::string("IIm7(b5) / Ivm")},      // 46
        {2, std::string("IVm7 / IVm")},          // 47
        {2, std::string("V / IVm")},             // 48
        {2, std::string("V / V")},               // 49
        {2, std::string("bVI / Im")},            // 50
        {2, std::string("II / IIm")},            // 51
        {2, std::string("IVm")},                 // 52
        {2, std::string("bV / V")},              // 53
        {2, std::string("IV#dim")},              // 54
        {2, std::string("V7 / I")},              // 55
        {2, std::string("V7 / III")},            // 56
        {2, std::string("V7 / V")},              // 57
        {2, std::string("bVII")},                // 58  [NEW: common in rock/jazz]
        {2, std::string("bIII")},                // 59  [NEW: relative major]

        // ---------------------------------------------------------------
        // Melodic minor branches (scaleIndex = 3) — 15 entries
        // ---------------------------------------------------------------
        {3, std::string("Im")},                  // 60
        {3, std::string("IIm7(b5) / Ivm")},      // 61
        {3, std::string("IVm7 / IVm")},          // 62
        {3, std::string("V / IVm")},             // 63
        {3, std::string("V / V")},               // 64
        {3, std::string("bVI / Im")},            // 65
        {3, std::string("II / IIm")},            // 66
        {3, std::string("IVm")},                 // 67
        {3, std::string("bV / V")},              // 68
        {3, std::string("IV#dim")},              // 69
        {3, std::string("V7 / I")},              // 70
        {3, std::string("V7 / III")},            // 71
        {3, std::string("V7 / V")},              // 72
        {3, std::string("bVII")},                // 73  [NEW: common in rock/jazz]
        {3, std::string("bIII")}                 // 74  [NEW: relative major]
    };
}

// ---------------------------------------------------------------------------
// Tree paths — 57 path connections between branches
// ---------------------------------------------------------------------------

void LookupTree::init_paths() {
    paths_ = Table({"scaleIndex","originIndex","targetIndex",
                     "branchOrigin","branchTarget"});

    paths_.data = {
        {0,0,0,std::string("I"),std::string("I")},                                        // 0
        {0,0,1,std::string("I"),std::string("IIm / IV")},                                 // 1
        {0,0,2,std::string("I"),std::string("IIm7(b5) / IIm")},                           // 2
        {0,0,3,std::string("I"),std::string("IIm7(11) / IV")},                            // 3
        {0,0,6,std::string("I"),std::string("VIm")},                                      // 4
        {0,0,9,std::string("I"),std::string("#Idim")},                                  // 5
        {0,0,10,std::string("I"),std::string("bIIIdim")},                               // 6
        {0,0,21,std::string("I"),std::string("V7")},                                      // 7
        {0,0,22,std::string("I"),std::string("V7 / VI")},                                 // 8
        {0,1,5,std::string("IIm / IV"),std::string("V7 / IV")},                           // 9
        {0,2,7,std::string("IIm7(b5) / IIm"),std::string("V7 / IIm")},                   // 10
        {0,3,4,std::string("IIm7(11) / IV"),std::string("SUBV7 / IV")},                   // 11
        {0,4,12,std::string("SUBV7 / IV"),std::string("IV")},                             // 12
        {0,5,12,std::string("V7 / IV"),std::string("IV")},                                // 13
        {0,6,12,std::string("VIm"),std::string("IV")},                                    // 14
        {0,7,14,std::string("V7 / IIm"),std::string("IIm")},                              // 15
        {0,8,14,std::string("Idim"),std::string("IIm")},                                // 16
        {0,9,14,std::string("#Idim"),std::string("IIm")},                               // 17
        {0,10,14,std::string("bIIIdim"),std::string("IIm")},                            // 18
        {0,12,11,std::string("IV"),std::string("IV#dim")},                              // 19
        {0,11,21,std::string("IV#dim"),std::string("V7")},                              // 20
        {0,11,0,std::string("IV#dim"),std::string("I")},                                // 21
        {0,12,15,std::string("IV"),std::string("IVm")},                                   // 22
        {0,12,16,std::string("IV"),std::string("bVI")},                                   // 23
        {0,12,17,std::string("IV"),std::string("bVII")},                                  // 24
        {0,12,21,std::string("IV"),std::string("V7")},                                    // 25
        {0,13,14,std::string("V7 / V"),std::string("IIm")},                                // 26
        {0,13,21,std::string("V7 / V"),std::string("V7")},                                 // 27
        {0,14,21,std::string("IIm"),std::string("V7")},                                   // 28
        {0,15,21,std::string("IVm"),std::string("V7")},                                   // 29
        {0,16,0,std::string("bVI"),std::string("I")},                                     // 30
        {0,16,21,std::string("bVI"),std::string("V7")},                                   // 31
        {0,17,0,std::string("bVII"),std::string("I")},                                    // 32
        {0,17,21,std::string("bVII"),std::string("V7")},                                  // 33
        {0,18,0,std::string("IIm7(b5)"),std::string("I")},                                // 34
        {0,18,21,std::string("IIm7(b5)"),std::string("V7")},                              // 35
        {0,20,0,std::string("SUBV7"),std::string("I")},                                   // 36
        {0,12,18,std::string("IV"),std::string("IIm7(b5)")},                              // 37
        {0,0,8,std::string("I"),std::string("Idim")},                                   // 38
        {0,7,13,std::string("V7 / IIm"),std::string("V7 / V")},                            // 39
        {0,8,13,std::string("Idim"),std::string("V7 / V")},                              // 40
        {0,9,13,std::string("#Idim"),std::string("V7 / V")},                             // 41
        {0,10,13,std::string("bIIIdim"),std::string("V7 / V")},                          // 42
        {0,13,20,std::string("V7 / V"),std::string("SUBV7")},                              // 43
        {0,14,13,std::string("IIm"),std::string("V7 / V")},                                // 44
        {0,14,15,std::string("IIm"),std::string("IVm")},                                  // 45
        {0,14,16,std::string("IIm"),std::string("bVI")},                                  // 46
        {0,14,17,std::string("IIm"),std::string("bVII")},                                 // 47
        {0,14,18,std::string("IIm"),std::string("IIm7(b5)")},                             // 48
        {0,13,15,std::string("V7 / V"),std::string("IVm")},                                // 49
        {0,13,16,std::string("V7 / V"),std::string("bVI")},                                // 50
        {0,13,17,std::string("V7 / V"),std::string("bVII")},                               // 51
        {0,13,18,std::string("V7 / V"),std::string("IIm7(b5)")},                           // 52
        {0,15,0,std::string("IVm"),std::string("I")},                                     // 53
        {0,21,0,std::string("V7"),std::string("I")},                                      // 54
        {0,11,0,std::string("IV#dim"),std::string("I")},                                // 55
        {0,14,20,std::string("IIm"),std::string("SUBV7")},                                 // 56
        {0,14,19,std::string("IIm"),std::string("II#dim")},                               // 57  [NEW: IIm -> II#dim chromatic]
        {0,19,0,std::string("II#dim"),std::string("I")},                                  // 58  [NEW: II#dim -> I resolution]

        // ---------------------------------------------------------------
        // Natural minor paths (scaleIndex = 1) — based on minor tree
        // ---------------------------------------------------------------
        {1,29,29,std::string("Im"),std::string("Im")},                                    // 57
        {1,29,30,std::string("Im"),std::string("IIm7(b5) / Ivm")},                       // 58
        {1,29,35,std::string("Im"),std::string("II / IIm")},                             // 59
        {1,29,33,std::string("Im"),std::string("V / V")},                                // 60
        {1,29,34,std::string("Im"),std::string("bVI / Im")},                             // 61
        {1,29,39,std::string("Im"),std::string("V7 / I")},                               // 62
        {1,30,31,std::string("IIm7(b5) / Ivm"),std::string("IVm7 / IVm")},              // 63
        {1,31,32,std::string("IVm7 / IVm"),std::string("V / IVm")},                      // 64
        {1,32,36,std::string("V / IVm"),std::string("IVm")},                             // 65
        {1,33,39,std::string("V / V"),std::string("V7 / I")},                            // 66
        {1,34,29,std::string("bVI / Im"),std::string("Im")},                             // 67
        {1,35,36,std::string("II / IIm"),std::string("IVm")},                            // 68
        {1,36,39,std::string("IVm"),std::string("V7 / I")},                              // 69
        {1,36,38,std::string("IVm"),std::string("IV#dim")},                              // 70
        {1,38,39,std::string("IV#dim"),std::string("V7 / I")},                           // 71
        {1,38,29,std::string("IV#dim"),std::string("Im")},                               // 72
        {1,39,29,std::string("V7 / I"),std::string("Im")},                               // 73
        {1,40,29,std::string("V7 / III"),std::string("Im")},                             // 74
        {1,41,39,std::string("V7 / V"),std::string("V7 / I")},                           // 75
        {1,29,42,std::string("Im"),std::string("bVII")},                                 // 76  [NEW: Im-bVII typical]
        {1,42,43,std::string("bVII"),std::string("bIII")},                               // 77  [NEW: bVII-bIII]
        {1,43,36,std::string("bIII"),std::string("IVm")},                                // 78  [NEW: bIII-IVm]
        {1,43,39,std::string("bIII"),std::string("V7 / I")},                             // 79  [NEW: bIII-V7/I]
        {1,42,36,std::string("bVII"),std::string("IVm")},                                // 80  [NEW: bVII-IVm]
        {1,44,29,std::string("Vm"),std::string("Im")},                                   // 81  [NEW: Vm-Im (authentic)]

        // ---------------------------------------------------------------
        // Harmonic minor paths (scaleIndex = 2) — similar to natural minor
        // ---------------------------------------------------------------
        {2,45,45,std::string("Im"),std::string("Im")},                                    // 82
        {2,45,46,std::string("Im"),std::string("IIm7(b5) / Ivm")},                       // 83
        {2,45,51,std::string("Im"),std::string("II / IIm")},                             // 84
        {2,45,49,std::string("Im"),std::string("V / V")},                                // 85
        {2,45,50,std::string("Im"),std::string("bVI / Im")},                             // 86
        {2,45,55,std::string("Im"),std::string("V7 / I")},                               // 87
        {2,46,47,std::string("IIm7(b5) / Ivm"),std::string("IVm7 / IVm")},              // 88
        {2,47,48,std::string("IVm7 / IVm"),std::string("V / IVm")},                      // 89
        {2,48,52,std::string("V / IVm"),std::string("IVm")},                             // 90
        {2,49,55,std::string("V / V"),std::string("V7 / I")},                            // 91
        {2,50,45,std::string("bVI / Im"),std::string("Im")},                             // 92
        {2,51,52,std::string("II / IIm"),std::string("IVm")},                            // 93
        {2,52,55,std::string("IVm"),std::string("V7 / I")},                              // 94
        {2,52,54,std::string("IVm"),std::string("IV#dim")},                              // 95
        {2,54,55,std::string("IV#dim"),std::string("V7 / I")},                           // 96
        {2,54,45,std::string("IV#dim"),std::string("Im")},                               // 97
        {2,55,45,std::string("V7 / I"),std::string("Im")},                               // 98
        {2,56,45,std::string("V7 / III"),std::string("Im")},                             // 99
        {2,57,55,std::string("V7 / V"),std::string("V7 / I")},                           // 100
        {2,45,58,std::string("Im"),std::string("bVII")},                                 // 101 [NEW: Im-bVII typical]
        {2,58,59,std::string("bVII"),std::string("bIII")},                               // 102 [NEW: bVII-bIII]
        {2,59,52,std::string("bIII"),std::string("IVm")},                                // 103 [NEW: bIII-IVm]
        {2,59,55,std::string("bIII"),std::string("V7 / I")},                             // 104 [NEW: bIII-V7/I]
        {2,58,52,std::string("bVII"),std::string("IVm")},                                // 105 [NEW: bVII-IVm]

        // ---------------------------------------------------------------
        // Melodic minor paths (scaleIndex = 3) — similar to harmonic minor
        // ---------------------------------------------------------------
        {3,60,60,std::string("Im"),std::string("Im")},                                    // 106
        {3,60,61,std::string("Im"),std::string("IIm7(b5) / Ivm")},                       // 107
        {3,60,66,std::string("Im"),std::string("II / IIm")},                             // 108
        {3,60,64,std::string("Im"),std::string("V / V")},                                // 109
        {3,60,65,std::string("Im"),std::string("bVI / Im")},                             // 110
        {3,60,70,std::string("Im"),std::string("V7 / I")},                               // 111
        {3,61,62,std::string("IIm7(b5) / Ivm"),std::string("IVm7 / IVm")},              // 112
        {3,62,63,std::string("IVm7 / IVm"),std::string("V / IVm")},                      // 113
        {3,63,67,std::string("V / IVm"),std::string("IVm")},                             // 114
        {3,64,70,std::string("V / V"),std::string("V7 / I")},                            // 115
        {3,65,60,std::string("bVI / Im"),std::string("Im")},                             // 116
        {3,66,67,std::string("II / IIm"),std::string("IVm")},                            // 117
        {3,67,70,std::string("IVm"),std::string("V7 / I")},                              // 118
        {3,67,69,std::string("IVm"),std::string("IV#dim")},                              // 119
        {3,69,70,std::string("IV#dim"),std::string("V7 / I")},                           // 120
        {3,69,60,std::string("IV#dim"),std::string("Im")},                               // 121
        {3,70,60,std::string("V7 / I"),std::string("Im")},                               // 122
        {3,71,60,std::string("V7 / III"),std::string("Im")},                             // 123
        {3,72,70,std::string("V7 / V"),std::string("V7 / I")},                           // 124
        {3,60,73,std::string("Im"),std::string("bVII")},                                 // 125 [NEW: Im-bVII typical]
        {3,73,74,std::string("bVII"),std::string("bIII")},                               // 126 [NEW: bVII-bIII]
        {3,74,67,std::string("bIII"),std::string("IVm")},                                // 127 [NEW: bIII-IVm]
        {3,74,70,std::string("bIII"),std::string("V7 / I")},                             // 128 [NEW: bIII-V7/I]
        {3,73,67,std::string("bVII"),std::string("IVm")}                                 // 129 [NEW: bVII-IVm]
    };

    paths_.row_name_to_index = {
        {"I : I",0},
        {"I : IIm / IV",1},
        {"I : IIm7(b5) / IIm",2},
        {"I : IIm7(11) / IV",3},
        {"I : VIm",4},
        {"I : #Idim",5},
        {"I : bIIIdim",6},
        {"I : V7",7},
        {"I : V7 / VI",8},
        {"IIm / IV : V7 / IV",9},
        {"IIm7(b5) / IIm : V7 / IIm",10},
        {"IIm7(11) / IV : SUBV7 / IV",11},
        {"SUBV7 / IV : IV",12},
        {"V7 / IV : IV",13},
        {"VIm : IV",14},
        {"V7 / IIm : IIm",15},
        {"Idim : IIm",16},
        {"#Idim : IIm",17},
        {"bIIIdim : IIm",18},
        {"IV : IV#dim",19},
        {"IV#dim : V7",20},
        {"IV#dim : I",21},
        {"IV : IVm",22},
        {"IV : bVI",23},
        {"IV : bVII",24},
        {"IV : V7",25},
        {"V7/V7 : IIm",26},
        {"V7/V7 : V7",27},
        {"IIm : V7",28},
        {"IVm : V7",29},
        {"bVI : I",30},
        {"bVI : V7",31},
        {"bVII : I",32},
        {"bVII : V7",33},
        {"IIm7(b5) : I",34},
        {"IIm7(b5) : V7",35},
        {"SUBV7 : I",36},
        {"IV : IIm7(b5)",37},
        {"I : Idim",38},
        {"V7 / IIm : V7/V7",39},
        {"Idim : V7/V7",40},
        {"#Idim : V7/V7",41},
        {"bIIIdim : V7/V7",42},
        {"V7/V7 : SUBV7",43},
        {"IIm : V7/V7",44},
        {"IIm : IVm",45},
        {"IIm : bVI",46},
        {"IIm : bVII",47},
        {"IIm : IIm7(b5)",48},
        {"V7/V7 : IVm",49},
        {"V7/V7 : bVI",50},
        {"V7/V7 : bVII",51},
        {"V7/V7 : IIm7(b5)",52},
        {"IVm : I",53},
        {"V7 : I",54},
        {"IV#dim : I",55},
        {"IIm : SUBV7",56},
        // Natural minor
        {"Im : Im",57},
        {"Im : IIm7(b5) / Ivm",58},
        {"Im : II / IIm",59},
        {"Im : V / V",60},
        {"Im : bVI / Im",61},
        {"Im : V7 / I",62},
        {"IIm7(b5) / Ivm : IVm7 / IVm",63},
        {"IVm7 / IVm : V / IVm",64},
        {"V / IVm : IVm",65},
        {"V / V : V7 / I",66},
        {"bVI / Im : Im",67},
        {"II / IIm : IVm",68},
        {"IVm : V7 / I",69},
        {"IVm : IV#dim",70},
        {"IV#dim : V7 / I",71},
        {"IV#dim : Im",72},
        {"V7 / I : Im",73},
        {"V7 / III : Im",74},
        {"V7 / V : V7 / I",75}
    };
}

} // namespace gingo::internal
