void BinLogAxis(TH1 *h) {
  //
  // Method for the correct logarithmic binning of histograms
  //
  TAxis* axis = h->GetXaxis();
  int bins = axis->GetNbins();
  double from = axis->GetXmin();
  double to = axis->GetXmax();
  double *newBins = new double[bins + 1];

  newBins[0] = from;
  double factor = pow(to / from, 1. / bins);

  for (int i = 1; i <= bins; i++) {
    newBins[i] = factor * newBins[i - 1];
  }
  axis->Set(bins, newBins);
  delete[] newBins;
}

void betaRes(float timeres=20.e-12f, float l = 0.4) {
  TCanvas* cv = new TCanvas();
  TH2D* res = new TH2D("res",";#it{p} (GeV/#it{c}); #beta;", 2000, 0.1, 20.1, 360, 0.2, 1.1); 
  TH2D* res2 = new TH2D("res2",";#it{p} (GeV/#it{c}); #beta;", 2000, 0.1, 20.1, 360, 0.2, 1.1); 
  BinLogAxis(res);
  BinLogAxis(res2);
  for (int iP{1}; iP <= res->GetNbinsX(); ++iP) {
    TAxis* pAx = res->GetXaxis();
    float mom = pAx->GetBinCenter(iP);  
    for (int i{0}; i <= 8; ++i) {
      double origB{mom / std::hypot(AliPID::ParticleMass(i), mom)}; 
      double origT{l / (origB * TMath::C())};
      TH2* hist = AliPID::ParticleCharge(i) == 1 ? res : res2;
      for (int j{0}; j < 1000; ++j) {
        hist->Fill(mom, l / ((origT + gRandom->Gaus(0., timeres)) * TMath::C())); 
      }
    }
  }
  cv->SetLogx();
  res->Draw("colz");
  cv->SaveAs("cv1.png");
  TCanvas* cv2 = new TCanvas("cv2");
  cv2->SetLogx();
  res2->Draw("colz");
  cv2->SaveAs("cv2.png");
}
