#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void dft() {

}

int main (int argc, char** argv) {
    SNDFILE* file;          // Pointer to file
    SF_INFO sf_info;        // File info
    double* x;              // Data from audio file, considered the input signal
    sf_count_t sf_count;    // To count number of data read
    long N;                 // Number of samples
    long n;                 // Sample index in time domain
    long k;                 // Sample index in frequency domain
    double* X_real;         // DFT of the Real part
    double* X_imag;         // DFT of the Imaginary part
    double* P;              // Power spectrum

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Might be necessary?   */
    /* sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT; */

    file = sf_open("wave.wav", SFM_READ, &sf_info);
    if(!file) {
        fprintf(stderr, "\n%s.\n\n", sf_strerror(file));
        return 1;
    };

    x = calloc(sf_info.frames, sizeof(double));
    sf_count = sf_readf_double(file, x, sf_info.frames);
    if (sf_count != sf_info.frames) {
        fprintf(stderr, "\nRead count not equal to requested frames.\n\n");
    }

    printf("\n");
    printf("samplerate: %d\n", sf_info.samplerate);
    printf("frames: %lld\n", sf_info.frames);
    printf("channels: %d\n", sf_info.channels);
    printf("sf count: %lld\n", sf_count);
    printf("data: %lf\n", x[100]);
    printf("\n");

    N = sf_info.frames;
    X_real = calloc(sf_info.frames, sizeof(double));
    X_imag = calloc(sf_info.frames, sizeof(double));
    P = calloc(sf_info.frames, sizeof(double));

    /* Calculate DFT */
    for (k = 0; k < N; k++) {
        for (n = 0; n < N; n++) { 
            X_real[k] += x[n] * cos(2 * M_PI * n * k / N);
            X_imag[k] -= x[n] * sin(2 * M_PI * n * k / N);
        }

        /* Calculate the power spectrum of x[n] */
        P[k] = (X_real[k] * X_real[k]) + (X_imag[k] * X_imag[k]);
    }

    FILE * ofile = fopen("out.txt", "w");
    
    for (long c = 0; c < N; c++){
        fprintf(ofile, "%lf\n", P[c]);
    }

    sf_close(file);
    fclose(ofile);
    free(x);
    free(X_real);
    free(X_imag);
    free(P);
    return 0;
}
