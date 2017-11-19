#ifndef msdlog_h
#define msdlog_h

class MSDLOG {
  public:
    MSDLOG();
    ~MSDLOG();
    void Init(int nstep, int n_part, double x_length);
    void Sample(int tstep, int **x, int *image);
    void Finish(char filename[100]);

  private:
    FILE *pFile;

    int n_max;
    int n_part;
    int length;
    int *time;
    int *rate;
    int **sample;

    int ***xo;
    int ***imageo;

    int64_t *r2t;
    int64_t *counter;

    double *msd;
    double *mean_velocity;
};
#endif // msdlog_h
