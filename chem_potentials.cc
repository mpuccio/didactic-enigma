#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cmath>

#include "HRGBase.h"
#include "HRGEV.h"
#include "HRGFit.h"
#include "HRGVDW.h"

#include "ThermalFISTConfig.h"

#include <TH2D.h>

using namespace std;

#ifdef ThermalFIST_USENAMESPACE
using namespace thermalfist;
#endif

#include "data.h"

int chem_potentials()
{
  std::vector<FittedQuantity> quantities;
  double values[6][4][5];
  for (int i{0}; i < 4; ++i)
  {
    for (int j{0}; j < 5; ++j)
    {
      values[0][i][j] = pion[i][j];
      values[1][i][j] = proton[i][j];
      values[2][i][j] = he3[i][j];
      values[3][i][j] = triton[i][j];
      values[4][i][j] = hypertriton[i][j];
      values[5][i][j] = omega[i][j];
    }
  }

  // Particle list file
  string listname = "/home/mpuccio/alice/Thermal-FIST/input/list/PDG2020/list-withnuclei.dat";

  // Create the hadron list instance and read the list from file
  ThermalParticleSystem TPS(listname);

  string modeltype; // For output

  // Pointer to the thermal model instance used in calculations
  ThermalModelBase *model = new ThermalModelIdeal(&TPS);
  modeltype = "Id-HRG";
  model->ConstrainMuQ(false);
  model->ConstrainMuB(false);
  model->ConstrainMuS(true);

  // Use quantum statistics
  model->SetStatistics(true);

  // Use mass integration over Breit-Wigner shapes in +-2Gamma interval, as in THERMUS
  model->SetUseWidth(ThermalParticle::BWTwoGamma);
  // model->SetUseWidth(ThermalParticle::ZeroWidth);

  // Set chemical potentials to zero
  model->SetBaryonChemicalPotential(0.0);
  model->SetElectricChemicalPotential(0.0);
  model->SetStrangenessChemicalPotential(0.0);
  model->SetCharmChemicalPotential(0.0);
  model->FillChemicalPotentials();

  // Prepare fitter
  ThermalModelFit fitter(model);

  // By default T, muB, and R parameters are fitted, while others (gammaS etc.) are fixed
  // Initial parameters values are taken from those currently set in the ThermalModel object
  // Here we do not fit muB, which is set to zero above
  fitter.SetParameterFitFlag("muB", false); // Do not fit muB
  fitter.SetParameterFitFlag("muQ", false); // Do not fit muB
  fitter.SetParameterFitFlag("R", false);   // Do not fit muB
  fitter.SetParameterFitFlag("T", false);   // Do not fit muB

  double wt1 = get_wall_time(); // Timing

  int iters = 0; // Number of data points

  // Temperature interval, in GeV
  double muBmin = 0.000;
  double muBmax = 0.002;
  double muQmin = -0.002;
  double muQmax = 0.002;
  double dmuQ = 0.00004;
  double dmuB = 0.00001;

  fitter.SetParameterFitFlag("T", false);
  fitter.SetParameterValue("T", 0.155); // Set the temperature

  TFile fout("fout.root", "recreate");

  for (int centrality{0}; centrality < 5; ++centrality)
  {
    for (int i{0}; i < 6; ++i)
    {
      if (values[i][0][centrality] < 0)
      {
        continue;
      }
      ExperimentRatio ratio;
      ratio.fPDGID1 = -pdg[i];
      ratio.fPDGID2 = pdg[i];
      ratio.fValue = values[i][0][centrality];
      ratio.fError = std::hypot(values[i][1][centrality], values[i][2][centrality]);
      ratio.fFeedDown1 = Feeddown::Electromagnetic;
      ratio.fFeedDown2 = Feeddown::Electromagnetic;
      quantities.emplace_back(ratio);
    }
    // Load the experimental data
    fitter.SetQuantities(quantities);
    char tmpc[1000];
    sprintf(tmpc, "ratios%i.out", centrality);
    ThermalModelFit::saveExpDataToFile(quantities, tmpc);

    TH2D chem(Form("chem%i", centrality), ";#mu_{B} (MeV);#mu_{Q} (MeV);#chi^{2}", 100, 0., 2., 100, -1., 1.);
    TH2D chemFixMuQ(Form("chemFixMuQ%i", centrality), ";#mu_{B} (MeV);#mu_{Q} (MeV);#chi^{2}", 100, 0., 2., 100, -1., 1.);
    TH2D chemFixMuQ_nopions(Form("chemFixMuQ_nopions%i", centrality), ";#mu_{B} (MeV);#mu_{Q} (MeV);#chi^{2}", 100, 0., 2., 100, -1., 1.);

    for (double iB = 1; iB <= chem.GetNbinsX(); iB++)
    {
      double muB = chem.GetXaxis()->GetBinCenter(iB) * 1.e-3;
      model->ConstrainMuQ(false);
      quantities[0].toFit = true;
      fitter.SetQuantities(quantities);

      for (double iQ = 1; iQ <= chem.GetNbinsY(); iQ++)
      {
        double muQ = chem.GetYaxis()->GetBinCenter(iQ) * 1.e-3;
        fitter.SetParameterValue("muB", muB);
        fitter.SetParameterValue("muQ", muQ);

        ThermalModelFitParameters result = fitter.PerformFit(false); // We still have to fit the radius, the argument suppresses the output during minimization

        chem.SetBinContent(iB, iQ, result.chi2);
        iters++;
      }
      model->ConstrainMuQ(true);
      ThermalModelFitParameters result = fitter.PerformFit(false); // We still have to fit the radius, the argument suppresses the output during minimization
      chemFixMuQ.SetBinContent(iB, chem.GetYaxis()->FindBin(result.muQ.value * 1.e3f), result.chi2);
      result = fitter.PerformFit(false);
      quantities[0].toFit = false;
      fitter.SetQuantities(quantities);
      result = fitter.PerformFit(false);
      chemFixMuQ_nopions.SetBinContent(iB, chem.GetYaxis()->FindBin(result.muQ.value * 1.e3f), result.chi2);
    }
    chem.Write();
    chemFixMuQ.Write();

    chemFixMuQ_nopions.Write();
  }

  delete model;

  double wt2 = get_wall_time();

  printf("%30s %lf s\n", "Running time:", (wt2 - wt1));
  printf("%30s %lf s\n", "Time per single calculation:", (wt2 - wt1) / iters);

  return 0;
}
