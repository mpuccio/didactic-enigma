#if !defined(CLING) || defined(ROOTCLING)
#include <iostream>
#include "SimulationDataFormat/MCTrack.h"

#include <gsl/gsl>
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TSystemDirectory.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"
#include "TLegend.h"
#include <TKey.h>
#endif

using MCTrack = o2::MCTrack;

struct particle {
	int pdg;
	int d0;
	int d1;
};

void checkAODkine()
{
	TFile inFile("AO2D.root");
	TIter keyList(inFile.GetListOfKeys());
	TKey *key;

	int           fIndexMcCollisions;
	int           fPdgCode;
	int           fStatusCode;
	unsigned char fFlags;
	int           fIndexArray_Mothers_size;
	int           fIndexArray_Mothers[2];   //[fIndexArray_Mothers_size]
	int           fIndexSlice_Daughters[2];
	float         fWeight;
	float         fPx;
	float         fPy;
	float         fPz;
	float         fE;
	float         fVx;
	float         fVy;
	float         fVz;
	float         fVt;

	while ((key = (TKey*)keyList())) {
		TTree* tree = (TTree*)inFile.Get(Form("%s/O2mcparticle_001",key->GetName()));
		if (!tree) continue;
		tree->SetBranchAddress("fIndexMcCollisions", &fIndexMcCollisions);
		tree->SetBranchAddress("fPdgCode", &fPdgCode);
		tree->SetBranchAddress("fStatusCode", &fStatusCode);
		tree->SetBranchAddress("fFlags", &fFlags);
		tree->SetBranchAddress("fIndexArray_Mothers_size", &fIndexArray_Mothers_size);
		tree->SetBranchAddress("fIndexArray_Mothers", fIndexArray_Mothers);
		tree->SetBranchAddress("fIndexSlice_Daughters", fIndexSlice_Daughters);
		tree->SetBranchAddress("fWeight", &fWeight);
		tree->SetBranchAddress("fPx", &fPx);
		tree->SetBranchAddress("fPy", &fPy);
		tree->SetBranchAddress("fPz", &fPz);
		tree->SetBranchAddress("fE", &fE);
		tree->SetBranchAddress("fVx", &fVx);
		tree->SetBranchAddress("fVy", &fVy);
		tree->SetBranchAddress("fVz", &fVz);
		tree->SetBranchAddress("fVt", &fVt);

		std::vector<particle> parts;
		for (int n{0}; n < tree->GetEntries(); ++n) {
			tree->GetEvent(n);
			particle p;
			p.pdg = fPdgCode;
			p.d0 = fIndexSlice_Daughters[0];
			p.d1 = fIndexSlice_Daughters[1];
			parts.emplace_back(p);
		}
		for (auto part : parts) {
			if (part.pdg == 1010010030) {
				std::vector<int> daug;
				bool save{false};
				for (int i{part.d0}; i <= part.d1; ++i) {
					save = save || parts[i].pdg == 1000010020;
					daug.push_back(parts[i].pdg);
				}
				for (int i{0}; i < save * daug.size(); ++i) {
					std::cout << daug[i] << "\t";
				}
				if (save * daug.size()) std::cout << std::endl;
			}
		}
	}
}
