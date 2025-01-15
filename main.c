#include "dftt.h"

//TODO: Open a file using the CLI
//TODO: Set up output method - file (comma separated, value on each line, hex dump?) or stdout
//TODO: Info flag which outputs as much info about the file as possible

int main (int argc, char** argv) {
    SF_INFO sf_info;        // File info
    sf_count_t sf_count;    // To count number of data read
    double* X_real;         // DFT of the Real part
    double* X_imag;         // DFT of the Imaginary part
    double* Pow;            // Power spectrum

    dftt_config_t dftt_conf; // Tool config

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Might be necessary?   */
    /* sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT; */

    CHECK_ERR(get_options(&argc, argv, &dftt_conf));

    SNDFILE* file = NULL;          // Pointer to file
    CHECK_ERR(open_file(&file, &sf_info, &dftt_conf));

    double* x = NULL;              // Data from audio file, considered the input signal
    CHECK_ERR(read_file(file, &sf_info, &dftt_conf, &x));

    printf("\n\tSuccesfuly read file and data!\n");

    printf("\n");
    printf("samplerate: %d\n", sf_info.samplerate);
    printf("frames: %lld\n", sf_info.frames);
    printf("channels: %d\n", sf_info.channels);
    printf("sf count: %lld\n", sf_count);
    printf("data: %lf\n", x[100]);
    printf("\n");

    X_real = calloc(sf_info.frames, sizeof(double));
    X_imag = calloc(sf_info.frames, sizeof(double));
    Pow = calloc(sf_info.frames, sizeof(double));
    
    printf("\nWorking on it...\n");
    dft(X_real, X_imag, Pow, &sf_info.frames, x);

    FILE * ofile = fopen("out.txt", "w");
    
    for (long c = 0; c < sf_info.frames; c++){
        fprintf(ofile, "%lf\n", Pow[c]);
    }

    sf_close(file);
    fclose(ofile);
    free(x);
    free(X_real);
    free(X_imag);
    free(Pow);
    return 0;
}
