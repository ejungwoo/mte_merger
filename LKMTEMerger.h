#ifndef LKMTEMERGER_HH
#define LKMTEMERGER_HH

#include "TTree.h"
#include <vector>

#define NUM_MTE_SOURCES 10

#ifndef LILAK_VERSION
#define e_info cout<<"\033[0;32m==\033[0m "
#define e_error cout<<"\033[0;31mER\033[0m "
#define e_test cout<<"\033[0;36mtest\033[0m "
//#define coutn cout<<"\033[0;36mNT\033[0m "
//#define couti cout<<"\033[0;32m==\033[0m "
//#define coutw cout<<"\033[0;33mWR\033[0m "
//#define coute cout<<"\033[0;31mER\033[0m "
#endif

/// Master trigger electronics
class LKMTEMerger
{
    public:
        LKMTEMerger(TString outputFileName);
        virtual ~LKMTEMerger() {};

        void SetKeyChannel  (int no, TString name); ///< Set Key channel bit number and name
        void SetInputChannel(int no, TString name); ///< Set channel bit number and name (not key channel)
        void GetTimeOffset  (TString fileName="data/mte_time_offset.txt"); ///< Read and store time offset data from fileName
        void FindTimeOffset (TString fileName="data/mte_time_offset.txt"); ///< Read single source trees and find time offset between them, and save out to fileName
        bool ReadMTE        (TString inputFileName); ///< Read MTE to create separate trees for different sources
        bool MapKobra       (TString fileName, TString kobraName="Kobra"); ///< Read Kobra DAQ data and match time-stamp with MTE Kobra tree by matching time-diffs between events
        void WriteSummary   (); ///< Write summary tree with entry matching data
        bool ReadKobra      (TString fileName, TString kobraName="Kobra"); ///< Read Kobra DAQ to find matching entry of the Key source entry

        int GetKobraEntry(int i); ///< XXX should be written in feature

    public:
        void SetTimeWindowFactor(double factor) { fTimeWindowFactor = factor; }

    private:
        TString fOutputFileName = "";

        int fKeyChannelNumber;
        TString fInputChannelName[NUM_MTE_SOURCES];
        std::vector<int> fInputChannelArray;
        int fEntryOffset[NUM_MTE_SOURCES] = {0};
        double fMTETimeOffset[NUM_MTE_SOURCES] = {0};

        // MTE tree
        int    bTrigNum = 0;
        int    bTrigType = 0;
        double bTrigTime = 0;

        // Summary tree
        int    bKeyNum  = 0;
        double bKeyTime = 0;
        int    bKeyType = 0;
        int    bMTEEntry[NUM_MTE_SOURCES] = {0};
        int    bDAQEntry[NUM_MTE_SOURCES] = {0};

        TTree* fMTETree[NUM_MTE_SOURCES];
        int fMTEEntries[NUM_MTE_SOURCES];
        int fMaxEntries = 0;

        double fTimeWindowFactor = 1./50000;

        TFile *fOutputFile = nullptr;
        TTree *fTreeSummary = nullptr;

#ifndef LILAK_VERSION
    ClassDef(LKMTEMerger, 1)
#endif
};

#endif
