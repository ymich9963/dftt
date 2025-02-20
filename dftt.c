#include "dftt.h"

void set_defaults(dftt_config_t* dftt_conf) {
    strcpy(dftt_conf->ofile, "dftt.txt");
    dftt_conf->dft_bins = 0;
    dftt_conf->channels = 1;
    dftt_conf->tolerance = 10e-7;
    dftt_conf->info_flag = 0;
    dftt_conf->timer_flag = 0;
    dftt_conf->dft = &dft;
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
            CHECK_RET(select_outp(strval, dftt_conf));
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
            CHECK_RET(select_fft_algo(strval, dftt_conf));
            i++;
            continue;
        }

        if (!(strcmp("--dft", argv[i]))) {
            dftt_conf->dft = &dft;
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

int select_fft_algo(char* strval, dftt_config_t* dftt_conf) {
    if (!(strcmp("radix2-dit", strval))) {
        dftt_conf->dft = &fft_radix2_dit;
    } else {
        fprintf(stderr, "\nFFT algorithm not implemented. Exiting...\n\n");
        return 1;
    }
    return 0;
}

int open_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf) {

    *file = sf_open(dftt_conf->ifile, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "\n%s\n", sf_strerror(*file));

        return 1;
    }

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

    dftt_conf->total_samples = sf_info->frames;

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

size_t get_padded_size(size_t* size) 
{
    size_t padded_size;
    for (size_t i = 1; i < 2 * *size; i <<= 2) {

        if (i >= *size) {
            padded_size = i;
        }

    }

    return padded_size;
}

void index_bit_reversal(size_t* n, uint32_t* arr) 
{
    arr[0] = 0;
    arr[1] = 1;

    for (int k = 4; k <= *n; k <<= 1) {

        for (int i = k >> 1; i >= 0; i--) {
            for (int c = 1; c <= i+1; c++) {
                arr[i+c] = arr[i];
            }
        }

        for (int j = 1; j < *n; j += 2) {
            arr[j] += k >> 1;
        }

    }

}

void reorder_data(uint32_t* index_arr, double complex* data_arr, size_t* data_size)
{
    double* copy_arr = malloc(sizeof(double complex) * *data_size);
    for (size_t i = 0; i < *data_size; i++) {
        copy_arr[i] = data_arr[i];
    }
    for (size_t i = 0; i < *data_size; i++) {
        data_arr[i] = copy_arr[index_arr[i]];
    }
}

void convert_to_complex(double* x, double complex* X_complex, size_t* size) {
    for (size_t i = 0; i < *size; i++){
        X_complex[i] = creal(x[i]);
    }
}

double complex get_twiddle_factor(size_t* nk, size_t* N) {
    const double theta = 2 * M_PI * *nk / *N;
    double complex w = cos(theta) - (I * sin(theta)); 

    return w;
}

void dft(double complex** X, double* x, dftt_config_t* dftt_conf) {
    uint64_t n;                 // Sample index in time domain
    uint64_t k;                 // Sample index in frequency domain
    uint64_t N;                 // Total samples
    
    N = dftt_conf->total_samples;
    *X = malloc(sizeof(double complex) * dftt_conf->total_samples);

    printf("\tCalculating DFT...\n");

    /* Calculate DFT */
    for (k = 0; k < N; k++) {
        for (n = 0; n < N; n++) { 
            (*X)[k] += (x[n] * cos(2 * M_PI * n * k / N)) + (I * x[n] * sin(2 * M_PI * n * k / N));
        }
    }
}

void fft_radix2_dit(double complex** X, double* x, dftt_config_t* dftt_conf) {

    printf("\tCalculating DFT using Radix-2 Decimation In Time...\n");

    /* Add padding to the size to make it a power of 2 and reallocate */
    dftt_conf->padded_size = get_padded_size(&dftt_conf->total_samples);

    /* Allocate the required size for the DFT array */
    *X = calloc(dftt_conf->padded_size, sizeof(double complex));

    /* Create an array to convert the mono input to complex values */
    double complex* x_mono_complex_copy = calloc(dftt_conf->padded_size, sizeof(double complex));
    convert_to_complex(x, x_mono_complex_copy, &dftt_conf->total_samples);

    /* Get the array of the bit-reversed indexes */
    uint32_t* index_arr = malloc(sizeof(uint32_t) * dftt_conf->padded_size);
    index_bit_reversal(&dftt_conf->padded_size, index_arr);

    /* Re-order the data based on the bit-reversed indexes */
    reorder_data(index_arr, x_mono_complex_copy, &dftt_conf->padded_size);

    /* Execute butterfly and twiddle factor calculations */
    double complex w, a, b;
    for (size_t N = 2; N <= dftt_conf->padded_size; N *= 2) {

        /* Counter j moves the point of where the N-size sequence starts */
        for (size_t j = 0; j < dftt_conf->padded_size; j += N) {

            for (size_t n = j, nk = 0; n < j + (N/2); n++, nk++) {
                a = x_mono_complex_copy[n];
                b = x_mono_complex_copy[n + (N/2)];
                w = get_twiddle_factor(&nk, &N);

                (*X)[n] = a + b * w;
                (*X)[n + (N/2)] = a - b * w;

                // printf("[%lld] = (%lf+%lfi) + (%lf+%lfi) * (%lf+%lfi)[%lld, %lld]\n", n, creal(a), cimag(a), creal(b), cimag(b), creal(w), cimag(w), nk, N);
                // printf("[%lld] = (%lf+%lfi) - (%lf+%lfi) * (%lf+%lfi)[%lld, %lld]\n", n + N/2, creal(a), cimag(a), creal(b), cimag(b), creal(w), cimag(w), nk, N);
            }

        }

        /* Using the input array to store the data to be used in the next loop */
        for (size_t i = 0; i < dftt_conf->padded_size; i++) {
            x_mono_complex_copy[i] = (*X)[i];
        }

    }

    /* Set the total samples to be the new padded size for outputting later */
    dftt_conf->total_samples = dftt_conf->padded_size;
}

void set_zeros(double complex* X, dftt_config_t* dftt_conf) {
    size_t k;                 // Sample index in frequency domain
    double X_real, X_imag;

    for (k = 0; k < dftt_conf->total_samples; k++) {
        X_real = creal(X[k]);
        check_zero_tolerance(&X_real, dftt_conf);
        X_imag = cimag(X[k]);
        check_zero_tolerance(&X_imag, dftt_conf);
        X[k] = X_real + (I * X_imag);
    }
}

void check_zero_tolerance(double* x, dftt_config_t* dftt_conf) {
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
    } else {
        fprintf(stderr, "\nOutput method not available.\n");

        return 1;
    }
}

int output_file_stdout(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    for (size_t i = 0; i < dftt_conf->total_samples; i++){
        fprintf(stdout, "%lf + j%lf\n", creal(X[i]), cimag(X[i]));
    }

    return 0;
}

int output_file_line(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    for (size_t i = 0; i < dftt_conf->total_samples; i++){
        fprintf(*file, "%lf", creal(X[i]));
        fprintf(*file, cimag(X[i]) >= 0 ? " + %lfi\n" : " - %lfi\n", fabs(cimag(X[i])));
    }

    printf("\tOutputted data to '%s'!\n\n", dftt_conf->ofile);

    return 0;
}

int output_file_csv(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fprintf(*file, "Real, Imag\n");
    for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
        fprintf(*file, "%lf, %lf\n", creal(X[i]), cimag(X[i]));
    }
    fprintf(*file, "%lf, %lf", creal(X[dftt_conf->total_samples]), cimag(X[dftt_conf->total_samples]));

    return 0;
}

int output_file_hex_dump(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "wb");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fwrite(X, sizeof(double complex), dftt_conf->total_samples, *file);

    fprintf(stdout, "Data covers 2 x %llud byte sections.", sizeof(double) * dftt_conf->total_samples);

    return 0;
}

int output_file_c_array(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fprintf(*file, "#define DFT_ARR_SIZE\t%lld\n\n", dftt_conf->total_samples);

    fprintf(*file, "X_real[DFT_ARR_SIZE] = [");
    for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
        fprintf(*file, "%lf, ", creal(X[i]));
    }
    fprintf(*file, "%lf]\n\n", creal(X[dftt_conf->total_samples]));

    fprintf(*file, "X_imag[DFT_ARR_SIZE] = [");
    for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
        fprintf(*file, "%lf, ", cimag(X[i]));
    }
    fprintf(*file, "%lf]\n", cimag(X[dftt_conf->total_samples]));

    return 0;
}

void check_end_timer_output(dftt_config_t* dftt_conf) {
    if (dftt_conf->timer_flag) {
        dftt_conf->end_time = clock() - dftt_conf->start_time;
        printf("\n\tTime taken: %.5f seconds\n\n", (float) dftt_conf->end_time/CLOCKS_PER_SEC);
    }
}

void output_help() {
    printf(
            "\nDiscrete Fourier Transform Tool (DFTT) help page:\n\n"
            "\t-i,\t--input <Audio File>\t\t= Path or name of the input audio file.\n"
            "\t-o,\t--output <File Name>\t\t= Path or name of the output file.\n"
            "\t-f,\t--output-format <Format>\t= Format of the output file. Select between: 'stdout', 'line', 'csv', 'hex-dump', and 'c-array'.\n"
            "\t-N,\t--dft-bins <Number>\t\t= Number of DFT bins.\n"
            "\t-t,\t--tolerance <Number>\t\t= Decimal number that if results are not within its range, they get set to 0.\n"
            "\t\t--fft <Algo>\t\t\t= Use an FFT algorithm to compute the DFT. Selecte between 'radix2-dit'.\n"
            "\t\t--dft\t\t\t\t= Regular DFT calculation using Euler's formula to expand the summation. Default behaviour, included for completion.\n"
            "\t\t--timer\t\t\t\t= Start a timer to see how long the calculation takes.\n"
            "\t\t--info\t\t\t\t= Output to stdout some info about the input file.\n"
            "\n"
          );
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

