#include "dftt.h"

//TODO: Timer
//TODO: FFT
//FIX: How tf do I implement different bins. Made the option take the data but implementation is wrong.

int main (int argc, char** argv) {
    SNDFILE* file;          // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info
    FILE * ofile;           // Output file pointer
    double* x;              // Data from audio file, considered the input signal
    double* x_mono;
    double* X_real;         // DFT of the Real part
    double* X_imag;         // DFT of the Imaginary part
    double complex* X;

    dftt_config_t dftt_conf; // Tool config

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Set defaults to ensure certain behabiour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    file = NULL;          // Pointer to file
    CHECK_ERR(open_file(&file, &sf_info, &dftt_conf));

    /* Output info on the inputted file */
    output_info(&sf_info, &dftt_conf);

    x = NULL;              // Data from audio file, considered the input signal
    CHECK_ERR(read_file_data(file, &sf_info, &dftt_conf, &x));

    /* Translate the data to one channel (mono) */
    x_mono = NULL;
    mix2mono(&sf_info, x, &x_mono);

    printf("\tWorking on it...\n");

    /* Allocate */
    X_real = calloc(sf_info.frames, sizeof(double));
    X_imag = calloc(sf_info.frames, sizeof(double));
    X = calloc(sf_info.frames, sizeof(double complex));

    /* DFT and Power Spectrum Calculation */
    dft(X, &sf_info.frames, x_mono);

    /* Set the zeros based on the tolerance */
    set_zeros(X, &sf_info.frames, &dftt_conf);

    ofile = NULL;
    dftt_conf.outp(&ofile, &sf_info, &dftt_conf, X);
    printf("\tOutputted data to '%s'!\n\n", dftt_conf.ofile);

    sf_close(file);
    fclose(ofile);
    free(x);
    free(X_real);
    free(X_imag);
    return 0;
}
