#include "dftt.h"

//TODO: Plotting? Create a dfttplot? Figure out gnuplot?
//TODO: Add dB units for power spectrum?
//TODO: More FFT algorithms! RadixM? Have a -M option.
//TODO: Add DFT for any file as binary
//TODO: Implement imaginary number input
//TODO: Add different format to the output like \g

int main (int argc, char** argv) {
    double* x;                  // Input data
    double complex* X = NULL;   // Fourier Transform result
    double** X_RIB = NULL;      // Array containing the real, and imaginary data, and frequency bins.
    dftt_config_t dftt_conf;    // Tool config

    printf("-DFTT START-\n");

    /* Set defaults to ensure certain behaviour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(argc, argv, &dftt_conf));

    /* Execute the read input function  */
    CHECK_ERR(dftt_conf.inp(&dftt_conf, &x));

    /* Execute any potential windowing */
    dftt_conf.w(&dftt_conf, x);

    /* Set the array sizes to be used in the DFT */
    CHECK_ERR(set_transform_size(&dftt_conf, &X, &x));

    /* Start the timer */
    check_timer_start(&dftt_conf);

    /* DFT */
    dftt_conf.dft(&dftt_conf, X, x);

    /* Check if timer was activated and output time */
    check_timer_end_output(&dftt_conf);

    /* Parses the complex data buffer into real numbers, imaginary numbers, and frequency bins */
    parse_complex_buff_to_RIB(X, &X_RIB, dftt_conf.total_samples);

    /* Prepare the output based on the flags */
    prep_outp(&dftt_conf, X_RIB);

    generate_file_name(dftt_conf.ofile, dftt_conf.ibuff, dftt_conf.input_flag);

    /* Output the DFT array */
    dftt_conf.outp(&dftt_conf, X_RIB);

    fflush(stdout);

    free(x);
    free(X);
    free(X_RIB);

    return 0;
}
