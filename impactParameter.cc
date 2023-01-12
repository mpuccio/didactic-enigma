// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TFile.h"
#include "TNtuple.h"
#include "TH3F.h"

#include "DataFormatsParameters/GRPObject.h"
#include "DetectorsBase/GeometryManager.h"
#include "DetectorsBase/Propagator.h"
#include "MathUtils/Cartesian.h"
#include "GPUCommonArray.h"
#include "ReconstructionDataFormats/PrimaryVertex.h"
#include "ReconstructionDataFormats/VtxTrackIndex.h"
#include "ReconstructionDataFormats/VtxTrackRef.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DataFormatsITS/TrackITS.h"
#include "ReconstructionDataFormats/TrackTPCITS.h"
#include "Steer/MCKinematicsReader.h"
#include "CommonDataFormat/InteractionRecord.h"
#include "SimulationDataFormat/DigitizationContext.h"

#include <vector>

using namespace o2;
using namespace o2::framework;

using GIndex = o2::dataformats::VtxTrackIndex;

template <typename T>
void BinLogAxisY(T h) {
  //
  // Method for the correct logarithmic binning of histograms
  //
  TAxis* axis = h->GetYaxis();
  int bins = axis->GetNbins();
  double from = axis->GetXmin();
  double to = axis->GetXmax();
  double *newBins = new double[bins + 1];

  newBins[0] = from;
  double factor = std::pow(to / from, 1. / bins);

  for (int i = 1; i <= bins; i++) {
    newBins[i] = factor * newBins[i - 1];
  }
  axis->Set(bins, newBins);
  delete[] newBins;
}

template <typename TrackT>
std::vector<TrackT> *fetchTracks(const char *filename, const char *treename, const char *branchname)
{
	TFile file(filename, "OPEN");
	auto tree = (TTree *)file.Get(treename);
	auto br = tree->GetBranch(branchname);
	std::vector<TrackT> *tracks = nullptr;
	br->SetAddress(&tracks);
	br->GetEntry(0);
	return tracks;
}

// add vertices/collisions
void impactParameterResolution()
{
	// open the file for vertices
	TFile f("o2_primary_vertex.root", "OPEN");
	auto t = (TTree *)f.Get("o2sim");

	const auto grp = o2::parameters::GRPObject::loadFrom("/data1/raw/o2sim_grp_b5m_128HB.root");
	o2::base::GeometryManager::loadGeometry("/data1/raw/o2sim_geometry-aligned.root");

	// fetch the tracks (these names are not following any convention!!)
	auto itstracks = fetchTracks<o2::its::TrackITS>("o2trac_its.root", "o2sim", "ITSTrack");
	auto itstpctracks = fetchTracks<o2::dataformats::TrackTPCITS>("o2match_itstpc.root", "matchTPCITS", "TPCITS");

	if (t)
	{
		auto br = t->GetBranch("PrimaryVertex");
		std::vector<o2::dataformats::PrimaryVertex> *vertices = nullptr;
		br->SetAddress(&vertices);
		br->GetEntry(0);

		// this referes to actual tracks
		auto indexbr = t->GetBranch("PVTrackIndices");
		std::vector<GIndex> *vertexTrackIDs = nullptr;
		indexbr->SetAddress(&vertexTrackIDs);
		indexbr->GetEntry(0);

		// this makes the connection of vertex to track indices
		auto v2totrackrefbr = t->GetBranch("PV2TrackRefs");
		std::vector<o2::dataformats::VtxTrackRef> *v2trackref = nullptr;
		v2totrackrefbr->SetAddress(&v2trackref);
		v2totrackrefbr->GetEntry(0);

		if (vertices && vertexTrackIDs)
		{
			o2::base::Propagator::initFieldFromGRP(grp);
  		auto propagator = o2::base::Propagator::Instance();
			TFile outfile("output.root", "RECREATE");
			TH3 *hImpactParameterXY[2], *hImpactParameterZ[2];
			std::string matter[2]{"M", "A"};
			for (int iC{0}; iC < 2; ++iC) {
				hImpactParameterXY[iC] = new TH3F(fmt::format("hImpactParameterXY_{}", matter[iC]).data(), ";Vertex contributors;#it{p}_{T} (GeV/#it{c});Impact parameter XY (cm)", 100, 0.5f, 100.5f, 200, 0.05f, 10.f, 1000, -1.f, 1.f);
				BinLogAxisY(hImpactParameterXY[iC]);
				hImpactParameterZ[iC] = new TH3F(fmt::format("hImpactParameterZ_{}", matter[iC]).data(), ";Vertex contributors;#it{p}_{T} (GeV/#it{c});Impact parameter Z (cm)", 100, 0.5f, 100.5f, 200, 0.05f, 10.f, 1000, -1.f, 1.f);
				BinLogAxisY(hImpactParameterZ[iC]);
			}
			// TNtuple nt("imptrk","Track ntuple for impact parameter studies","xv:yv:zv:nc:y:z:ncl");
			for (unsigned int index = 0; index < vertices->size(); ++index)
			{
				// get the track for each vertex and fill the tracks table
				// now go over tracks via the indices
				auto &v = vertices->at(index);
				auto &trackref = v2trackref->at(index);
				int start = trackref.getFirstEntryOfSource(0);
				int ntracks = trackref.getEntriesOfSource(0);
				for (int ti = 0; ti < ntracks; ++ti)
				{
					auto trackindex = (*vertexTrackIDs)[start + ti];

					// now we need to fetch the actual track and fill the table
					const auto source = trackindex.getSource();
					o2::track::TrackParCov *track = nullptr;
					int nCls = 0;
					if (source == o2::dataformats::VtxTrackIndex::Source::ITS)
					{
						auto& sTrack = (*itstracks)[trackindex.getIndex()];
						nCls = sTrack.getNumberOfClusters();
						track = &sTrack;
					}
					else if (source == o2::dataformats::VtxTrackIndex::Source::ITSTPC)
					{
						auto& sTrack = (*itstpctracks)[trackindex.getIndex()];
						auto& itsTrack = (*itstracks)[sTrack.getRefITS().getIndex()];
						nCls = itsTrack.getNClusters();
						track = &sTrack;
					}
					else
					{
						continue;
					}
					gpu::gpustd::array<float, 2> dca{-999.f, -999.f};
					if (!propagator->propagateToDCA(v.getXYZ(), *track, -5.f, 2.f, o2::base::PropagatorF::MatCorrType::USEMatCorrLUT, &dca)) {
						continue;
					}
					hImpactParameterXY[track->getSign() < 0.f]->Fill(v.getNContributors(), track->getPt(), dca[0]);
					hImpactParameterZ[track->getSign() < 0.f]->Fill(v.getNContributors(), track->getPt(), dca[1]);
					// nt.Fill(v.getX(), v.getY(), v.getZ(), v.getNContributors(), dca[0], dca[1], nCls);
				}
			}
			// nt.Write();
			for (int iC{0}; iC < 2; ++iC) {
				hImpactParameterXY[iC]->Write();
				hImpactParameterZ[iC]->Write();
			}
			f.Close();
		}
	}
}
