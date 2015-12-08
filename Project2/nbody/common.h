#ifndef COMMON_H
#define COMMON_H

#define XMIN 0.0
#define YMIN 0.0
#define XMAX 1.0
#define YMAX 1.0

float compute_LJ_scalar(float r2, float eps, float sig2);
float potential_LJ(float r2, float eps, float sig2);

void leapfrog1(int n, float dt, float* restrict x, 
               float* restrict v, float* restrict a);
void leapfrog2(int n, float dt, float* restrict v, float* restrict a);
void apply_reflect(int n, float* restrict x, float* restrict v, 
                   float* restrict a);

#endif /* COMMON_H */
