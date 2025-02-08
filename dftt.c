#include "dftt.h"

void set_defaults(dftt_config_t* dftt_conf) {
    strcpy(dftt_conf->ofile, "dftt.txt");
    dftt_conf->dft_bins = 0;
    dftt_conf->channels = 1;
    dftt_conf->tolerance = 10e-7;
    dftt_conf->fft_flag = 0;
    dftt_conf->timer_flag = 0;
    dftt_conf->outp = &output_file_line;
}

int get_options(int* restrict argc, char** restrict argv, dftt_config_t* restrict dftt_conf) {
    char strval[MAX_STR];
    long lval = 0;
    double lfval = 0.0;

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

        if (!(strcmp("-f", argv[i])) || !(strcmp("--output-format", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            select_outp(strval, dftt_conf);
            i++;
            continue;
        }

        if (!(strcmp("-N", argv[i])) || !(strcmp("--dft-bins", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%ld", &lval));
            dftt_conf->dft_bins = lval; 
            i++;
            continue;
        }

        if (!(strcmp("-t", argv[i])) || !(strcmp("--tolerance", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%lf", &lfval));
            dftt_conf->tolerance = lfval; 
            i++;
            continue;
        }

        if (!(strcmp("--fft", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            select_fft_algo(strval, dftt_conf);
            i++;
            continue;
        }

        if (!(strcmp("--timer", argv[i]))) {
            dftt_conf->timer_flag = 1;
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
        fprintf(stderr, "\nSNDFile Error: %s.\n\n", sf_strerror(*file));

        return 1;
    };
    return 0;
}

int read_file_data(SNDFILE* file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double** x) {
    size_t data_size = sf_info->frames * sf_info->channels;
    *x = calloc(data_size, sizeof(double));
    dftt_conf->sf_count = sf_readf_double(file, *x, data_size);
    if (dftt_conf->sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", dftt_conf->sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int mix2mono(SF_INFO* sf_info, double* x, double** x_mono) {

    *x_mono = calloc(sf_info->frames, sizeof(double));
    uint64_t i = 0;
    uint16_t c = 0;
    for (i = 0; i < sf_info->frames; i++) {
        for (c = 0; c < sf_info->channels; c++) {
            (*x_mono)[i] += (x[sf_info->channels * i + c]/sf_info->channels);
        }
    }

    return 0;
}

void check_start_timer(dftt_config_t* dftt_conf) {
    if (dftt_conf->timer_flag) {
        dftt_conf->start_time = clock();
        printf("\tStarted timer!\n");
    }
}

void dft(double _Complex* X, long long* N, double* x) {
    uint64_t n;                 // Sample index in time domain
    uint64_t k;                 // Sample index in frequency domain

    /* Calculate DFT */
    for (k = 0; k < *N; k++) {
        for (n = 0; n < *N; n++) { 
            X[k] += (x[n] * cos(2 * M_PI * n * k / *N)) + (I * x[n] * sin(2 * M_PI * n * k / *N));
        }
    }
}

void fft_radix2_dit() {

}

void set_zeros(double _Complex* X, long long* N, dftt_config_t* dftt_conf) {
    uint64_t k;                 // Sample index in frequency domain
    double X_real, X_imag;

    /* Calculate DFT */
    for (k = 0; k < *N; k++) {
        X_real = creal(X[k]);
        check_zero_tolerance(&X_real, N, dftt_conf);
        X_imag = cimag(X[k]);
        check_zero_tolerance(&X_imag, N, dftt_conf);
        X[k] = X_real + (I * X_imag);
    }
}

void check_zero_tolerance(double* x, long long* N, dftt_config_t* dftt_conf) {
    if (*x < dftt_conf->tolerance && *x > -dftt_conf->tolerance) {
        *x = 0;
    }
}

char* get_sndfile_major_format(SF_INFO* sf_info) {
    const uint32_t format_mask = 0x00FF0000;
    const uint32_t major_format = sf_info->format & format_mask;

    for (int i = 0; i < SND_MAJOR_FORMAT_NUM; i++) {
        if (major_format == snd_format_arr[i]) {
            return snd_format_arr_desc[i];
        }
    }

    return "N/A";
}

char* get_sndfile_subtype(SF_INFO* sf_info) {
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    for (int i = 0; i < SND_SUBTYPE_NUM; i++) {
        if (subtype == snd_subtype_arr[i]) {
            return snd_subtype_arr_desc[i];
        }
    }

    return "N/A";
}

void output_info(SF_INFO* sf_info, dftt_config_t* dftt_conf) {
    if (dftt_conf->info_flag) {
        fprintf(stdout, "\n\t\t---FILE INFO---\n");
        fprintf(stdout, "\tFile Name: %s\n", dftt_conf->ifile);
        fprintf(stdout, "\tSample Rate: %d\n", sf_info->samplerate);
        fprintf(stdout, "\tSamples: %lld\n", sf_info->frames);
        fprintf(stdout, "\tChannels: %d\n", sf_info->channels);
        fprintf(stdout, "\tFormat: %s\n", get_sndfile_major_format(sf_info));
        fprintf(stdout, "\tSubtype: %s\n", get_sndfile_subtype(sf_info));
        fprintf(stdout, "\t\t---------------\n\n");
    }
}

int select_outp(char* strval, dftt_config_t* dftt_conf) {
    if(!(strcmp("stdout", strval))) {
        dftt_conf->outp = &output_file_stdout; 
        return 0;
    }
    if(!(strcmp("line", strval))) {
        dftt_conf->outp = &output_file_line; 
        return 0;
    }
    if(!(strcmp("csv", strval))) {
        dftt_conf->outp = &output_file_csv; 
        return 0;
    }
    if(!(strcmp("hex-dump", strval))) {
        dftt_conf->outp = &output_file_hex_dump; 
        return 0;
    }
    if(!(strcmp("c-array", strval))) {
        dftt_conf->outp = &output_file_c_array; 
        return 0;
    }

    fprintf(stderr, "\nOutput method not available.\n");
    return 1;
}

int select_fft_algo(char* strval, dftt_config_t* dftt_conf) {
    if (!(strcmp("radix2-dit", strval))) {
        dftt_conf->fft = &fft_radix2_dit;
    }
}

int output_file_stdout(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X) {

    for (uint32_t i = 0; i < sf_info->frames; i++){
        fprintf(stdout, "%lf + j%lf\n", creal(X[i]), cimag(X[i]));
    }

    return 0;
}

int output_file_line(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    for (long i = 0; i < sf_info->frames; i++){
        fprintf(*file, "%lf + j%lf\n", creal(X[i]), cimag(X[i]));
    }

    return 0;
}

int output_file_csv(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fprintf(*file, "Real, Imag\n");
    for (long i = 0; i < (sf_info->frames) - 1; i++){
        fprintf(*file, "%lf, %lf\n", creal(X[i]), cimag(X[i]));
    }
    fprintf(*file, "%lf, %lf", creal(X[sf_info->frames]), cimag(X[sf_info->frames]));

    return 0;
}

int output_file_hex_dump(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X) {

    *file = fopen(dftt_conf->ofile, "wb");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    /* Write an extract function  */
    /* fwrite(X_real, sizeof(double), sf_info->frames, *file); */
    /* fwrite(X_imag, sizeof(double), sf_info->frames, *file); */

    fprintf(stdout, "Data covers 2 x %llud byte sections.", sizeof(double) * sf_info->frames);

    return 0;
}

int output_file_c_array(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fprintf(*file, "#define DFT_ARR_SIZE\t%lld\n\n", sf_info->frames);

    fprintf(*file, "X_real[DFT_ARR_SIZE] = [");
    for (long i = 0; i < (sf_info->frames) - 1; i++){
        fprintf(*file, "%lf, ", creal(X[i]));
    }
    fprintf(*file, "%lf]\n\n", creal(X[sf_info->frames]));

    fprintf(*file, "X_imag[DFT_ARR_SIZE] = [");
    for (long i = 0; i < (sf_info->frames) - 1; i++){
        fprintf(*file, "%lf, ", cimag(X[i]));
    }
    fprintf(*file, "%lf]\n", cimag(X[sf_info->frames]));

    return 0;
}


void check_end_timer_output(dftt_config_t* dftt_conf) {
    if (dftt_conf->timer_flag) {
        dftt_conf->end_time = clock() - dftt_conf->start_time;
        printf("\n\tTime taken: %ld seconds\n\n", dftt_conf->end_time/CLOCKS_PER_SEC);
    }
}

void output_help() {

}

uint32_t snd_format_arr[SND_MAJOR_FORMAT_NUM] = {
    SF_FORMAT_WAV,
    SF_FORMAT_AIFF,
    SF_FORMAT_AU,
    SF_FORMAT_RAW,
    SF_FORMAT_PAF,
    SF_FORMAT_SVX,
    SF_FORMAT_NIST,
    SF_FORMAT_VOC,
    SF_FORMAT_IRCAM,
    SF_FORMAT_W64,
    SF_FORMAT_MAT4,
    SF_FORMAT_MAT5,
    SF_FORMAT_PVF,
    SF_FORMAT_XI,
    SF_FORMAT_HTK,
    SF_FORMAT_SDS,
    SF_FORMAT_AVR,
    SF_FORMAT_WAVEX,
    SF_FORMAT_SD2,
    SF_FORMAT_FLAC,
    SF_FORMAT_CAF,
    SF_FORMAT_WVE,
    SF_FORMAT_OGG,
    SF_FORMAT_MPC2K,
    SF_FORMAT_RF64,
    SF_FORMAT_MPEG,
};

char* snd_format_arr_desc[SND_MAJOR_FORMAT_NUM] = {
    "Microsoft WAV format (little endian)",
    "Apple/SGI AIFF format (big endian)",
    "Sun/NeXT AU format (big endian)",
    "RAW PCM data",
    "Ensoniq PARIS file format",
    "Amiga IFF / SVX8 / SV16 format",
    "Sphere NIST format",
    "VOC files",
    "Berkeley/IRCAM/CARL",
    "Sonic Foundry’s 64 bit RIFF/WAV",
    "Matlab (tm) V4.2 / GNU Octave 2.0",
    "Matlab (tm) V5.0 / GNU Octave 2.1",
    "Portable Voice Format",
    "Fasttracker 2 Extended Instrument",
    "HMM Tool Kit format",
    "Midi Sample Dump Standard",
    "Audio Visual Research",
    "MS WAVE with WAVEFORMATEX",
    "Sound Designer 2",
    "FLAC lossless file format",
    "Core Audio File format",
    "Psion WVE format",
    "Xiph OGG container",
    "Akai MPC 2000 sampler",
    "RF64 WAV file",
    "MPEG-1/2 audio stream",
};

uint32_t snd_subtype_arr[SND_SUBTYPE_NUM] = {
    SF_FORMAT_PCM_S8,
    SF_FORMAT_PCM_16,
    SF_FORMAT_PCM_24,
    SF_FORMAT_PCM_32,
    SF_FORMAT_PCM_U8,
    SF_FORMAT_FLOAT,
    SF_FORMAT_DOUBLE,
    SF_FORMAT_ULAW,
    SF_FORMAT_ALAW,
    SF_FORMAT_IMA_ADPCM,
    SF_FORMAT_MS_ADPCM,
    SF_FORMAT_GSM610,
    SF_FORMAT_VOX_ADPCM,
    SF_FORMAT_NMS_ADPCM_16,
    SF_FORMAT_NMS_ADPCM_24,
    SF_FORMAT_NMS_ADPCM_32,
    SF_FORMAT_G721_32,
    SF_FORMAT_G723_24,
    SF_FORMAT_G723_40,
    SF_FORMAT_DWVW_12,
    SF_FORMAT_DWVW_16,
    SF_FORMAT_DWVW_24,
    SF_FORMAT_DWVW_N,
    SF_FORMAT_DPCM_8,
    SF_FORMAT_DPCM_16,
    SF_FORMAT_VORBIS,
    SF_FORMAT_OPUS,
    SF_FORMAT_ALAC_16,
    SF_FORMAT_ALAC_20,
    SF_FORMAT_ALAC_24,
    SF_FORMAT_ALAC_32,
    SF_FORMAT_MPEG_LAYER_I,
    SF_FORMAT_MPEG_LAYER_II,
    SF_FORMAT_MPEG_LAYER_III,
};

char* snd_subtype_arr_desc[SND_SUBTYPE_NUM] = {
    "Signed 8 bit data",
    "Signed 16 bit data",
    "Signed 24 bit data",
    "Signed 32 bit data",
    "Unsigned 8 bit data (WAV and RAW only)",
    "32 bit float data",
    "64 bit float data",
    "U-Law encoded.",
    "A-Law encoded.",
    "IMA ADPCM.",
    "Microsoft ADPCM.",
    "GSM 6.10 encoding.",
    "OKI / Dialogix ADPCM",
    "16kbs NMS G721-variant encoding.",
    "24kbs NMS G721-variant encoding.",
    "32kbs NMS G721-variant encoding.",
    "32kbs G721 ADPCM encoding.",
    "24kbs G723 ADPCM encoding.",
    "40kbs G723 ADPCM encoding.",
    "12 bit Delta Width Variable Word encoding.",
    "16 bit Delta Width Variable Word encoding.",
    "24 bit Delta Width Variable Word encoding.",
    "N bit Delta Width Variable Word encoding.",
    "8 bit differential PCM (XI only)",
    "16 bit differential PCM (XI only)",
    "Xiph Vorbis encoding.",
    "Xiph/Skype Opus encoding.",
    "Apple Lossless Audio Codec (16 bit).",
    "Apple Lossless Audio Codec (20 bit).",
    "Apple Lossless Audio Codec (24 bit).",
    "Apple Lossless Audio Codec (32 bit).",
    "MPEG-1 Audio Layer I.",
    "MPEG-1 Audio Layer II.",
    "MPEG-2 Audio Layer III.",
};

