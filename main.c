#include "dftt.h"

//FIX: FFT output
//FIX: How tf do I implement different bins. Made the option take the data but implementation is wrong.
//TODO: Add different info output functions depending on the process used
//TODO: Create a tool to extract the data from an audio file (adx - audio data extractor)

int main (int argc, char** argv) {
    double* x;              // Input data
    FILE * ofile;           // Output file pointer
    double complex* X;      // Fourier Transform result
    dftt_config_t dftt_conf; // Tool config

    /* Set defaults to ensure certain behaviour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    /* Execute the read input function  */
    CHECK_ERR(dftt_conf.inp(&x, &dftt_conf));

    /* Start the timer */
    check_start_timer(&dftt_conf);

    /* Initialise DFT result array */
    X = NULL;

    /* DFT */
    dftt_conf.dft(&X, x, &dftt_conf);

    /* Set the zeros based on the tolerance */
    set_zeros(X, &dftt_conf);

    /* Initialise outuput buffer and output the DFT array */
    ofile = NULL;
    dftt_conf.outp(&ofile, &dftt_conf, X);

    /* Check if timer was activated and output time if yes */
    check_end_timer_output(&dftt_conf);

    fclose(ofile);
    free(x);
    free(X);

    return 0;
}
