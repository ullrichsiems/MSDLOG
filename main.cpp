#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <random>
#include "lgq1d.h"

using namespace std;

int main(int argc, char *argv[]) {
    LGq1D *lg = new LGq1D(argc, argv);
    delete lg;
}
