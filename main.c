#include "dftt.h"

//TODO: Plotting? Create a dfttplot? Figure out gnuplot?
//TODO: RadixM? 
//TODO: Decimation in frequency?
//TODO: Add auto-naming of files from the twc project
//BUG: Issue when compiling with -O3.

int main (int argc, char** argv) {
    double* x;                  // Input data
    double complex* X = NULL;   // Fourier Transform result
    double** X_RIB = NULL;      // Array containing the real, and imaginary data, and frequency bins.
    dftt_config_t dftt_conf;    // Tool config

    /* Set defaults to ensure certain behaviour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    /* Execute the read input function  */
    CHECK_ERR(dftt_conf.inp(&dftt_conf, &x));

    /* Execute any potential windowing */
    dftt_conf.w(&dftt_conf, x);

    /* Set the array sizes to be used in the DFT */
    set_transform_size(&dftt_conf, &X, &x);

    /* Start the timer */
    check_start_timer(&dftt_conf);

    /* DFT */
    dftt_conf.dft(&dftt_conf, X, x);

    /* Check if timer was activated and output time */
    check_end_timer_output(&dftt_conf);

    /* Dissects the complex values array into frequency bins, real, and imaginary numbers */
    dissect_complex_arr(X, &X_RIB, dftt_conf.total_samples);

    prep_outp(&dftt_conf, X_RIB);

    /* Output the DFT array */
    dftt_conf.outp(&dftt_conf, X_RIB);

    free(x);
    free(X);
    free(X_RIB);

    return 0;
}
