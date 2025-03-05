#include "dftt.h"

void set_defaults(dftt_config_t* dftt_conf) {
    strcpy(dftt_conf->ofile, "dftt.txt");
    dftt_conf->total_samples = 0;
    dftt_conf->detected_samples = 0;
    dftt_conf->channels = 1;
    dftt_conf->precision = 6;
    dftt_conf->sampling_freq = 0;
    dftt_conf->info_flag = 0;
    dftt_conf->fft_flag = 0;
    dftt_conf->timer_flag = 0;
    dftt_conf->quiet_flag = 0;
    dftt_conf->pow_flag = 0;
    dftt_conf->inp = &read_audio_file_input;
    dftt_conf->dft = &dft;
    dftt_conf->outp = &output_file_stdout;
}

int get_options(int* restrict argc, char** restrict argv, dftt_config_t* restrict dftt_conf) {
    char strval[MAX_STR];
    long lval = 0;
    int dval = 0;

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
            strcpy(dftt_conf->ibuff, strval);
            dftt_conf->inp = &read_audio_file_input;
            continue;
        }

        if (!(strcmp("-i", argv[i])) || !(strcmp("--input", argv[i])) || !(strcmp("--input-audio", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(dftt_conf->ibuff, strval);
            dftt_conf->inp = &read_audio_file_input;
            i++;
            continue;
        }

        if (!(strcmp("--input-csv", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(dftt_conf->ibuff, strval);
            dftt_conf->inp = &read_csv_string_file_input;
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

        if (!(strcmp("-N", argv[i])) || !(strcmp("--total-samples", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%ld", &lval));
            dftt_conf->total_samples = lval; 
            i++;
            continue;
        }
        
        if (!(strcmp("-s", argv[i])) || !(strcmp("--sampling-frequency", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%ld", &lval));
            dftt_conf->sampling_freq = lval; 
            i++;
            continue;
        }

        if (!(strcmp("-p", argv[i])) || !(strcmp("--precision", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%d", &dval));
            dftt_conf->precision = dval; 
            i++;
            continue;
        }

        if (!(strcmp("--fft", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            CHECK_RET(select_fft_algo(strval, dftt_conf));
            dftt_conf->fft_flag = 1;
            i++;
            continue;
        }

        if (!(strcmp("--dft", argv[i]))) {
            dftt_conf->dft = &dft;
            dftt_conf->fft_flag = 0;
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

        if (!(strcmp("-q", argv[i])) || !(strcmp("--quiet", argv[i]))) {
            dftt_conf->quiet_flag = 1; 
            continue;
        }

        if (!(strcmp("-b", argv[i])) || !(strcmp("--show-bins", argv[i]))) {
            dftt_conf->quiet_flag = 1; 
            continue;
        }

        if (!(strcmp("--pow", argv[i])) || !(strcmp("--power-spectrum", argv[i]))) {
            dftt_conf->pow_flag = 1; 
            continue;
        }

        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n\n", argv[i]);

        return 1;
    }

    return 0;
}

int read_audio_file_input(double** x, dftt_config_t* dftt_conf) {
    SNDFILE* file;          // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info
    double* file_data;         // Input data from file

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Initialise input file buffer and open he input file */
    file = NULL;
    CHECK_ERR(open_audio_file(&file, &sf_info, dftt_conf));

    /* Initialise input data array and read the input audio file */
    file_data = NULL; 
    CHECK_ERR(read_audio_file_data(file, &sf_info, &file_data));

    /* Translate the data to one channel (mono) */
    mix2mono(&sf_info, file_data, x);

    /* Output info on the inputted file */
    output_audio_file_info(&sf_info, dftt_conf);

    dftt_conf->detected_samples = sf_info.frames;

    return 0;
}

int read_csv_string_file_input(double** x, dftt_config_t* dftt_conf) {
    FILE* file;          // Pointer to the input audio file
    char* data_string;   // Input data from file

    /* Initialise */
    file = NULL;
    data_string = NULL; 

    /* Try to open and read input file, if not then it's considered a data string */
    if (!(open_csv_file(&file, dftt_conf))) {
        CHECK_ERR(read_csv_file_data(file, dftt_conf, &data_string));
        dftt_conf->input_flag = 'f';
    } else {
        data_string = malloc(sizeof(char) * strlen(dftt_conf->ibuff)); 
        strcpy(data_string, dftt_conf->ibuff);
        dftt_conf->input_flag = 's';
    }

    get_data_from_string(data_string, x, dftt_conf);

    /* Output info on the inputted file */
    output_csv_file_string_info(dftt_conf);

    free(data_string);
    return 0;
}
int open_audio_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf) {

    *file = sf_open(dftt_conf->ibuff, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "\n%s\n", sf_strerror(*file));

        return 1;
    }

    return 0;
}

int open_csv_file(FILE** file, dftt_config_t* dftt_conf) {

    *file = fopen(dftt_conf->ibuff, "r");
    if(!(*file)) {

        return 1;
    }

    return 0;
}

int get_data_from_string(char* data_string, double** x, dftt_config_t* dftt_conf) {
    /* Make a copy of the string to get the number of data points  */
    char* data_string_copy;
    data_string_copy = malloc(sizeof(char) * strlen(data_string)); 
    strcpy(data_string_copy, data_string);

    /* Get the number of data points */
    size_t samples = 0;
    char* token = strtok(data_string_copy, ",");
    while (token != NULL) {
        samples++;
        token = strtok(NULL, ",");
    }
    free(data_string_copy);

    /* Allocate the size of the array based on the data points */
    *x = calloc(samples, sizeof(double));

    /* Retrieve the data */
    size_t i = 0;
    double lfval = 0.0f;
    token = strtok(data_string, ",");
    while (token != NULL && i < samples) {
        sscanf(token, "%lf", &lfval);
        (*x)[i] = lfval;
        token = strtok(NULL, ",");
        i++;
    }

    dftt_conf->detected_samples = samples;

    return 0;
}

int read_audio_file_data(SNDFILE* file, SF_INFO* sf_info, double** x) {
    /* Get audio file data size */
    size_t data_size = sf_info->frames * sf_info->channels;
    *x = calloc(data_size, sizeof(double));

    /* Read data and place into buffer */
    sf_count_t sf_count = sf_readf_double(file, *x, data_size);

    /* Check */
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int read_csv_file_data(FILE* file, dftt_config_t* dftt_conf, char** data_string) {
    /* Go to the end of the file to find the size needed to store the data */
    fseek(file, 0, SEEK_END);
    size_t pos = ftell(file);

    /* Put the file position indicator back to the start */
    fseek(file, 0, SEEK_SET);

    /* Allocate the file size */
    *data_string = malloc(sizeof(char) * pos);

    /* Read the file data and place into string variable */
    fread(*data_string, sizeof(char), pos, file);

    return 0;
}

int mix2mono(SF_INFO* sf_info, double* x, double** x_mono) {

    uint64_t i = 0;
    uint16_t c = 0;

    *x_mono = calloc(sf_info->frames, sizeof(double));

    for (i = 0; i < sf_info->frames; i++) {
        for (c = 0; c < sf_info->channels; c++) {
            (*x_mono)[i] += (x[sf_info->channels * i + c]/sf_info->channels);
        }
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

void check_start_timer(dftt_config_t* dftt_conf) {
    if (dftt_conf->timer_flag && !dftt_conf->quiet_flag) {
        dftt_conf->start_time = clock();
        printf("Started timer.\n");
    }
}

void expand_array(double** arr, size_t* new_size, size_t* old_size) {
    *arr = realloc(*arr, *new_size * sizeof(double));
    if (*arr == NULL) {
        fprintf(stderr, "\nUnable to reallocate input array.\n");
    }

    for (size_t i = *old_size; i < *new_size; i++) {
        (*arr)[i] = 0.0f;
    }

}

void truncate_array(double** arr, size_t* new_size) {
    *arr = realloc(*arr, *new_size * sizeof(double));
    if (*arr == NULL) {
        fprintf(stderr, "\nUnable to reallocate input array.\n");
    }
}

void set_transform_size(dftt_config_t* dftt_conf, double complex** X, double** x) {

    if (!dftt_conf->total_samples) {
        dftt_conf->total_samples = dftt_conf->detected_samples;
    }

    /* If using an FFT algo, round up size to next power of 2 */
    if (dftt_conf->fft_flag) {
        set_adjusted_size(&dftt_conf->total_samples);
        if (!dftt_conf->quiet_flag && dftt_conf->detected_samples != dftt_conf->total_samples) {
            printf("Adjusted input data size to %lld.\n", dftt_conf->total_samples);
        }
    }

    /* Allocate DFT array */
    *X = calloc(dftt_conf->total_samples, sizeof(double complex));
    if (*X == NULL) {
        fprintf(stderr, "\nUnable to reallocate DFT array.\n");
    }

    /* Check if the input array needs expanding/truncating */
    if (dftt_conf->detected_samples < dftt_conf->total_samples) {
        expand_array(x, &dftt_conf->total_samples, &dftt_conf->detected_samples);
    }
    if (dftt_conf->detected_samples > dftt_conf->total_samples) {
        truncate_array(x, &dftt_conf->total_samples);
    }
}

void set_adjusted_size(size_t* size) 
{
    for (size_t i = 1; i < *size << 1; i <<= 1) {

        if (i >= *size) {
            *size = i;
        }

    }
}

void index_bit_reversal(size_t* n, size_t* arr) 
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

void reorder_data(size_t* index_arr, double* data_arr, size_t* data_size)
{
    double* copy_arr = calloc(*data_size, sizeof(double));
    if (copy_arr == NULL) {
        fprintf(stderr, "\nUnable to reallocate DFT array.\n");
    }

    for (size_t i = 0; i < *data_size; i++) {
        copy_arr[i] = data_arr[i];
    }
    for (size_t i = 0; i < *data_size; i++) {
        data_arr[i] = copy_arr[index_arr[i]];
    }
    free(copy_arr);
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

void dft(double complex* X, double* x, dftt_config_t* dftt_conf) {
    uint64_t n;                 // Sample index in time domain
    uint64_t k;                 // Sample index in frequency domain
    uint64_t N;                 // Total samples

    N = dftt_conf->total_samples;

    if (!dftt_conf->quiet_flag) {
        printf("Calculating DFT.\n");
    }

    /* Calculate DFT */
    for (k = 0; k < N; k++) {
        for (n = 0; n < N; n++) { 
            X[k] += (x[n] * cos(2 * M_PI * n * k / N)) + (I * x[n] * sin(2 * M_PI * n * k / N));
        }
    }

    if (!dftt_conf->quiet_flag) {
        printf("Finished.\n");
    }
}

void fft_radix2_dit(double complex* X, double* x, dftt_config_t* dftt_conf) {

    if (!dftt_conf->quiet_flag) {
        printf("Calculating DFT using Radix-2 Decimation In Time.\n");
    }

    /* Get the array of the bit-reversed indexes */
    size_t* index_arr = calloc(dftt_conf->total_samples, sizeof(size_t));
    index_bit_reversal(&dftt_conf->total_samples, index_arr);

    /* Re-order the data based on the bit-reversed indexes */
    reorder_data(index_arr, x, &dftt_conf->total_samples);

    /* Create an array to convert the mono input to complex values */
    double complex* x_mono_complex_copy = calloc(dftt_conf->total_samples, sizeof(double complex));
    convert_to_complex(x, x_mono_complex_copy, &dftt_conf->total_samples);

    /* Execute butterfly and twiddle factor calculations */
    size_t k = dftt_conf->total_samples;
    double complex w, a, b;
    for (size_t N = 2; N <= k; N *= 2) {

        /* Counter j moves the point of where the N-size sequence starts */
        for (size_t j = 0; j < k; j += N) {

            for (size_t n = j, nk = 0; n < j + (N/2); n++, nk++) {
                a = x_mono_complex_copy[n];
                b = x_mono_complex_copy[n + (N/2)];
                w = get_twiddle_factor(&nk, &N);

                X[n] = a + b * w;
                X[n + (N/2)] = a - b * w;
            }

        }

        /* Using the input array to store the data to be used in the next loop */
        for (size_t i = 0; i < k; i++) {
            x_mono_complex_copy[i] = X[i];
        }

    }

    if (!dftt_conf->quiet_flag) {
        printf("Finished.\n");
    }

    free(index_arr);
    free(x_mono_complex_copy);
}

void pow_spec(double complex* X, dftt_config_t* dftt_conf) {
    if (dftt_conf->pow_flag) {
        for (size_t i = 0; i < dftt_conf->total_samples; i++) {
            X[i] = (cabs(X[i]) * cabs(X[i]))/dftt_conf->total_samples;
        }

        if (!dftt_conf->quiet_flag) {
            printf("Calculated power spectrum.\n");
        }
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

void output_audio_file_info(SF_INFO* sf_info, dftt_config_t* dftt_conf) {
    if (dftt_conf->info_flag && !dftt_conf->quiet_flag) {
        fprintf(stdout, "\n--INFO--\n");
        fprintf(stdout, "File Name: %s\n", dftt_conf->ibuff);
        fprintf(stdout, "Sample Rate: %d\n", sf_info->samplerate);
        fprintf(stdout, "Samples: %lld\n", sf_info->frames);
        fprintf(stdout, "Channels: %d\n", sf_info->channels);
        fprintf(stdout, "Format: %s\n", get_sndfile_major_format(sf_info));
        fprintf(stdout, "Subtype: %s\n", get_sndfile_subtype(sf_info));
        fprintf(stdout, "---\n\n");
    }
}

void output_csv_file_string_info(dftt_config_t* dftt_conf) {
    if (dftt_conf->info_flag && !dftt_conf->quiet_flag) {
        fprintf(stdout, "\n--INFO--\n");
        fprintf(stdout, dftt_conf->input_flag == 'f' ? "File Name: %s\n" : "Input String: %s\n", dftt_conf->ibuff);
        fprintf(stdout, "Samples: %lld\n", dftt_conf->total_samples);
        fprintf(stdout, dftt_conf->input_flag == 'f' ? "Format: CSV File\n" : "Format: CSV String\n");
        fprintf(stdout, "--------\n\n");
    }
}

int select_outp(char* strval, dftt_config_t* dftt_conf) {
    if(!(strcmp("stdout", strval))) {
        dftt_conf->outp = &output_file_stdout; 

        return 0;
    }
    if(!(strcmp("txt-line", strval))) {
        dftt_conf->outp = &output_file_txt_line; 

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

void show_freq_bin(dftt_config_t* dftt_conf, size_t* i, char separator[4]) {
    if (dftt_conf->sampling_freq) {
        size_t bin = *i * dftt_conf->sampling_freq / dftt_conf->total_samples;
        printf("%lld", bin);
        printf("%s", separator);
    }
}

int output_file_stdout(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = stdout;
    if (!dftt_conf->quiet_flag) {
        fprintf(*file, "\n");
    }

    char format[9];
    sprintf(format, "%%.%dlf", dftt_conf->precision);

    if (dftt_conf->pow_flag) {

        for (size_t i = 0; i < dftt_conf->total_samples; i++) {
            show_freq_bin(dftt_conf, &i, " | ");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, "\n");
        }

    } else {

        for (size_t i = 0; i < dftt_conf->total_samples; i++) {
            show_freq_bin(dftt_conf, &i, " | ");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, cimag(X[i]) >= 0 ? " + " : " - ");
            fprintf(*file, "j");
            fprintf(*file, format, fabs(cimag(X[i])));
            fprintf(*file, "\n");
        }

    }

    return 0;
}

int output_file_txt_line(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    char format[9];
    sprintf(format, "%%.%dlf", dftt_conf->precision);

    if (dftt_conf->pow_flag) {
        for (size_t i = 0; i < dftt_conf->total_samples; i++){
            show_freq_bin(dftt_conf, &i, " | ");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, "\n");
        }
    } else {
        for (size_t i = 0; i < dftt_conf->total_samples; i++){
            show_freq_bin(dftt_conf, &i, " | ");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, cimag(X[i]) >= 0 ? " + " : " - ");
            fprintf(*file, "j");
            fprintf(*file, format, fabs(cimag(X[i])));
            fprintf(*file, "\n");
        }
    }

    if (!dftt_conf->quiet_flag) {
        printf("Outputted data to '%s'.\n", dftt_conf->ofile);
    }

    return 0;
}

int output_file_csv(FILE** file, dftt_config_t* dftt_conf, double complex* X) {

    *file = fopen(dftt_conf->ofile, "w");
    if(!(*file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    char format[9];
    sprintf(format, "%%.%dlf", dftt_conf->precision);

    if (dftt_conf->pow_flag) {
        fprintf(*file, dftt_conf->sampling_freq > 0 ? "Bin,Pow" : "Pow\n");
        for (size_t i = 0; i < dftt_conf->total_samples; i++){
            show_freq_bin(dftt_conf, &i, ",");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, "\n");
        }
    } else {
        fprintf(*file, dftt_conf->sampling_freq > 0 ? "Bin,Real,Imag" :"Real, Imag\n");
        for (size_t i = 0; i < dftt_conf->total_samples; i++){
            show_freq_bin(dftt_conf, &i, ",");
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, ",");
            fprintf(*file, format, cimag(X[i]));
            fprintf(*file, "\n");
        }
    }
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

    char format[9];
    sprintf(format, "%%.%dlf", dftt_conf->precision);

    fprintf(*file, "#define DFT_ARR_SIZE\t%lld\n\n", dftt_conf->total_samples);

    if (dftt_conf->pow_flag) {
        fprintf(*file, "X_Pow[DFT_ARR_SIZE] = [");

        for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, ", ");
        }

        fprintf(*file, format, creal(X[dftt_conf->total_samples]));
        fprintf(*file, "]\n\n");
    } else {
        fprintf(*file, "X_real[DFT_ARR_SIZE] = [");

        for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
            fprintf(*file, format, creal(X[i]));
            fprintf(*file, ", ");
        }

        fprintf(*file, format, creal(X[dftt_conf->total_samples]));
        fprintf(*file, "]\n\n");

        fprintf(*file, "X_imag[DFT_ARR_SIZE] = [");

        for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
            fprintf(*file, format, cimag(X[i]));
            fprintf(*file, ", ");
        }

        fprintf(*file, format, cimag(X[dftt_conf->total_samples]));
        fprintf(*file, "]\n\n");
    }

    return 0;
}

void check_end_timer_output(dftt_config_t* dftt_conf) {
    if (dftt_conf->timer_flag && !dftt_conf->quiet_flag) {
        dftt_conf->end_time = clock() - dftt_conf->start_time;
        printf("Time taken: %.5f seconds\n", (float) dftt_conf->end_time/CLOCKS_PER_SEC);
    }
}

void output_help() {
    printf(
            "\nDiscrete Fourier Transform Tool (DFTT) help page:\n\n"
            "\t-i,\t--input <Audio File>\t\t= Path or name of the input audio file.\n"
            "\t\t--input-audio\n"
            "\t\t--input-csv <CSV File/String>\t= Path or name of the input csv file, or the input string. Must be separated by comma. Example input '1,0,0,1' or 'input.csv' containing '1,0,0,1'.\n"
            "\t-o,\t--output <File Name>\t\t= Path or name of the output file.\n"
            "\t-f,\t--output-format <Format>\t= Format of the output file. Select between: 'stdout', 'txt-line', 'csv', 'hex-dump', and 'c-array'.\n"
            "\t-N,\t--total-samples <Number>\t\t= Set total number of samples to use when calculating. If using the FFT, it rounds up to the next power of 2 samples, zero-padding the signal if necessary.\n"
            "\t-p,\t--precision <Number>\t\t= Decimal number to define how many decimal places to output.\n"
            "\t\t--fft <Algo>\t\t\t= Use an FFT algorithm to compute the DFT. Selecte between 'radix2-dit'.\n"
            "\t\t--dft\t\t\t\t= Regular DFT calculation using Euler's formula to expand the summation. Default behaviour, included for completion.\n"
            "\t\t--timer\t\t\t\t= Start a timer to see how long the calculation takes.\n"
            "\t\t--info\t\t\t\t= Output to stdout some info about the input file.\n"
            "\t-q,\t--quiet\t\t\t\t= Silence all status messages to stdout. Overwrites '--timer' and '--info'.\n"
            "\t-p,\t--power-spectrum\t\t= Output the power spectrum instead of the DFT itself.\n"
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
