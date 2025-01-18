#include "dftt.h"

//TODO: Set up output method - file (comma separated, value on each line, hex dump?, C array, Python list) or stdout
//TODO: Choose what to output, real part, imag part, power
//TODO: FFT
//TODO: Test multiple channels and different formats 

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

    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    SNDFILE* file = NULL;          // Pointer to file
    CHECK_ERR(open_file(&file, &sf_info, &dftt_conf));

    double* x = NULL;              // Data from audio file, considered the input signal
    CHECK_ERR(read_file(file, &sf_info, &dftt_conf, &x));

    output_info(&sf_info, &dftt_conf);

    X_real = calloc(sf_info.frames, sizeof(double));
    X_imag = calloc(sf_info.frames, sizeof(double));
    Pow = calloc(sf_info.frames, sizeof(double));
    
    printf("\n\tWorking on it...\n");
    dft(X_real, X_imag, Pow, &sf_info.frames, x);

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
