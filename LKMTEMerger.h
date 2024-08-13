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

        void SetKeyChannel(int no, TString name);
        void SetInputChannel(int no, TString name);

        bool ReadMTE(TString inputFileName);
        bool MapKobra(TString fileName, TString kobraName="Kobra");

        void GetTimeOffset (TString fileName="data/mte_time_offset.txt");
        void FindTimeOffset(TString fileName="data/mte_time_offset.txt");

        void WriteSummaryTree();
        bool ReadKobra(TString fileName, TString kobraName="Kobra");

        int GetKobraEntry(int i);

    public:
        void SetOffsetFileName(TString name) { fOffsetFileName = name; }
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

        TString fOffsetFileName = "data/mte_time_offset.txt";

#ifndef LILAK_VERSION
    ClassDef(LKMTEMerger, 1)
#endif
};

#endif
