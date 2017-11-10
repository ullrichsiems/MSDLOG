#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <inttypes.h>
#include <cmath>
#include <random>
#include "lgq1d.h"

using namespace std;

std::random_device rd;
std::mt19937 gen(rd());

LGq1D::LGq1D(int, char **) {
    ReadInput();
    PrintParameter();
    Init();
    Run();
}

void LGq1D::ReadInput() {
    // default
    n_x = 1000;
    n_y = 2;
    n_z = 1;
    d = 2;
    p = 0.5;
    is_random = false;
    is_periodic = false;
    x_periodic = true;

    eq_step = 1e4;
    n_step = 1e4;
    n_samp = 1;
    // read input
    if ((pFile = fopen("input.para", "r")) == NULL) {
        printf("No parameter file: input.para\n");
        printf("Using default parameter\n");
    } else {
        char line[200];
        char *var;
        char *val;

        while (fgets(line, sizeof(line), pFile) != NULL) {
            var = strtok(line, " \t\n");
            val = strtok(NULL, " \t\n");
            if (strcmp("n_step", var) == 0) {
                n_step = atoi(val);
            } else if (strcmp("eq_step", var) == 0) {
                eq_step = atoi(val);
            } else if (strcmp("n_x", var) == 0) {
                n_x = atoi(val);
            } else if (strcmp("n_y", var) == 0) {
                n_y = atoi(val);
            } else if (strcmp("n_z", var) == 0) {
                n_z = atoi(val);
            } else if (strcmp("d", var) == 0) {
                d = atoi(val);
            } else if (strcmp("is_random", var) == 0) {
                is_random = atoi(val);
            } else if (strcmp("is_periodic", var) == 0) {
                is_periodic = atoi(val);
            } else if (strcmp("x_periodic", var) == 0) {
                x_periodic = atoi(val);
            } else if (strcmp("p", var) == 0) {
                p = atof(val); // asep (asymmetriefaktor) (ssep=0.5,tasep=1)
            } else {
                printf("Unknown var in input.para.\n");
                exit(EXIT_FAILURE);
            }
        }
        fclose(pFile);
    }
}

void LGq1D::PrintParameter() {
    printf("n_x = %d\n", n_x);
    printf("n_y = %d\n", n_y);
    printf("n_z = %d\n", n_z);

    printf("d = %d\n", d);
    printf("n_step = %d\n", n_step);
}

void LGq1D::Init() {
    DefineLattice();
    FillLattice();
}

void LGq1D::Run() {
    printf("n_part = %d\n", n_part);
    printf("length = %d\n", n_x);

    pFile = fopen("SLattice.dat", "w");
    PrintLattice(pFile);
    fclose(pFile);

    for (int i = 0; i < eq_step; i++) {
        MakeStep();
    }

    pFile = fopen("MLattice.dat", "w");
    PrintLattice(pFile);
    fclose(pFile);
    printf("EQ fertig\n");

    MSDInit();
    for (int i = 0; i < n_step; i++) {
        MSDSample(i);
        MakeStep();
    }
    MSDPrint();

    pFile = fopen("ELattice.dat", "w");
    PrintLattice(pFile);
    fclose(pFile);
}

void LGq1D::DefineLattice() {
    lattice = new bool **[n_x];
    for (int i = 0; i < n_x; i++) {
        lattice[i] = new bool *[n_y];
        for (int j = 0; j < n_y; j++) {
            lattice[i][j] = new bool[n_z];
        }
    }
}

void LGq1D::FillLattice() {
    n_part = 0;
    if (is_random == 0) {
        for (int i = 0; i < n_x; i++) {
            for (int j = 0; j < n_y; j++) {
                for (int h = 0; h < n_z; h += 2) {
                    if (((i + j * d) % (2 * d)) == 0) {
                        lattice[i][j][h] = true;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = false;
                        }
                        n_part++;
                    } else if (((i + j * d) % (2 * d)) == d) {
                        lattice[i][j][h] = false;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = true;
                            n_part++;
                        }
                    } else {
                        lattice[i][j][h] = false;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = false;
                        }
                    }
                }
            }
        }
    } else {
        std::uniform_int_distribution<int> randx(0, n_x - 1);
        std::uniform_int_distribution<int> randy(0, n_y - 1);
        std::uniform_int_distribution<int> randz(0, n_z - 1);
        n_part = ((n_x + 1) * n_y * n_z) / (2 * d);
        for (int n = 0; n < n_part; n++) {
            int i = randx(gen);
            int j = randy(gen);
            int h = randz(gen);
            if (lattice[i][j][h] == false) {
                lattice[i][j][h] = true;
            } else {
                n--;
            }
        }
    }

    v = new int[n_part];
    image = new int[n_part];
    x = new int *[n_part];
    for (int i = 0; i < n_part; i++) {
        x[i] = new int[3];
        image[i] = 0;
    }

    int n = 0;
    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            for (int h = 0; h < n_z; h++) {
                if (lattice[i][j][h]) {
                    x[n][0] = i;
                    x[n][1] = j;
                    x[n][2] = h;
                    n++;
                }
            }
        }
    }
}

void LGq1D::MakeStep() {
    std::uniform_real_distribution<double> d1(0, 1);
    std::uniform_int_distribution<int> d2(0, n_part - 1);
    for (int i = 0; i < n_part; i++) {
        v[i] = 0;
    }

    for (int i = 0; i < n_part; i++) {
        // int j = rand() % N;
        int j = d2(gen);
        int dx;
        int dimg = 0;
        // if((rand()%2) == 0){
        if (p == 1 || d1(gen) < p) {
            // move right
            next = x[j][0] + 1;
            if (next >= n_x) {
                if (x_periodic) {
                    dimg = +1;
                    next -= n_x;
                } else {
                    continue;
                }
            }
            dx = 1;
        } else {
            // move left
            next = x[j][0] - 1;
            if (next < 0) {
                if (x_periodic) {
                    dimg = -1;
                    next += n_x;
                } else {
                    continue;
                }
            }
            dx = -1;
        }
        int left = (x[j][1] + 1);
        int right = (x[j][1] - 1);
        if (n_y == 1) {
            left = right = 0;
        } else {
            if (is_periodic == true) {
                if (left >= n_y)
                    left = 0;
                else if (right < 0)
                    right = n_y - 1;
            } else {
                if (left >= n_y)
                    left = right;
                else if (right < 0)
                    right = left;
            }
        }
        int up = (x[j][2] + 1);
        int down = (x[j][2] - 1);
        if (n_z == 1) {
            up = down = 0;
        } else {
            if (is_periodic == true) {
                if (up >= n_z)
                    up = 0;
                else if (down < 0)
                    down = n_z - 1;
            } else {
                if (up >= n_z)
                    up = down;
                else if (down < 0)
                    down = up;
            }
        }
        // printf("%d,%dnext%d up%d down%d\n",v[j][0],v[j][1],next,up,down);
        if (lattice[next][left][x[j][2]] == false) {
            if (lattice[next][right][x[j][2]] == false) {
                if (lattice[next][x[j][1]][up] == false) {
                    if (lattice[next][x[j][1]][down] == false) {
                        lattice[x[j][0]][x[j][1]][x[j][2]] = false;
                        x[j][0] = next;
                        image[j] += dimg;
                        v[j] += dx;
                        lattice[x[j][0]][x[j][1]][x[j][2]] = true;
                    } else {
                        // i--;
                    }
                } else {
                    // i--;
                }
            } else {
                // i--;
            }
        } else {
            // i--;
        }
    }
}

void LGq1D::PrintLattice(FILE *pFile) {
    for (int i = 0; i < n_part; i++) {
        fprintf(pFile, "%d\t%d\t%d\n", x[i][0], x[i][1], x[i][2]);
    }
}

void LGq1D::MSDInit() {
    double a = pow(10, 1.0 / 25);
    int n_min = static_cast<int>(-log(a - 1) / log(a)) + 1;
    n_max = static_cast<int>(log(n_step) / log(a));
    n_max -= n_min;
    printf("n_max = %d\n", n_max);
    ntime = new int[n_max];
    nntime = new int[n_max];
    counter = new int[n_max];
    r2t = new int64_t[n_max];
    msd = new double[n_max];
    mean_velocity = new double[n_max];
    sample = new int *[n_max];
    xo = new int **[n_max];
    imageo = new int **[n_max];

    for (int n = 0; n < n_max; n++) {
        ntime[n] = static_cast<int>(pow(a, n + 1 + n_min) - pow(a, n_min));
        if (ntime[n] < n)
            ntime[n] = n;
        nntime[n] = static_cast<int>(pow(ntime[n], log(30) / log(n_step)));
        counter[n] = 0;
        r2t[n] = 0;
        msd[n] = 0.0;
        xo[n] = new int *[nntime[n]];
        imageo[n] = new int *[nntime[n]];
        sample[n] = new int[nntime[n]];
        for (int nn = 0; nn < nntime[n]; nn++) {
            sample[n][nn] = ntime[n] * nn / nntime[n];
            xo[n][nn] = new int[n_part];
            imageo[n][nn] = new int[n_part];
        }
    }
}

void LGq1D::MSDSample(int tstep) {
    for (int n = 0; n < n_max; n++) {
        for (int nn = 0; nn < nntime[n]; nn++) {
            if ((tstep % ntime[n]) == sample[n][nn]) {
                if (tstep > sample[n][nn]) {
                    double drift = 0;
                    for (int i = 0; i < n_part; i++) {
                        int dx = x[i][0] - xo[n][nn][i];
                        dx += (image[i] - imageo[n][nn][i]) * n_x;
                        drift += static_cast<double>(dx);
                        msd[n] += pow(static_cast<double>(dx), 2.0);
                        r2t[n] += dx * dx;
                    }
                    msd[n] -= pow(drift, 2) / n_step;
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

void LGq1D::MSDPrint() {
    FILE *pFile;
    pFile = fopen("MSD.dat", "w");
    for (int n = 0; n < n_max; n++) {
        double msda = static_cast<double>(r2t[n]) / counter[n] / n_part;
        msd[n] /= static_cast<double>(counter[n]);
        msd[n] /= static_cast<double>(n_part);
        fprintf(pFile, "%d\t%12.10f\t%12.10f\t%d\n", ntime[n], msd[n], msda,
                counter[n]);
    }
    fclose(pFile);
}

LGq1D::~LGq1D() {
    for (int n = 0; n < n_max; n++) {
        for (int nn = 0; nn < nntime[n]; nn++) {
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
    delete[] r2t;
    delete[] msd;
    delete[] mean_velocity;
    delete[] ntime;
    delete[] nntime;
    delete[] counter;

    for (int i = 0; i < n_part; i++)
        delete[] x[i];
    delete[] x;
    delete[] v;
    delete[] image;
    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            delete[] lattice[i][j];
        }
        delete[] lattice[i];
    }
    delete[] lattice;
}
