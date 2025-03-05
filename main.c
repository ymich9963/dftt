#include "dftt.h"

//TODO: Plotting?
//TODO: RadixM?
//TODO: Show sampling frequency in output info and use the new bin flag for outputting the bins

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

    /* Initialise DFT result array */
    X = NULL;

    /* Set the array sizes to be used in the DFT */
    set_transform_size(&dftt_conf, &X, &x);

    /* Start the timer */
    check_start_timer(&dftt_conf);

    /* DFT */
    dftt_conf.dft(X, x, &dftt_conf);

    /* Check if timer was activated and output time */
    check_end_timer_output(&dftt_conf);

    /* Calculate power spectrum if selected */
    pow_spec(X, &dftt_conf);

    /* Initialise outuput buffer and output the DFT array */
    ofile = NULL;
    dftt_conf.outp(&ofile, &dftt_conf, X);

    fclose(ofile);
    free(x);
    free(X);

    return 0;
}
