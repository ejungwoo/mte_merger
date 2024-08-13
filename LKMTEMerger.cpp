#include "LKMTEMerger.h"
#include <stdio.h>
#include <iostream>
using namespace std;

#ifndef LILAK_VERSION
ClassImp(LKMTEMerger)
#endif

LKMTEMerger::LKMTEMerger(TString outputFileName)
{
    fOutputFileName = outputFileName;
}

void LKMTEMerger::SetKeyChannel(int no, TString name)
{
    fInputChannelArray.push_back(no);
    fInputChannelName[no] = name;
    fKeyChannelNumber = no;
}

void LKMTEMerger::SetInputChannel(int no, TString name)
{
    fInputChannelArray.push_back(no);
    fInputChannelName[no] = name;
}

bool LKMTEMerger::ReadMTE(TString inputFileName)
{
    FILE *fileMTE;
    unsigned int fileSize;
    char data[16];
    int extPattern[NUM_MTE_SOURCES];
    int iTmp;
    double fTmp;
    int numEvents;

    if (fOutputFile==nullptr)
        fOutputFile = new TFile(fOutputFileName,"recreate");

    for (auto iSource : fInputChannelArray)
    {
        fMTETree[iSource] = new TTree(Form("mte_%s",fInputChannelName[iSource].Data()),"");
        fMTETree[iSource] -> Branch("num",&bTrigNum);
        fMTETree[iSource] -> Branch("time",&bTrigTime);
        fMTETree[iSource] -> Branch("type",&bTrigType);
    }

    if (inputFileName.IsNull()) {
        e_error << "Cannot find " << inputFileName << endl;
        return false;
    }
    e_info << "MTE file: " << inputFileName << endl;
    fileMTE = fopen(inputFileName, "rb");
    if (fileMTE==nullptr) {
        e_error << "Cannot open " << inputFileName << endl;
        return false;
    }

    // get file size to know # of events, 1 event = 32 byte
    fseek(fileMTE, 0L, SEEK_END);
    fileSize = ftell(fileMTE);
    fclose(fileMTE);
    numEvents = fileSize / 16;
    fileMTE = fopen(inputFileName, "rb");

    for (int evt=0; evt<numEvents; evt++)
    {
        fread(data, 1, 16, fileMTE);

        // trigger logic #
        memcpy(&bTrigNum, data, 4);

        // trigger time
        iTmp = data[4] & 0xFF;
        fTmp = iTmp;
        bTrigTime = fTmp * 0.008;        // trig_ftime = 8 ns unit
        iTmp = data[5] & 0xFF;
        fTmp = iTmp;
        bTrigTime = bTrigTime + fTmp;
        iTmp = data[6] & 0xFF;
        fTmp = iTmp;
        fTmp = fTmp * 256.0;
        bTrigTime = bTrigTime + fTmp;
        iTmp = data[7] & 0xFF;
        fTmp = iTmp;
        fTmp = fTmp * 256.0 * 256.0;
        bTrigTime = bTrigTime + fTmp;
        iTmp = data[8] & 0xFF;
        fTmp = iTmp;
        fTmp = fTmp * 256.0 * 256.0 * 256.0;
        bTrigTime = bTrigTime + fTmp;
        iTmp = data[9] & 0xFF;
        fTmp = iTmp;
        fTmp = fTmp * 256.0 * 256.0 * 256.0 * 256.0;
        bTrigTime = bTrigTime + fTmp;
        iTmp = data[NUM_MTE_SOURCES] & 0xFF;
        fTmp = iTmp;
        fTmp = fTmp * 256.0 * 256.0 * 256.0 * 256.0 * 256.0;
        bTrigTime = bTrigTime + fTmp;

        // trigger type
        bTrigType = data[11] & 0xFF;

        // external trigger pattern
        extPattern[0] = data[12] & 0x1;
        extPattern[1] = (data[12] >> 1) & 0x1;
        extPattern[2] = (data[12] >> 2) & 0x1;
        extPattern[3] = (data[12] >> 3) & 0x1;
        extPattern[4] = (data[12] >> 4) & 0x1;
        extPattern[5] = (data[12] >> 5) & 0x1;
        extPattern[6] = (data[12] >> 6) & 0x1;
        extPattern[7] = (data[12] >> 7) & 0x1;
        extPattern[8] = data[13] & 0x1;
        extPattern[9] = (data[13] >> 1) & 0x1;

        for (auto iSource : fInputChannelArray)
        {
            if (extPattern[iSource]>0)
                fMTETree[iSource] -> Fill();
        }
    }

    fclose(fileMTE);

    fOutputFile -> cd();
    for (auto iSource : fInputChannelArray) {
        fMTEEntries[iSource] = fMTETree[iSource] -> GetEntries();
        if (fMaxEntries<fMTEEntries[iSource])
            fMaxEntries=fMTEEntries[iSource];
        fMTETree[iSource] -> Write();
    }
    e_info << "Output file: " << fOutputFileName << endl;

    return true;
}

void LKMTEMerger::WriteSummaryTree()
{
    fOutputFile -> cd();
    fTreeSummary = new TTree("summary","");
    fTreeSummary -> Branch("num" ,&bKeyNum);
    fTreeSummary -> Branch("type",&bKeyType);
    fTreeSummary -> Branch("time",&bKeyTime);
    for (auto iSource : fInputChannelArray) {
        if (fKeyChannelNumber==iSource)
            continue;
        fTreeSummary -> Branch(Form("mte_entry%d",iSource), &bMTEEntry[iSource]);
        fTreeSummary -> Branch(Form("daq_entry%d",iSource), &bDAQEntry[iSource]);
    }

    //fMTETimeOffset[] = {0,-990.576 ,-990.464 ,1.29106e+06};
    double fTimeWindowCut = 5;

    for (auto key_entry=0; key_entry<fMTEEntries[fKeyChannelNumber]; ++key_entry)
    //for (auto key_entry : {36})
    {
        fMTETree[fKeyChannelNumber] -> GetEntry(key_entry);
        bKeyNum  = bTrigNum;
        bKeyTime = bTrigTime;
        bKeyType = bTrigType;
        for (auto iSource : fInputChannelArray)
        {
            if (fKeyChannelNumber==iSource)
                continue;

            bMTEEntry[iSource] = -1;
            bDAQEntry[iSource] = -1;
            double time_diff = DBL_MAX;
            auto n = fMTEEntries[iSource];
            for (auto entry=0; entry<n; ++entry)
            {
                fMTETree[iSource] -> GetEntry(entry);
                time_diff = bKeyTime - bTrigTime - fMTETimeOffset[iSource];
                if (abs(time_diff)<fTimeWindowCut)
                {
                    bMTEEntry[iSource] = entry;
                    bDAQEntry[iSource] = entry + fEntryOffset[iSource];
                }
                else if (time_diff<0)
                {
                    break;
                }
            }
        }

        fTreeSummary -> Fill();
    }

    fTreeSummary -> Write();

    return true;
}

void LKMTEMerger::GetTimeOffset(TString fileName)
{
    int iSource;
    double offset;
    e_info << "Reading " << fileName << endl;
    ifstream offsetFile(fileName);
    while (offsetFile >> iSource >> offset) {
        fMTETimeOffset[iSource] = offset;
        e_info << fInputChannelName[iSource] << " time-offset = " << fMTETimeOffset[iSource] << endl;
    }
}

void LKMTEMerger::FindTimeOffset(TString fileName)
{
    ///////////////////////////////////////////////////////////////////////
    // Find tree with maximum bTrigNum at entry 0
    int    num [NUM_MTE_SOURCES] = {0};
    double time[NUM_MTE_SOURCES] = {0};
    int    type[NUM_MTE_SOURCES] = {0};
    for (auto iSource : fInputChannelArray) {
        num [iSource] = 0;
        time[iSource] = 0;
        type[iSource] = 0;
    }
    for (auto iSource : fInputChannelArray)
    {
        fMTETree[iSource] -> GetEntry(0);
        num [iSource] = bTrigNum;
        time[iSource] = bTrigTime;
        type[iSource] = bTrigType;
    }
    int numAtMax = 0;
    double timeAtMax = 0;
    int iSourceAtMax = 0;
    for (auto iSource : fInputChannelArray)
    {
        if (numAtMax<num[iSource])  {
            numAtMax = num[iSource];
            timeAtMax = time[iSource];
            iSourceAtMax = iSource;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // For each tree, find closest time with timeAtMax
    double timeAtMin[NUM_MTE_SOURCES] = {0};
    timeAtMin[iSourceAtMax] = timeAtMax;
    for (auto iSource : fInputChannelArray)
    {
        if (iSourceAtMax==iSource)
            continue;
        double time_diff_min = DBL_MAX;
        for (auto entry=0; entry<fMaxEntries; ++entry)
        {
            fMTETree[iSource] -> GetEntry(entry);
            time[iSource] = bTrigTime;
            double time_diff = abs(time[iSource]-timeAtMax);
            if (time_diff_min>time_diff)
            {
                time_diff_min = time_diff;
                //firstEntry[iSource] = entry;
                timeAtMin[iSource] = bTrigTime;
            }
            else if (time_diff_min<time_diff)
                break;
        }
    }

    ofstream offsetFile(fileName);
    for (auto iSource : fInputChannelArray) {
        fMTETimeOffset[iSource] = timeAtMin[fKeyChannelNumber] - timeAtMin[iSource];
        e_info     << fInputChannelName[iSource] << " time-offset = " << fMTETimeOffset[iSource] << endl;
        offsetFile << iSource << " " << fMTETimeOffset[iSource]  << endl;
    }
}

bool LKMTEMerger::MapKobra(TString fileName, TString kobraName)
{
    int iKobraTree = -1;
    for (auto iSource : fInputChannelArray) {
        if (fInputChannelName[iSource]==kobraName) {
            iKobraTree = iSource;
            break;
        }
    }
    if (iKobraTree<0) {
        e_error << "MTE tree for Kobra do not exist!" << endl;
        return false;
    }

    auto file = new TFile(fileName,"read");
    if (file->IsOpen()==false) {
        e_error << "Kobra file do not exist! " << fileName << endl;
        return false;
    }
    else
        e_info << "Kobra file: " << fileName << endl;
    auto tree = (TTree*) file -> Get("midas_data");
    int eventidT1, ref_pulse, scaler, scaler_prev;
    tree -> SetBranchAddress("scaler",    &scaler);

    scaler_prev = INT_MAX;
    tree -> GetEntry(5);
    scaler_prev = scaler;
    tree -> GetEntry(6);
    int dscaler = scaler - scaler_prev;

    //cout.precision(9);

    double time_prev = DBL_MAX;
    double time_diff = 0;
    int entry;
    for (entry=0; entry<fMTEEntries[iKobraTree]; ++entry)
    {
        time_prev = bTrigTime;
        fMTETree[iKobraTree] -> GetEntry(entry);

        if (entry>0) {
            time_diff = (bTrigTime - time_prev)*100;

            double time_window = dscaler*fTimeWindowFactor; // XXX
            if (abs(dscaler-time_diff)<time_window) {
                e_test << "Matching event! entry=" << entry << ", dsacaler-time_diff=" << dscaler - time_diff << ", time-window=" << time_window << endl;
                break;
            }
        }
    }

    fEntryOffset[iKobraTree] = 6 - entry;;
    e_info << fInputChannelName[iKobraTree] << " " << fEntryOffset[iKobraTree] << endl;

    return true;
}

bool LKMTEMerger::ReadKobra(TString fileName, TString kobraName)
{
    int iKobraTree = -1;
    for (auto iSource : fInputChannelArray) {
        if (fInputChannelName[iSource]==kobraName) {
            iKobraTree = iSource;
            break;
        }
    }
    if (iKobraTree<0) {
        e_error << "MTE tree for Kobra do not exist!" << endl;
        return false;
    }

    //fTreeSummary -> SetBranchAddress("num" ,&bKeyNum);
    //fTreeSummary -> SetBranchAddress("type",&bKeyType);
    //fTreeSummary -> SetBranchAddress("time",&bKeyTime);
    //for (auto iSource : {iKobraTree}) {
    //    fTreeSummary -> SetBranchAddress(Form("mte_entry%d",iSource), &bMTEEntry[iSource]);
    //    fTreeSummary -> SetBranchAddress(Form("daq_entry%d",iSource), &bDAQEntry[iSource]);
    //}

    auto fileKobra = new TFile(fileName,"read");
    if (fileKobra->IsOpen()==false) {
        e_error << "Kobra file do not exist! " << fileName << endl;
        return false;
    }
    else
        e_info << "Kobra file: " << fileName << endl;
    auto treeKobra = (TTree*) fileKobra -> Get("midas_data");
    int eventidT1, ref_pulse, scaler;
    treeKobra -> SetBranchAddress("eventidT1", &eventidT1);
    treeKobra -> SetBranchAddress("ref_pulse", &ref_pulse);
    treeKobra -> SetBranchAddress("scaler",    &scaler);

    int numSummary = fTreeSummary -> GetEntries();
    for (int i=0; i<numSummary; ++i)
    {
        fTreeSummary -> GetEntry(i);
        treeKobra -> GetEntry(bDAQEntry[iKobraTree]);
    }
    return true;
}


int LKMTEMerger::GetKobraEntry(int i)
{
    return 0;
}
