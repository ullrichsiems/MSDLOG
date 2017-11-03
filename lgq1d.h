
#ifndef lgq1d_h
#define lgq1d_h

#include <random>

class LGq1D {
  public:
    LGq1D(int, char **);
    ~LGq1D();

  private:
    void ReadInput();
    void PrintParameter();
    void Init();
    void Run();
    void DefineLattice();
    void FillLattice();
    void MakeStep();
    void PrintLattice(FILE *);
    bool ***lattice;
    int **x;
    int *image;
    int *v;
    int n_part;

    int n_x; // m
    int n_y; // k
    int n_z; // l
    int d;
    double p;
    bool is_periodic;
    bool is_random;
    int n_step;
    int eq_step;
    int n_samp;
    int next;
    FILE *pFile;

    // msd
    void MSDInit();
    void MSDSample(int tstep);
    void MSDPrint();
    int ***xo;
    int ***imageo;
    int64_t *r2t;
    double *msd;
    double *mean_velocity;
    int *ntime;
    int *nntime;
    int **sample;
    int *counter;

    int n_max;
    int displ;
};
#endif
