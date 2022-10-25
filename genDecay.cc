#include <TF1.h>
#include <TFile.h>
#include <TGenPhaseSpace.h>
#include <TH1D.h>
#include <TLorentzVector.h> // ugly, but TGenPhaseSpace uses this.
#include <TMath.h>
#include <TRandom.h>

constexpr int kNtrials{1000000};
constexpr double kHypMass{2.99125};
constexpr double kTritonMass{2.80892};
constexpr double kPi0Mass{0.139};
constexpr double kPtResKink{0.2};
constexpr double kPtRes{0.02};

void genDecay() {
  gRandom->SetSeed(1234);

  TF1 mtExpo("mtExpo","[0]*x*std::exp(-std::hypot([2], x)/[1])", 0, 10);
  mtExpo.SetParameter(0, 1.);
  mtExpo.SetParameter(1, 0.5);
  mtExpo.SetParameter(2, kHypMass);

  TH1D hTritonPt("hTritonPt", ";#it{p}_{T} (GeV/#it{c});Entries", 100, 0, 10);
  TH1D hPi0Pt("hPi0Pt", ";#it{p}_{T} (GeV/#it{c});Entries", 100, 0, 2);
  TH1D hGammaPt("hGammaPt", ";#it{p}_{T} (GeV/#it{c});Entries", 100, 0, 2);
  TH2D hHyperMkink("hHyperMkink", ";#it{p}_{T} (GeV/#it{c});M (GeV/#it{c}^{2});Entries", 10, 0, 5, 300, 2.9, 3.2);

  TLorentzVector mother, kinkPi0, recTriton;
  TGenPhaseSpace genH, genPi0;
  const double massesH[2]{kTritonMass, kPi0Mass}, massesPi0[2]{0.,0.};
  for (int i{0}; i < kNtrials; ++i) {
    mother.SetPtEtaPhiM(mtExpo.GetRandom(), gRandom->Uniform(-1, 1), gRandom->Uniform(0, TMath::TwoPi()), kHypMass);
    genH.SetDecay(mother, 2, massesH);
    genH.Generate();
    hTritonPt.Fill(genH.GetDecay(0)->Pt());
    hPi0Pt.Fill(genH.GetDecay(1)->Pt());
    genPi0.SetDecay(*genH.GetDecay(1), 2, massesPi0);
    genPi0.Generate();
    hGammaPt.Fill(genPi0.GetDecay(0)->Pt());
    hGammaPt.Fill(genPi0.GetDecay(1)->Pt());
    double momPt{mother.Pt()};
    mother.SetPtEtaPhiM(gRandom->Gaus(momPt, momPt * kPtResKink), mother.Eta(), mother.Phi(), kHypMass);
    recTriton.SetPtEtaPhiM(gRandom->Gaus(genH.GetDecay(0)->Pt(), genH.GetDecay(0)->Pt() * kPtRes), genH.GetDecay(0)->Eta(), genH.GetDecay(0)->Phi(), kTritonMass);
    TLorentzVector kinkPi0 = mother - recTriton;
    kinkPi0.SetPtEtaPhiM(kinkPi0.Pt(), kinkPi0.Eta(), kinkPi0.Phi(), kPi0Mass); // make sure that we know the mass
    hHyperMkink.Fill(momPt, (recTriton+kinkPi0).M());
  }

  TFile output("output.root", "recreate");
  hTritonPt.Write();
  hPi0Pt.Write();
  hGammaPt.Write();
  hHyperMkink.Write();
}