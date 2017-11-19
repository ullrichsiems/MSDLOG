#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <inttypes.h>
#include <cmath>
#include "msdlog.h"

MSDLOG::MSDLOG() {
    n_max = 0;
    n_part = 0;
    time = nullptr;
    rate = nullptr;
    sample = nullptr;

    xo = nullptr;
    imageo = nullptr;
    counter = nullptr;

    msd = nullptr;
    mean_velocity = nullptr;
}

void MSDLOG::Init(int n_step, int npart, double x_length) {
    n_part = npart;
    length = x_length;
    double a = pow(10, 1.0 / 25);
    double b = log(30) / log(n_step);
    int n_min = static_cast<int>(-log(a - 1) / log(a)) + 1;
    n_max = static_cast<int>(log(n_step) / log(a));
    n_max -= n_min;
    printf("n_max = %d\n", n_max);

    time = new int[n_max];
    rate = new int[n_max];
    sample = new int *[n_max];

    xo = new int **[n_max];
    imageo = new int **[n_max];
    counter = new int64_t[n_max];

    msd = new double[n_max];
    mean_velocity = new double[n_max];

    for (int n = 0; n < n_max; n++) {
        time[n] = static_cast<int>(pow(a, n + 1 + n_min) - pow(a, n_min));
        if (time[n] < n)
            time[n] = n;
        rate[n] = static_cast<int>(pow(time[n], b));

        counter[n] = 0;
        msd[n] = 0.0;

        xo[n] = new int *[rate[n]];
        imageo[n] = new int *[rate[n]];
        sample[n] = new int[rate[n]];
        for (int nn = 0; nn < rate[n]; nn++) {
            sample[n][nn] = time[n] * nn / rate[n];
            xo[n][nn] = new int[n_part];
            imageo[n][nn] = new int[n_part];
        }
    }
}

void MSDLOG::Sample(int tstep, int **x, int *image) {
    for (int n = 0; n < n_max; n++) {
        for (int nn = 0; nn < rate[n]; nn++) {
            if ((tstep % time[n]) == sample[n][nn]) {
                if (tstep > sample[n][nn]) {
                    double drift = 0;
                    for (int i = 0; i < n_part; i++) {
                        double dx = x[i][0] - xo[n][nn][i];
                        dx += (image[i] - imageo[n][nn][i]) * length;
                        drift += dx;
                        msd[n] += dx * dx;
                    }
                    msd[n] -= (drift * drift) / n_part;
                    counter[n]++;
                }
                for (int i = 0; i < n_part; i++) {
                    xo[n][nn][i] = x[i][0];
                    imageo[n][nn][i] = image[i];
                }
            }
        }
    }
}

void MSDLOG::Finish(char filename[100]) {
    FILE *pFile;
    pFile = fopen(filename, "w");
    for (int n = 0; n < n_max; n++) {
        msd[n] /= static_cast<double>(counter[n]);
        msd[n] /= static_cast<double>(n_part);
        fprintf(pFile, "%d\t%12.10f\t%ld\n", time[n], msd[n], counter[n]);
    }
    fclose(pFile);
}

MSDLOG::~MSDLOG() {
    for (int n = 0; n < n_max; n++) {
        for (int nn = 0; nn < rate[n]; nn++) {
            delete[] xo[n][nn];
            delete[] imageo[n][nn];
        }
        delete[] xo[n];
        delete[] imageo[n];
        delete[] sample[n];
    }
    delete[] xo;
    delete[] imageo;
    delete[] sample;
    delete[] msd;
    delete[] counter;
    delete[] mean_velocity;
    delete[] time;
    delete[] rate;
}
