#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include <cmath>
#include <ctime>
#include <random>
#include "lgq1d.h"

using namespace std;

int main(int argc, char *argv[]) {

    double stime = clock();

    LGq1D *lg = new LGq1D(argc, argv);
    delete lg;
    stime = clock() - stime;
    stime /= 1e6;
    if (stime < 3600) {
        printf("The program took %2.2f seconds.\n", stime);
    } else if (stime < 86400) {
        int h = int(floor(stime / 3600));
        printf("The program took %d hours and %2.2f seconds.\n", h,
               stime - h * 3600);
    } else {
        int h = int(floor(stime / 3600));
        int d = h / 24;
        printf("The program took %d days, %d hours and %2.2f seconds.\n", d,
               h - d * 24, stime - h * 3600);
    }
    return 0;
}
