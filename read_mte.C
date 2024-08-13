//#include "LKLogger.h"
#include "LKMTEMerger.cpp"

void read_mte()
{
    auto merger = new LKMTEMerger("data/mte_summary.root");

    merger -> SetKeyChannel(0,"Cobo");
    merger -> SetInputChannel(1,"Kobra");
    merger -> SetInputChannel(2,"Pulser");
    merger -> SetInputChannel(3,"Scint");
    merger -> SetMTEKobraTimeWindowFactor(1./50000);
    merger -> SetTestKobraDAQEntry(5);

    merger -> ReadMTE("data/lte_tag_128.dat");
    //merger -> FindTimeOffset("data/mte_time_offset.txt");
    merger -> GetTimeOffset("data/mte_time_offset.txt");
    merger -> MapKobra("data/output00581.root");
    merger -> WriteSummary();

    //merger -> TestKobraEntry(50); // just for testing

    /// for cobo entry of interest, get data from kobra daq tree
    auto tree = merger -> GetKobraTree();
    int eventidT1, ref_pulse, scaler;
    tree -> SetBranchAddress("eventidT1", &eventidT1);
    tree -> SetBranchAddress("ref_pulse", &ref_pulse);
    tree -> SetBranchAddress("scaler",    &scaler);
    for (auto entryCobo : {40,45,50})
    {
        auto entryKobra = merger -> GetKobraEntry(entryCobo);
        e_test << "cobo=" << entryCobo << " kobra=" << entryKobra <<  " eventidT1=" << eventidT1 << " ref_pulse=" << ref_pulse << " scaler=" << scaler << endl;
    }

}
