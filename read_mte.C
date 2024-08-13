#include "LKMTEMerger.cpp"

void read_mte()
{
    auto merger = new LKMTEMerger();
    merger -> SetKeyChannel(0,"Cobo");
    merger -> SetInputChannel(1,"Kobra");
    merger -> SetInputChannel(2,"Pulser");
    merger -> SetInputChannel(3,"Scint");
    merger -> ReadMTE("data/lte_tag_128.dat","data/mte_summary.root");
    //merger -> FindTimeOffset();
    merger -> GetTimeOffset();
    merger -> MapKobra("data/output00581.root");
    merger -> WriteSummaryTree();
    merger -> ReadKobra("data/output00581.root");

    auto koboEntry = merger -> GetKobraEntry(1);
    cout << koboEntry << endl;

    //merger -> ReadKobra("data/output00581.root"); // write file name to summary file
    //merger -> ReadCobo("data/cobo.root");
}
