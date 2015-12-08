#include <stdio.h>


#define VERSION_TAG "NBView00 "


void write_header(FILE* fp, int n)
{
    fprintf(fp, "%s%d 1\n", VERSION_TAG, n);
}


void write_frame_data(FILE* fp, int n, float* x)
{
    for (int i = 0; i < n; ++i) {
        float xi = *x++;
        float yi = *x++;
        fprintf(fp, "%e %e\n", xi, yi);
    }
}
