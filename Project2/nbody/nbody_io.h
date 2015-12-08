#ifndef NBODY_IO_H
#define NBODY_IO_H

void write_header(FILE* fp, int n);
void write_frame_data(FILE* fp, int n, float* x);

#endif /* NBODY_IO_H */
