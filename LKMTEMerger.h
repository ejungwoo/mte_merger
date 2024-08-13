#ifndef LKMTEMERGER_HH
#define LKMTEMERGER_HH

#include "TTree.h"
#include <vector>

#define NUM_MTE_INPUT_CHANNELS 10

/// Master trigger electronics
class LKMTEMerger
{
    public:
        LKMTEMerger();
        virtual ~LKMTEMerger() {};

        void SetKeyChannel(int no, TString name);
        void SetInputChannel(int no, TString name);
        bool ReadMTE(TString inputFileName, TString outputFileName);
        bool MapKobra(TString fileName, TString kobraName="Kobra");
        void GetTimeOffset();
        void FindTimeOffset();
        void WriteSummaryTree();
        bool ReadKobra(TString fileName, TString kobraName="Kobra");

        int GetKobraEntry(int i);

    private:
        int trig_num;
        double trig_time;
        int trig_type;

        bool fInitialized = false;

        TTree* fMTETree[NUM_MTE_INPUT_CHANNELS];
        int fMTEEntries[NUM_MTE_INPUT_CHANNELS];
        int fMaxEntries = 0;
        TTree* fCoboTree;

        int fKeyChannelNumber;
        TString fInputChannelName[NUM_MTE_INPUT_CHANNELS];
        std::vector<int> fInputChannelArray;

        TFile *fOutputFile = nullptr;
        TTree *fTreeSummary = nullptr;

        int fNumEntriesKobra = 0;

        int fEntryOffset[NUM_MTE_INPUT_CHANNELS] = {0};
        double fMTETimeOffset[NUM_MTE_INPUT_CHANNELS] = {0};

    //ClassDef(LKMTEMerger, 1)
};

#endif
