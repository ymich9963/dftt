#include "dftt.h"

//FIX: FFT output
//FIX: How tf do I implement different bins. Made the option take the data but implementation is wrong.
//TODO: Add text file and string input
//TODO: Add output input data flag
//TODO: Create a tool to extract the data from an audio file (adx - audio data extractor)

int main (int argc, char** argv) {
    SNDFILE* file;          // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info
    FILE * ofile;           // Output file pointer
    FILE * oifile;          // Input file data output pointer
    double* x;              // Data from audio file, considered the input signal
    double* x_mono;         // Store the audio file data converted to single-channel
    double complex* X;      // Fourier Transform result
    dftt_config_t dftt_conf; // Tool config

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Set defaults to ensure certain behabiour */
    set_defaults(&dftt_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    /* Initialise input file buffer and open the input file */
    file = NULL;          // Pointer to file
    CHECK_ERR(open_file(&file, &sf_info, &dftt_conf));

    /* Output info on the inputted file */
    output_info(&sf_info, &dftt_conf);

    /* Initialise input data array and read the input audio file */
    x = NULL; 
    CHECK_ERR(read_file_data(file, &sf_info, &dftt_conf, &x));

    /* Translate the data to one channel (mono) */
    x_mono = NULL;
    mix2mono(&sf_info, x, &x_mono);

    /* Start the timer */
    check_start_timer(&dftt_conf);

    /* Initialise DFT result array */
    X = NULL;

    /* DFT */
    dftt_conf.dft(&X, x_mono, &dftt_conf);

    /* Set the zeros based on the tolerance */
    set_zeros(X, &dftt_conf);

    /* Initialise outuput buffer and output the DFT array */
    ofile = NULL;
    dftt_conf.outp(&ofile, &dftt_conf, X);

    /* Check if timer was activated and output time if yes */
    check_end_timer_output(&dftt_conf);

    sf_close(file);
    fclose(ofile);
    free(x);
    free(x_mono);
    free(X);

    return 0;
}
