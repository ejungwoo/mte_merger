//#include "LKLogger.h"
#include "LKMTEMerger.cpp"

void read_mte()
{
    auto merger = new LKMTEMerger("data/mte_summary.root");

    merger -> SetKeyChannel(0,"Cobo");
    merger -> SetInputChannel(1,"Kobra");
    merger -> SetInputChannel(2,"Pulser");
    merger -> SetInputChannel(3,"Scint");

    merger -> ReadMTE("data/lte_tag_128.dat");
    //merger -> FindTimeOffset("data/mte_time_offset.txt");
    merger -> GetTimeOffset("data/mte_time_offset.txt");
    merger -> MapKobra("data/output00581.root");
    merger -> WriteSummaryTree();
    merger -> ReadKobra("data/output00581.root");

    //auto koboEntry = merger -> GetKobraEntry(1);
}
