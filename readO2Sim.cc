#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Headers needed by this particular selector
#include "SimulationDataFormat/MCTrack.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>

void readO2Sim(std::string filename = "sgn_1_Kine.root")
{
  TTreeReader fReader;
  TFile file(filename.data());
  TTree *tree = (TTree*)file.Get("o2sim");
  TTreeReaderArray<o2::MCTrackT<float>> MCTracks = {fReader, "MCTrack"};
  fReader.SetTree(tree);

  std::cout << "Starting the loop" << std::endl;
  int stop{0};
  while (fReader.Next()) {
    if (++stop == 2) {
      break;
    }
    std::cout << "MCTracks size " << MCTracks.GetSize() << std::endl;
    for (auto &part : MCTracks) {
      if (std::abs(part.GetPdgCode()) != 1010010030) {
        continue;
      }
      std::stringstream stream;
      stream << part.GetPdgCode() << ":\t";
      for (int j{part.getFirstDaughterTrackId()}; j <= part.getLastDaughterTrackId(); ++j) {
        stream << MCTracks.At(j).GetPdgCode() << "\t";
      }
      std::cout << stream.str() << std::endl;
    }
  }

}
