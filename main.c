#include "dftt.h"

//TODO: Set up output method - C array, Python list
//TODO: Choose what to output, real part, imag part, power
//TODO: FFT
//FIX: How tf do I implement different bins. Made the option take the data but implementation is wrong.

int main (int argc, char** argv) {
    SF_INFO sf_info;        // File info
    double* X_real;         // DFT of the Real part
    double* X_imag;         // DFT of the Imaginary part
    double* Pow;            // Power spectrum

    dftt_config_t dftt_conf; // Tool config

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Might be necessary?   */
    /* sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT; */

    /* Set defaults to ensure certain behabiour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    SNDFILE* file = NULL;          // Pointer to file
    CHECK_ERR(open_file(&file, &sf_info, &dftt_conf));

    /* Output info on the inputted file */
    output_info(&sf_info, &dftt_conf);

    /* Set the DFT bin count */
    set_config(&sf_info, &dftt_conf);

    double* x = NULL;              // Data from audio file, considered the input signal
    CHECK_ERR(read_file_data(file, &sf_info, &dftt_conf, &x));

    X_real = calloc(sf_info.frames, sizeof(double));
    X_imag = calloc(sf_info.frames, sizeof(double));
    Pow = calloc(sf_info.frames, sizeof(double));

    /* Translate the data to one channel (mono) */
    double* x_mono = NULL;
    mix2mono(&sf_info, x, &x_mono);

    printf("\n\tWorking on it...\n");
    dft(X_real, X_imag, Pow, &sf_info.frames, x_mono);

    FILE * ofile = NULL;
    dftt_conf.outp(&ofile, &sf_info, &dftt_conf, Pow);
    printf("\n\tOutputted data to '%s'!\n\n", dftt_conf.ofile);

    sf_close(file);
    fclose(ofile);
    free(x);
    free(X_real);
    free(X_imag);
    free(Pow);
    return 0;
}
