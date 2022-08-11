{
  Printf("Starting ROOT version %s.", gROOT->GetVersion());

  if (gSystem->Getenv("TMPDIR"))
    gSystem->SetBuildDir(gSystem->Getenv("TMPDIR"));

  const string pwd = gSystem->WorkingDirectory();
  const string inc = pwd;
  const string aliroot = gSystem->Getenv("ALICE_ROOT") ? "$ALICE_ROOT/include" : "";
  const string aliphysics = gSystem->Getenv("ALICE_PHYSICS") ? "$ALICE_PHYSICS/include" : "";
  const string o2_inc = gSystem->Getenv("O2_ROOT") ? "$O2_ROOT/include" : "";
  const string gsl_inc = gSystem->Getenv("MS_GSL_ROOT") ? "$MS_GSL_ROOT/include" : "";
  const string o2gpu_inc = gSystem->Getenv("O2_ROOT") ? "$O2_ROOT/include/GPU" : "";
  const string utils_inc = "$HOME/Desktop/Repositories/Utils/include";

  // Include paths
  // =============
  // There are 2 separate paths: one for ACLiC, and one for CINT or CLING.
  // 1. ACLiC include path
  gSystem->AddIncludePath(Form("-I%s ", inc.data()));
  gSystem->AddIncludePath(Form("-I%s ", utils_inc.data()));
  if (aliroot != "") gSystem->AddIncludePath(Form("-I%s ", aliroot.data()));
  if (aliphysics != "") {
    gSystem->AddIncludePath(Form("-I%s ", aliphysics.data()));
    gSystem->Load("liblhapdf");
    gSystem->Load("libEGPythia6");
    gSystem->Load("libpythia6_4_25");
    gSystem->Load("libAliPythia6");
  }
  if (!o2_inc.empty()) gSystem->AddIncludePath(Form("-I%s ", o2_inc.data()));
  if (!gsl_inc.empty()) gSystem->AddIncludePath(Form("-I%s ", gsl_inc.data()));
  if (!o2gpu_inc.empty()) gSystem->AddIncludePath(Form("-I%s ", o2gpu_inc.data()));
  // 2. Interpreter include path
  // Type .include (ROOT 5) or .I (ROOT 6) at the ROOT REPL to see a listing
#ifdef __CINT__
#define CONSTEXPR const
  gROOT->ProcessLine(Form(".include %s", inc.data()));
  gROOT->ProcessLine(Form(".include %s", utils_inc.data()));
  if (aliroot != "") gROOT->ProcessLine(Form(".include %s", aliroot.data()));
  if (aliphysics != "") gROOT->ProcessLine(Form(".include %s", aliphysics.data()));
#endif
#ifdef __CLING__
#define CONSTEXPR constexpr
  gROOT->ProcessLine(Form(".I %s", inc.data()));
  gROOT->ProcessLine(Form(".I %s", utils_inc.data()));
  if (aliroot != "") gROOT->ProcessLine(Form(".I %s", aliroot.data()));
  if (aliphysics != "") gROOT->ProcessLine(Form(".I %s", aliphysics.data()));
#endif

  gStyle->SetEndErrorSize(0.);
  gStyle->SetOptStat(1);
  gStyle->SetOptDate(0);
  gStyle->SetOptFit(1);
  gStyle->SetLabelSize(0.04,"xyz"); // size of axis value font
  gStyle->SetTitleSize(0.05,"xyz"); // size of axis title font
  gStyle->SetTitleFont(42,"xyz"); // font option
  gStyle->SetLabelFont(42,"xyz");
  gStyle->SetTitleOffset(1.05,"x");
  gStyle->SetTitleOffset(1.1,"y");
  // default canvas options
  gStyle->SetCanvasDefW(800);
  gStyle->SetCanvasDefH(600);
  gStyle->SetPadBottomMargin(0.12); //margins...
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadGridX(0); // grids, tickmarks
  gStyle->SetPadGridY(0);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetPaperSize(20,24); // US letter size
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFillColor(0);

  // Rainbow colors
  const int __colors[10] = {TColor::GetColor("#ff3300"), TColor::GetColor("#ec6e0a"), TColor::GetColor("#daaa14"), TColor::GetColor("#c7e51e"), TColor::GetColor("#85dd69"), TColor::GetColor("#42d6b4"), TColor::GetColor("#00ceff"), TColor::GetColor("#009adf"), TColor::GetColor("#0067c0"), TColor::GetColor("#0033a1")};

  // Replace the defunct AliPID
  CONSTEXPR int __NumParticles = 9;
  CONSTEXPR int __PdgCode[9] = {11, 13, 211, 321, 2212, 1000010020, 1000010030, 1000020030, 1000020040};
  CONSTEXPR double __ParticleMass[9] = {0.000510999, 0.105658, 0.13957, 0.493677, 0.938272, 1.87561, 2.80892, 2.80839, 3.72738};
  CONSTEXPR double __ParticleCharge[9] = {1., 1., 1., 1., 1., 1., 1., 2., 2.};
  const char* __ParticleName[9] = {"e", "#mu", "#pi", "K", "p", "d", "t", "^{3}He", "#alpha"};

  return;
}
