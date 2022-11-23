#pragma cling add_library_path("build/lib")
#pragma cling add_include_path("include")
#pragma cling add_include_path("build/include")
#pragma cling load("libThermalFIST.so")
#pragma cling load("libMinuit2.so")

#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <vector>
#include <string>

#include "HRGBase.h"
#include "HRGEV.h"
#include "HRGFit.h"
#include "HRGVDW.h"

#include "ThermalFISTConfig.h"
