#include "LKLogger.h"
#include "LKMTEMerger.h"
#include <stdio.h>
#include <iostream>
using namespace std;

//ClassImp(LKMTEMerger)

LKMTEMerger::LKMTEMerger()
{
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

bool LKMTEMerger::ReadMTE(TString inputFileName, TString outputFileName)
{
    FILE *data_fp;
    FILE *text_fp;
    unsigned int file_size;
    int nevt;
    int evt;
    char data[16];
    int ext_pattern[NUM_MTE_INPUT_CHANNELS];
    int ext_pattern_channel;
    int itmp;
    double ftmp;
    int i;

    if (fOutputFile==nullptr)
        fOutputFile = new TFile(outputFileName,"recreate");

    for (auto i : fInputChannelArray)
    {
        fMTETree[i] = new TTree(Form("mte_%s",fInputChannelName[i].Data()),"");
        fMTETree[i] -> Branch("trig_num",&trig_num);
        fMTETree[i] -> Branch("trig_time",&trig_time);
        fMTETree[i] -> Branch("trig_type",&trig_type);
    }

    //fCoboTree = new TTree("mte","only cobo triggered event");
    //fCoboTree -> Branch("tn_cobo",&tn_cobo);
    //fCoboTree -> Branch("tt_cobo",&tt_cobo);
    //fCoboTree -> Branch("tn_kobra",&tn_kobra);
    //fCoboTree -> Branch("tt_kobra",&tt_kobra);

    if (inputFileName.IsNull()) {
        cout << "Cannot find " << inputFileName << endl;
        return false;
    }
    cout << "== MTE file: " << inputFileName << endl;
    data_fp = fopen(inputFileName, "rb");
    if (data_fp==nullptr) {
        cout << "Cannot open " << inputFileName << endl;
        return false;
    }

    // get file size to know # of events, 1 event = 32 byte
    fseek(data_fp, 0L, SEEK_END);
    file_size = ftell(data_fp);
    fclose(data_fp);
    nevt = file_size / 16;
    data_fp = fopen(inputFileName, "rb");

    for (evt = 0; evt < nevt; evt++)
    {
        fread(data, 1, 16, data_fp);

        // trigger logic #
        memcpy(&trig_num, data, 4);

        // trigger time
        itmp = data[4] & 0xFF;
        ftmp = itmp;
        trig_time = ftmp * 0.008;        // trig_ftime = 8 ns unit
        itmp = data[5] & 0xFF;
        ftmp = itmp;
        trig_time = trig_time + ftmp;
        itmp = data[6] & 0xFF;
        ftmp = itmp;
        ftmp = ftmp * 256.0;
        trig_time = trig_time + ftmp;
        itmp = data[7] & 0xFF;
        ftmp = itmp;
        ftmp = ftmp * 256.0 * 256.0;
        trig_time = trig_time + ftmp;
        itmp = data[8] & 0xFF;
        ftmp = itmp;
        ftmp = ftmp * 256.0 * 256.0 * 256.0;
        trig_time = trig_time + ftmp;
        itmp = data[9] & 0xFF;
        ftmp = itmp;
        ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0;
        trig_time = trig_time + ftmp;
        itmp = data[NUM_MTE_INPUT_CHANNELS] & 0xFF;
        ftmp = itmp;
        ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0 * 256.0;
        trig_time = trig_time + ftmp;

        // trigger type
        trig_type = data[11] & 0xFF;

        // external trigger pattern
        ext_pattern[0] = data[12] & 0x1;
        ext_pattern[1] = (data[12] >> 1) & 0x1;
        ext_pattern[2] = (data[12] >> 2) & 0x1;
        ext_pattern[3] = (data[12] >> 3) & 0x1;
        ext_pattern[4] = (data[12] >> 4) & 0x1;
        ext_pattern[5] = (data[12] >> 5) & 0x1;
        ext_pattern[6] = (data[12] >> 6) & 0x1;
        ext_pattern[7] = (data[12] >> 7) & 0x1;
        ext_pattern[8] = data[13] & 0x1;
        ext_pattern[9] = (data[13] >> 1) & 0x1;

        for (auto i : fInputChannelArray)
        {
            ext_pattern_channel = ext_pattern[i];
            if (ext_pattern_channel>0)
                fMTETree[i] -> Fill();
        }
    }

    fclose(data_fp);

    fOutputFile -> cd();
    for (auto iSource : fInputChannelArray) {
        fMTEEntries[iSource] = fMTETree[iSource] -> GetEntries();
        if (fMaxEntries<fMTEEntries[iSource])
            fMaxEntries=fMTEEntries[iSource];
        fMTETree[iSource] -> Write();
    }
    cout << "== Output file: " << outputFileName << endl;

    ///////////////////////////////////////////////////////////////////////

    /*
    int    num [NUM_MTE_INPUT_CHANNELS] = {0};
    double time[NUM_MTE_INPUT_CHANNELS] = {0};
    int    type[NUM_MTE_INPUT_CHANNELS] = {0};
    auto treeAll = new TTree("all","");
    for (auto iSource : fInputChannelArray) {
        treeAll -> Branch(Form("num%d" ,iSource),  &num [iSource]);
        treeAll -> Branch(Form("time%d",iSource), &time[iSource]);
        treeAll -> Branch(Form("type%d",iSource), &type[iSource]);
    }

    for (auto entry=0; entry<fMaxEntries; ++entry)
    {
        for (auto iSource : fInputChannelArray) {
            num [iSource] = 0;
            time[iSource] = 0;
            type[iSource] = 0;
        }
        for (auto iSource : fInputChannelArray)
        {
            if (entry>=fMTEEntries[iSource]) {
                continue;
            }
            fMTETree[iSource] -> GetEntry(entry);
            num [iSource] = trig_num;
            time[iSource] = trig_time;
            type[iSource] = trig_type;
        }
        treeAll -> Fill();
    }
    treeAll -> Write();
    treeAll -> Scan("*");
    */

    return true;
}

void LKMTEMerger::WriteSummaryTree()
{
    int    key_num   = 0;
    double key_time  = 0;
    int    key_type  = 0;
    int    mte_entry[NUM_MTE_INPUT_CHANNELS] = {0};
    int    daq_entry[NUM_MTE_INPUT_CHANNELS] = {0};
    fOutputFile -> cd();
    fTreeSummary = new TTree("summary","");
    fTreeSummary -> Branch("num" ,&key_num);
    fTreeSummary -> Branch("type",&key_type);
    fTreeSummary -> Branch("time",&key_time);
    for (auto iSource : fInputChannelArray) {
        if (fKeyChannelNumber==iSource)
            continue;
        fTreeSummary -> Branch(Form("mte_id%d",iSource), &mte_entry[iSource]);
        fTreeSummary -> Branch(Form("daq_id%d",iSource), &daq_entry[iSource]);
    }

    //fMTETimeOffset[] = {0,-990.576 ,-990.464 ,1.29106e+06};
    double fTimeWindowCut = 5;

    for (auto key_entry=0; key_entry<fMTEEntries[fKeyChannelNumber]; ++key_entry)
    //for (auto key_entry : {36})
    {
        fMTETree[fKeyChannelNumber] -> GetEntry(key_entry);
        key_num  = trig_num;
        key_time = trig_time;
        key_type = trig_type;
        for (auto iSource : fInputChannelArray)
        {
            if (fKeyChannelNumber==iSource)
                continue;

            mte_entry[iSource] = -1;
            daq_entry[iSource] = -1;
            double time_diff = DBL_MAX;
            auto n = fMTEEntries[iSource];
            for (auto entry=0; entry<n; ++entry)
            {
                fMTETree[iSource] -> GetEntry(entry);
                time_diff = key_time - trig_time - fMTETimeOffset[iSource];
                //cout <<  key_time << " " <<  trig_time << " " << fMTETimeOffset[iSource] << " " << key_entry << " " << entry << " " << time_diff << endl;
                if (abs(time_diff)<fTimeWindowCut)
                {
                    //cout << "== " << fInputChannelName[iSource] << " knum=" << key_num << " inum=" << trig_num << " diff=" << time_diff << endl;
                    //entryAtMatch[iSource] = entry;
                    mte_entry[iSource] = entry;
                    daq_entry[iSource] = entry + fEntryOffset[iSource];
                    //time_diff_min ??
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
    //fTreeSummary -> Scan("*");

    return true;
}

void LKMTEMerger::GetTimeOffset()
{
    int iSource, offset;
    ifstream offsetFile("data/mte_time_offset.txt");
    while (offsetFile >> iSource >> offset) {
        fMTETimeOffset[iSource] = offset;
        cout << iSource << " " << fMTETimeOffset[iSource] << endl;
    }
}

void LKMTEMerger::FindTimeOffset()
{
    ///////////////////////////////////////////////////////////////////////
    // Find tree with maximum trig_num at entry 0

    int    num [NUM_MTE_INPUT_CHANNELS] = {0};
    double time[NUM_MTE_INPUT_CHANNELS] = {0};
    int    type[NUM_MTE_INPUT_CHANNELS] = {0};

    for (auto iSource : fInputChannelArray) {
        num [iSource] = 0;
        time[iSource] = 0;
        type[iSource] = 0;
    }
    for (auto iSource : fInputChannelArray)
    {
        fMTETree[iSource] -> GetEntry(0);
        num [iSource] = trig_num;
        time[iSource] = trig_time;
        type[iSource] = trig_type;
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
    //int firstEntry[NUM_MTE_INPUT_CHANNELS] = {0};
    //double timeOffset[NUM_MTE_INPUT_CHANNELS] = {0};
    double timeAtMin[NUM_MTE_INPUT_CHANNELS] = {0};
    timeAtMin[iSourceAtMax] = timeAtMax;
    for (auto iSource : fInputChannelArray)
    {
        if (iSourceAtMax==iSource)
            continue;

        double time_diff_min = DBL_MAX;
        for (auto entry=0; entry<fMaxEntries; ++entry)
        {
            fMTETree[iSource] -> GetEntry(entry);
            time[iSource] = trig_time;
            double time_diff = abs(time[iSource]-timeAtMax);
            if (time_diff_min>time_diff)
            {
                time_diff_min = time_diff;
                //firstEntry[iSource] = entry;
                timeAtMin[iSource] = trig_time;
            }
            else if (time_diff_min<time_diff)
                break;
        }
        //cout << iSource << " " << timeAtMin[iSource] << " " << time_diff_min << endl;
        //timeOffset[iSource] = time_diff_min;
        //cout << iSource << " " << firstEntry[iSource] << " " << time_diff_min << " " << timeAtMax << endl;
    }

    ofstream offsetFile("data/mte_time_offset.txt");
    for (auto iSource : fInputChannelArray) {
        //if (iSource==fKeyChannelNumber) continue;
        //cout       << iSource << " " << timeAtMin[fKeyChannelNumber] << " " <<  timeAtMin[iSource] << endl;
        cout       << iSource << " " << timeAtMin[fKeyChannelNumber] - timeAtMin[iSource] << endl;
        offsetFile << iSource << " " << timeAtMin[fKeyChannelNumber] - timeAtMin[iSource] << endl;
        fMTETimeOffset[iSource] = timeAtMin[fKeyChannelNumber] - timeAtMin[iSource];
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
        cout << "MTE tree for Kobra do not exist!" << endl;
        return false;
    }

    auto file = new TFile(fileName,"read");
    if (file->IsOpen()==false) {
        cout << "Kobra file do not exist! " << fileName << endl;
        return false;
    }
    else
        cout << "== Kobra file: " << fileName << endl;
    auto tree = (TTree*) file -> Get("midas_data");
    int eventidT1, ref_pulse, scaler, scaler_prev;
    //tree -> SetBranchAddress("eventidT1", &eventidT1);
    //tree -> SetBranchAddress("ref_pulse", &ref_pulse);
    tree -> SetBranchAddress("scaler",    &scaler);

    scaler_prev = INT_MAX;
    fNumEntriesKobra = tree -> GetEntries();
    //for (auto entry=0; entry<fNumEntriesKobra; ++entry)
    //int dscaler[10];
    //for (auto entry : {4,5,6})
    //{
    //    tree -> GetEntry(entry);
    //    //cout << setw(5) << entry << setw(5) << eventidT1 << setw(5) << ref_pulse << setw(15) << scaler << endl;
    //    dscaler[entry] = scaler - scaler_prev;
    //    scaler_prev = scaler;
    //}

    tree -> GetEntry(5);
    scaler_prev = scaler;
    tree -> GetEntry(6);
    int dscaler = scaler - scaler_prev;

    cout << scaler << " " << scaler_prev << " " << dscaler << endl;

    double time_prev = DBL_MAX;
    double time_diff = 0;
    int entry;
    for (entry=0; entry<fMTEEntries[iKobraTree]; ++entry)
    {
        time_prev = trig_time;
        fMTETree[iKobraTree] -> GetEntry(entry);

        if (entry>0) {
            time_diff = (trig_time - time_prev)*100;
            cout.precision(9);
            cout << entry << " ds=" << dscaler << " tf=" << time_diff << " " << abs(dscaler-time_diff) << endl;

            double time_window = dscaler*(1./50000);
            cout << "time-window=" << time_window << endl;
            if (abs(dscaler-time_diff)<time_window) {
                cout << "matching event " << entry << " " << dscaler - time_diff << endl;
                break;
            }
        }
    }

    fEntryOffset[iKobraTree] = 6 - entry;;
    cout << fInputChannelName[iKobraTree] << " " << fEntryOffset[iKobraTree] << endl;

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
        cout << "MTE tree for Kobra do not exist!" << endl;
        return false;
    }

    int    key_num   = 0;
    double key_time  = 0;
    int    key_type  = 0;
    int    mte_entry[NUM_MTE_INPUT_CHANNELS] = {0};
    int    daq_entry[NUM_MTE_INPUT_CHANNELS] = {0};

    //fTreeSummary = (TTree*) fOutputFile -> Get("summary");
    fTreeSummary -> SetBranchAddress("num" ,&key_num);
    fTreeSummary -> SetBranchAddress("type",&key_type);
    fTreeSummary -> SetBranchAddress("time",&key_time);
    for (auto iSource : {iKobraTree}) {
        fTreeSummary -> SetBranchAddress(Form("mte_id%d",iSource), &mte_entry[iSource]);
        fTreeSummary -> SetBranchAddress(Form("daq_id%d",iSource), &daq_entry[iSource]);
    }

    auto file = new TFile(fileName,"read");
    if (file->IsOpen()==false) {
        cout << "Kobra file do not exist! " << fileName << endl;
        return false;
    }
    else
        cout << "== Kobra file: " << fileName << endl;
    auto treeKobra = (TTree*) file -> Get("midas_data");
    int eventidT1, ref_pulse, scaler;
    treeKobra -> SetBranchAddress("eventidT1", &eventidT1);
    treeKobra -> SetBranchAddress("ref_pulse", &ref_pulse);
    treeKobra -> SetBranchAddress("scaler",    &scaler);

    int numSummary = fTreeSummary -> GetEntries();
    cout << numSummary << endl;
    for (int i=0; i<numSummary; ++i)
    {
        fTreeSummary -> GetEntry(i);
        treeKobra -> GetEntry(daq_entry[iKobraTree]);
        cout << i << " " << daq_entry[iKobraTree] << " " << eventidT1 << " " << ref_pulse << " " << scaler << endl;
    }
    return true;
}


int LKMTEMerger::GetKobraEntry(int i)
{
    return 0;
}
