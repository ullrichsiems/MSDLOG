#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <inttypes.h>
#include <cmath>
#include <random>
#include "lgq1d.h"
#include "msdlog.h"

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
    // default parameter
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

    // Equilibration steps
    for (int i = 0; i < eq_step; i++) {
        MakeStep();
    }

    pFile = fopen("MLattice.dat", "w");
    PrintLattice(pFile);
    fclose(pFile);
    printf("EQ fertig\n");

    double length = 1.0 * n_x;
    MSDLOG *msdlog = new MSDLOG();
    msdlog->Init(n_step, n_part, length);
    for (int i = 0; i < n_step; i++) {
        msdlog->Sample(i, x, image);
        MakeStep();
    }

    char filename[100];
    sprintf(filename, "MSD.dat");
    msdlog->Finish(filename);

    pFile = fopen("ELattice.dat", "w");
    PrintLattice(pFile);
    fclose(pFile);
}

void LGq1D::DefineLattice() {
    lattice = new int **[n_x];
    for (int i = 0; i < n_x; i++) {
        lattice[i] = new int *[n_y];
        for (int j = 0; j < n_y; j++) {
            lattice[i][j] = new int[n_z];
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
                        lattice[i][j][h] = n_part;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = -1;
                        }
                        n_part++;
                    } else if (((i + j * d) % (2 * d)) == d) {
                        lattice[i][j][h] = -1;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = n_part;
                            n_part++;
                        }
                    } else {
                        lattice[i][j][h] = -1;
                        if ((h + 1) < n_z) {
                            lattice[i][j][h + 1] = -1;
                        }
                    }
                }
            }
        }
    } else {
        std::uniform_int_distribution<int> randx(0, n_x - 1);
        std::uniform_int_distribution<int> randy(0, n_y - 1);
        std::uniform_int_distribution<int> randz(0, n_z - 1);
        for (int i = 0; i < n_x; i++) {
            for (int j = 0; j < n_y; j++) {
                for (int h = 0; h < n_z; h++) {
                    lattice[i][j][h] = -1;
                }
            }
        }
        n_part = ((n_x + 1) * n_y * n_z) / (2 * d);
        for (int n = 0; n < n_part; n++) {
            int i = randx(gen);
            int j = randy(gen);
            int h = randz(gen);
            if (lattice[i][j][h] == -1) {
                lattice[i][j][h] = n;
            } else {
                n--;
            }
        }
    }

    v = new int[n_part];
    image = new int[n_part];
    x = new int *[n_part];
    nlist = new int *[n_part];
    for (int i = 0; i < n_part; i++) {
        x[i] = new int[3];
        nlist[i] = new int[8];
        image[i] = 0;
    }

    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            for (int h = 0; h < n_z; h++) {
                if (lattice[i][j][h] != -1) {
                    int n = lattice[i][j][h];
                    x[n][0] = i;
                    x[n][1] = j;
                    x[n][2] = h;
                }
            }
        }
    }
    for (int i = 0; i < n_part; i++) {
        bool l = false;
        bool r = false;
        bool u = false;
        bool d = false;
        int next = x[i][0] + 1;
        int left = x[i][1] + 1;
        int right = x[i][1] - 1;
        int up = x[i][2] + 1;
        int down = x[i][2] - 1;
        if (is_periodic == true) {
            if (up >= n_z)
                up = 0;
            else if (down < 0)
                down = n_z - 1;
            if (left >= n_y)
                left = 0;
            else if (right < 0)
                right = n_y - 1;
        } else {
            if (left >= n_y) {
                l = true;
                nlist[i][0] = i;
                nlist[i][2] = i;
            }
            if (right < 0) {
                r = true;
                nlist[i][1] = i;
                nlist[i][3] = i;
            }
            if (up >= n_z) {
                u = true;
                nlist[i][4] = i;
                nlist[i][6] = i;
            }
            if (down < 0) {
                d = true;
                nlist[i][5] = i;
                nlist[i][7] = i;
            }
        }

        while (1) {
            next++;
            if (next == n_x) {
                next = 0;
            }
            if (!(l) && lattice[next][left][x[i][2]] != -1) {
                int j = lattice[next][left][x[i][2]];
                nlist[i][0] = j;
                nlist[j][3] = i;
                l = true;
            }
            if (!(r) && lattice[next][right][x[i][2]] != -1) {
                int j = lattice[next][right][x[i][2]];
                nlist[i][1] = j;
                nlist[j][2] = i;
                r = true;
            }
            if (!(u) && lattice[next][x[i][1]][up] != -1) {
                int j = lattice[next][x[i][1]][up];
                nlist[i][4] = j;
                nlist[j][7] = i;
                u = true;
            }
            if (!(d) && lattice[next][x[i][1]][down] != -1) {
                int j = lattice[next][x[i][1]][down];
                nlist[i][5] = j;
                nlist[j][6] = i;
                d = true;
            }
            if (r && l && u && d) {
                break;
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

    for (int n = 0; n < n_part; n++) {
        int j = d2(gen);
        int dimg = 0;
        if (p == 1 || d1(gen) < p) {
            // move forwards
            next = x[j][0] + 1;
            if (next >= n_x) {
                if (x_periodic) {
                    dimg = +1;
                    next -= n_x;
                } else {
                    continue;
                }
            }
            if ((next != x[nlist[j][0]][0]) && (next != x[nlist[j][1]][0]) &&
                (next != x[nlist[j][4]][0]) && (next != x[nlist[j][5]][0])) {
                x[j][0] = next;
                image[j] += dimg;
                v[j]++;
            }
        } else {
            // move backwards
            next = x[j][0] - 1;
            if (next < 0) {
                if (x_periodic) {
                    dimg = -1;
                    next += n_x;
                } else {
                    continue;
                }
            }
            if ((next != x[nlist[j][2]][0]) && (next != x[nlist[j][3]][0]) &&
                (next != x[nlist[j][6]][0]) && (next != x[nlist[j][7]][0])) {
                x[j][0] = next;
                image[j] += dimg;
                v[j]--;
            }
        }
    }
}

void LGq1D::PrintLattice(FILE *pFile) {
    for (int i = 0; i < n_part; i++) {
        fprintf(pFile, "%d\t%d\t%d\n", x[i][0], x[i][1], x[i][2]);
    }
}

LGq1D::~LGq1D() {
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
