#include "dftt.h"

int get_options(int* restrict argc, char** restrict argv, dftt_config_t* restrict dftt_conf) {
    char strval[MAX_STR];

    if (*argc == 1) {
        fprintf(stdout, WELCOME_STR);

        return 1;
    }

    if (*argc == 2) {
        if (!(strcmp("--version", argv[1]))) {
            fprintf(stdout, VERSION_STR);

            return 1;
        }

        if (!(strcmp("--help", argv[1]))) {
            output_help();

            return 1;
        }
    }

    for (int i = 1; i < *argc; i++) {
        if (argv[i][0] != '-' && argv[i - 1][0] != '-') {
            CHECK_RES(sscanf(argv[i], "%s", strval));
            strcpy(dftt_conf->ifile, strval);
            continue;
        }

        if (!(strcmp("-i", argv[i])) || !(strcmp("--input", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(dftt_conf->ifile, strval);
            i++;
            continue;
        }

        if (!(strcmp("-o", argv[i])) || !(strcmp("--output", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(dftt_conf->ofile, strval);
            i++;
            continue;
        }

        if (!(strcmp("--info", argv[i]))) {
            dftt_conf->info_flag = 1;
            continue;
        }
        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n\n", argv[i]);

        return 1;
    }
    return 0;
}

int open_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf) {

    *file = sf_open(dftt_conf->ifile, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "\nSF Error: %s.\n\n", sf_strerror(*file));

        return 1;
    };

    return 0;
}

int read_file(SNDFILE* file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double** x) {
    *x = calloc(sf_info->frames, sizeof(double));
    dftt_conf->sf_count = sf_readf_double(file, *x, sf_info->frames);
    if (dftt_conf->sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", dftt_conf->sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

void dft(double* X_real, double* X_imag, double* Pow, long long* N, double* x) {
    long n;                 // Sample index in time domain
    long k;                 // Sample index in frequency domain

    /* Calculate DFT */
    for (k = 0; k < *N; k++) {
        for (n = 0; n < *N; n++) { 
            X_real[k] += x[n] * cos(2 * M_PI * n * k / *N);
            X_imag[k] -= x[n] * sin(2 * M_PI * n * k / *N);
        }

        /* Calculate the power spectrum of x[n] */
        Pow[k] = (X_real[k] * X_real[k]) + (X_imag[k] * X_imag[k]);
    }
}

char* get_sndfile_major_format(SF_INFO* sf_info) {
    uint16_t format_mask = 0x0000;
    const int major_format = sf_info->format && format_mask;

    char* format;


    return format;
}

char* get_sndfile_subtype(SF_INFO* sf_info) {
    uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format && subtype_mask;

    char* format;


    return format;
}

void output_info(SF_INFO* sf_info, dftt_config_t* dftt_conf) {
    if (dftt_conf->info_flag) {
        fprintf(stdout, "\n\t\t---FILE INFO---\n");
        fprintf(stdout, "\tFile Name: %s\n", dftt_conf->ifile);
        fprintf(stdout, "\tSample Rate: %d\n", sf_info->samplerate);
        fprintf(stdout, "\tSamples: %lld\n", sf_info->frames);
        fprintf(stdout, "\tChannels: %d\n", sf_info->channels);
        fprintf(stdout, "\tFormat: 0x%x\n", sf_info->format);
        fprintf(stdout, "\t\t---------------\n");
    }
}

int output_file_double(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    for (long i = 0; i < sf_info->frames; i++){
        fprintf(*file, "%lf\n", data[i]);
    }

    return 0;
}

void output_help() {

}
