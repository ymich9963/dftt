#include "dftt.h"

void set_defaults(dftt_config_t* restrict dftt_conf)
{
    memset(dftt_conf->ofile, '\0', MAX_STR);

    dftt_conf->total_samples    = 0;
    dftt_conf->detected_samples = 0;
    dftt_conf->channels         = 1;
    dftt_conf->precision        = 6;
    dftt_conf->sampling_freq    = 0;

    dftt_conf->info_flag    = 0;
    dftt_conf->fft_flag     = 0;
    dftt_conf->timer_flag   = 0;
    dftt_conf->input_flag   = 0;
    dftt_conf->quiet_flag   = 0;
    dftt_conf->pow_flag     = 0;
    dftt_conf->norm_flag    = 0;
    dftt_conf->bins_flag    = 0;
    dftt_conf->headers_flag = 1;
    dftt_conf->half_flag    = 0;
    dftt_conf->shift_flag   = 0;

    dftt_conf->inp  = &read_audio_file_input;
    dftt_conf->w    = &window_rectangular;
    dftt_conf->dft  = &dft;
    dftt_conf->outp = &output_stdout;
}

int get_options(int argc, char** restrict argv, dftt_config_t* restrict dftt_conf)
{
    long lval = 0;
    int dval = 0;

    if (argc == 1) {
        fprintf(stdout, WELCOME_STR);

        return 1;
    }

    if (argc == 2) {
        if (!(strcmp("--version", argv[1]))) {
            fprintf(stdout, VERSION_STR);

            return 1;
        }

        if (!(strcmp("--help", argv[1]))) {
            output_help();

            return 1;
        }
    }

    printf("-DFTT START-\n");

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && argv[i - 1][0] != '-') {
            CHECK_STR_LEN(argv[i]);
            strcpy(dftt_conf->ibuff, argv[i]);
            CHECK_RET(read_input(dftt_conf));
            continue;
        }

        if (!(strcmp("-i", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(dftt_conf->ibuff, argv[i + 1]);
            CHECK_RET(read_input(dftt_conf));
            i++;
            continue;
        }

        if (!(strcmp("--input-audio", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(dftt_conf->ibuff, argv[i + 1]);
            dftt_conf->inp = &read_audio_file_input;
            i++;
            continue;
        }

        if (!(strcmp("--input-csv", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(dftt_conf->ibuff, argv[i + 1]);
            dftt_conf->inp = &read_csv_string_file_input;
            i++;
            continue;
        }

        if (!(strcmp("-o", argv[i])) || !(strcmp("--output", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(dftt_conf->ofile, argv[i + 1]);
            i++;
            continue;
        }

        if (!(strcmp("-f", argv[i])) || !(strcmp("--output-format", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            CHECK_RET(select_output_format(dftt_conf, argv[i + 1]));
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

        if (!(strcmp("-w", argv[i])) || !(strcmp("--window", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            CHECK_RET(select_windowing(dftt_conf, argv[i + 1]));
            i++;
            continue;
        }

        if (!(strcmp("--fft", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            CHECK_RET(select_fft_algo(dftt_conf, argv[i + 1]));
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

        if (!(strcmp("-b", argv[i])) || !(strcmp("--bins", argv[i]))) {
            dftt_conf->bins_flag = 1; 
            continue;
        }

        if (!(strcmp("--pow", argv[i])) || !(strcmp("--power-spectrum", argv[i]))) {
            dftt_conf->pow_flag = 1; 
            continue;
        }

        if (!(strcmp("--half", argv[i])) || !(strcmp("--output-half", argv[i]))) {
            dftt_conf->half_flag = 1; 
            continue;
        }

        if (!(strcmp("--norm", argv[i])) || !(strcmp("--normalise", argv[i]))) {
            dftt_conf->norm_flag = 1; 
            continue;
        }

        if (!(strcmp("--shift", argv[i])) || !(strcmp("--fft-shift", argv[i]))) {
            dftt_conf->shift_flag = 1; 
            continue;
        }

        if (!(strcmp("--no-headers", argv[i]))) {
            dftt_conf->headers_flag = 0; 
            continue;
        }

        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n\n", argv[i]);

        return 1;
    }

    return 0;
}

int select_output_format(dftt_config_t* restrict dftt_conf, char* restrict strval)
{
    dftt_conf->outp = NULL; 

    if(!(strcmp("stdout", strval))) {
        dftt_conf->outp = &output_stdout; 
    }
    if(!(strcmp("stdout-csv", strval))) {
        dftt_conf->outp = &output_stdout_csv; 
    }
    if(!(strcmp("columns", strval))) {
        dftt_conf->outp = &output_file_columns; 
    }
    if(!(strcmp("csv", strval))) {
        dftt_conf->outp = &output_file_csv; 
    }
    if(!(strcmp("hex-dump", strval))) {
        dftt_conf->outp = &output_file_hex_dump; 
    }
    if(!(strcmp("c-array", strval))) {
        dftt_conf->outp = &output_file_c_array; 
    }

    if (!dftt_conf->outp){
        fprintf(stderr, "\nOutput format '%s' not available.\n", strval);

        return 1;
    }

    return 0;
}

int read_input(dftt_config_t* restrict dftt_conf)
{
    SNDFILE* file;          // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Check to see if it can be opened as a file */
    file = sf_open(dftt_conf->ibuff, SFM_READ, &sf_info);
    if (file) {
        dftt_conf->inp = &read_audio_file_input;
    } else if (!(check_csv_extension(dftt_conf->ibuff))) {
        dftt_conf->inp = &read_csv_string_file_input;
    } else if (!(check_csv_string(dftt_conf->ibuff))) {
        dftt_conf->inp = &read_csv_string_file_input;
    } else {
        fprintf(stderr, "Input is not an audio file or a CSV file/string.\n");
        return 1;
    }

    sf_close(file);
    return 0;
}

int read_audio_file_input(dftt_config_t* restrict dftt_conf, double** restrict x)
{
    SNDFILE* file;          // Pointer to the input audio file
    SF_INFO sf_info;        // Input audio file info
    double* file_data;      // Input data from file

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Initialise input file buffer and open he input file */
    file = NULL;
    CHECK_ERR(open_audio_file(&file, &sf_info, dftt_conf->ibuff));

    /* Initialise input data array and read the input audio file */
    file_data = NULL; 
    CHECK_ERR(get_audio_file_data(file, &sf_info, &file_data));

    /* Translate the data to one channel (mono) */
    if (sf_info.channels > 1) {
        mix2mono(&sf_info, file_data, x);
    }

    if (!dftt_conf->sampling_freq) {
        dftt_conf->sampling_freq = sf_info.samplerate;
    }
    dftt_conf->detected_samples = sf_info.frames;

    /* Output info on the inputted file */
    output_audio_file_info(dftt_conf, &sf_info);

    return 0;
}

int open_audio_file(SNDFILE** restrict file, SF_INFO* restrict sf_info, char* restrict ibuff)
{
    *file = sf_open(ibuff, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "%s\n", sf_strerror(*file));

        return 1;
    }

    return 0;
}

int get_audio_file_data(SNDFILE* restrict file, SF_INFO* restrict sf_info, double** restrict x)
{
    /* Get audio file data size */
    size_t data_size = sf_info->frames * sf_info->channels;
    *x = calloc(data_size, sizeof(double));

    /* Read data and place into buffer */
    sf_count_t sf_count = sf_readf_double(file, *x, data_size);
    /* Check */
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int output_audio_file_info(dftt_config_t* restrict dftt_conf, SF_INFO* restrict sf_info)
{
    if (dftt_conf->info_flag && !dftt_conf->quiet_flag) {
        fprintf(stdout, "\n--INFO--\n");
        fprintf(stdout, "File Name: %s\n", dftt_conf->ibuff);
        fprintf(stdout, "Sample Rate: %lld\n", dftt_conf->sampling_freq);
        fprintf(stdout, "Samples: %lld\n", sf_info->frames);
        fprintf(stdout, "Channels: %d\n", sf_info->channels);
        fprintf(stdout, "Format: %s\n", get_sndfile_major_format(sf_info));
        fprintf(stdout, "Subtype: %s\n", get_sndfile_subtype(sf_info));
        fprintf(stdout, "---\n\n");
    }

    return 0;
}

const char* get_sndfile_major_format(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint32_t format_mask = 0x00FF0000;
    const uint32_t major_format = sf_info->format & format_mask;

    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof(format_info));
        if (major_format == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

const char* get_sndfile_subtype(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));
        if (subtype == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

int check_csv_string(char* restrict ibuff) {
    /* Make a copy of the string to get the number of data points  */
    char* data_string_copy = calloc(strlen(ibuff), sizeof(char)); 
    strcpy(data_string_copy, ibuff);

    /* Get the number of data points */
    size_t samples = 0;
    char* token = strtok(data_string_copy, ",");
    while (token != NULL) {
        samples++;
        token = strtok(NULL, ",");
    }

    free(data_string_copy);

    if (samples > 1) {
        return 0;
    } else {
        return 1;
    }
}


int check_csv_extension(char* restrict ibuff) {
    const int supported_csv_ext_num = 2;
    char* supported_csv_ext[] = {".csv", ".txt"};
    uint8_t supported_flag = 0;
    int ibuff_len = strlen(ibuff);

    char* ibuff_ext = calloc(ibuff_len, sizeof(char));
    memset(ibuff_ext, '\0', ibuff_len);

    /* Start from 1 to skip any './' */
    for (int i = 1; i < ibuff_len; i++) {
        if (ibuff[i] == '.') {
            for (int c = i, j = 0; c < ibuff_len; c++, j++) {
                ibuff_ext[j] = ibuff[c];
            }
            break;
        }
    }

    for (int i = 0; i < supported_csv_ext_num; i++) {
        if (!strcmp(supported_csv_ext[i], ibuff_ext)) {
            supported_flag = 1;
        }
    }

    free(ibuff_ext);

    if (!supported_flag) {
        return 1;
    }

    return 0;

}
int read_csv_string_file_input(dftt_config_t* restrict dftt_conf, double** restrict x)
{
    FILE* file = NULL;          // Pointer to the input audio file
    char* data_string = NULL;   // Input data from file

    /* Try to open and read input file, if not then it's considered a data string */
    if (!(open_csv_file(&file, dftt_conf->ibuff))) {
        CHECK_ERR(read_csv_file_data(file, &data_string));
        dftt_conf->input_flag = 0;
    } else {
        data_string = calloc(strlen(dftt_conf->ibuff) + 1, sizeof(char)); 
        strcpy(data_string, dftt_conf->ibuff);
        dftt_conf->input_flag = 1;
    }

    get_data_from_string(data_string, x, &dftt_conf->detected_samples);

    /* Output info on the inputted file */
    output_input_info(dftt_conf);

    free(data_string);
    fclose(file);
    return 0;
}

int open_csv_file(FILE** restrict file, char* restrict ibuff)
{
    *file = fopen(ibuff, "r");
    if(!(*file)) {

        return 1;
    }

    return 0;
}

int read_csv_file_data(FILE* restrict file, char** restrict data_string)
{
    /* Go to the end of the file to find the size needed to store the data */
    fseek(file, 0, SEEK_END);

    long pos = ftell(file);

    /* Put the file position indicator back to the start */
    long ret = fseek(file, 0, SEEK_SET);
    if (ret == -1L || pos < 0) {
        fprintf(stderr, "Error when trying to read CSV file data.\n");

        return 1;
    }

    /* Allocate the file size */
    *data_string = calloc(pos + 1, sizeof(char));

    /* Read the file data and place into string variable */
    fgets(*data_string, pos + 1, file);

    return 0;
}

int get_data_from_string(char* restrict data_string, double** restrict x, size_t* restrict detected_samples)
{
    /* Make a copy of the string to get the number of data points  */
    char* data_string_copy = malloc(sizeof(char) * strlen(data_string)); 
    strcpy(data_string_copy, data_string);

    /* Get the number of data points */
    size_t samples = 0;
    char* token = strtok(data_string_copy, ",");
    while (token != NULL) {
        samples++;
        token = strtok(NULL, ",");
    }

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

    *detected_samples = samples;
    free(data_string_copy);

    return 0;
}

int output_input_info(dftt_config_t* restrict dftt_conf)
{
    if (dftt_conf->info_flag && !dftt_conf->quiet_flag) {
        fprintf(stdout, "\n--INFO--\n");
        fprintf(stdout, !dftt_conf->input_flag ? "File Name: %s\n" : "Input String: %s\n", dftt_conf->ibuff);
        fprintf(stdout, "Samples: %lld\n", dftt_conf->detected_samples);
        fprintf(stdout, !dftt_conf->input_flag ? "Format: CSV File\n" : "Format: CSV String\n");
        fprintf(stdout, "--------\n\n");
    }

    return 0;
}

int select_windowing(dftt_config_t* restrict dftt_conf, char* restrict strval)
{
    dftt_conf->w = NULL;

    if (!(strcmp("rectangular", strval))) {
        dftt_conf->w = &window_rectangular;
    }
    if (!(strcmp("hann", strval))) {
        dftt_conf->w = &window_hann;
    }
    if (!(strcmp("hamming", strval))) {
        dftt_conf->w = &window_hamming;
    }
    if (!(strcmp("blackman", strval))) {
        dftt_conf->w = &window_blackman;
    } 

    if (!dftt_conf->w){
        fprintf(stderr, "\nWindowing function '%s' not implemented. Exiting...\n\n", strval);

        return 1;
    }

    return 0;
}

void window_rectangular(dftt_config_t* restrict dftt_conf, double* restrict x)
{
    /* Do nothing to simulate a rectangualr window of w[n] = 1 */
    STATUS(dftt_conf->quiet_flag, "Used a rectangular window.\n");
}

void window_hann(dftt_config_t* restrict dftt_conf, double* restrict x)
{
    for (size_t n = 0; n < dftt_conf->detected_samples; n++) {
        x[n] *= 0.5 - (0.5 * cos((2 * M_PI * n)/(dftt_conf->detected_samples - 1)));
    }
    STATUS(dftt_conf->quiet_flag, "Used a Hann window.\n");
}

void window_hamming(dftt_config_t* restrict dftt_conf, double* restrict x)
{
    for (size_t n = 0; n < dftt_conf->detected_samples; n++) {
        x[n] *= 0.54 - (0.46 * cos((2 * M_PI * n)/(dftt_conf->detected_samples - 1)));
    }
    STATUS(dftt_conf->quiet_flag, "Used a Hamming window.\n");
}

void window_blackman(dftt_config_t* restrict dftt_conf, double* restrict x)
{
    for (size_t n = 0; n < dftt_conf->detected_samples; n++) {
        x[n] *= 0.42 - (0.5 * cos((2 * M_PI * n)/(dftt_conf->detected_samples - 1))) + (0.08 * cos((4 * M_PI * n)/(dftt_conf->detected_samples- 1)));
    }
    STATUS(dftt_conf->quiet_flag, "Used a Blackman window.\n");
}

void mix2mono(SF_INFO* restrict sf_info, double* restrict x, double** restrict x_mono)
{
    uint64_t i = 0;
    uint16_t c = 0;

    *x_mono = calloc(sf_info->frames, sizeof(double));

    for (i = 0; i < sf_info->frames; i++) {
        for (c = 0; c < sf_info->channels; c++) {
            (*x_mono)[i] += (x[sf_info->channels * i + c]/sf_info->channels);
        }
    }
}

int select_fft_algo(dftt_config_t* restrict dftt_conf, char* restrict strval)
{
    dftt_conf->dft = NULL;

    if (!(strcmp("radix2-dit", strval))) {
        dftt_conf->dft = &fft_radix2_dit;
    }
    if (!(strcmp("radix2-dif", strval))) {
        dftt_conf->dft = &fft_radix2_dif;
    }

    if (!dftt_conf->dft) {
        fprintf(stderr, "FFT algorithm '%s' not implemented.\n", strval);
        return 1;
    }

    return 0;
}

void check_timer_start(dftt_config_t* restrict dftt_conf)
{
    if (dftt_conf->timer_flag && !dftt_conf->quiet_flag) {
        timespec_get(&dftt_conf->start_time, TIME_UTC);
        printf("Started timer.\n");
    }
}

int zero_pad_array(double** restrict arr, size_t new_size, size_t old_size)
{
    *arr = realloc(*arr, new_size * sizeof(double));
    if (*arr == NULL) {
        fprintf(stderr, "\nUnable to reallocate input array.\n");

        return 1;
    }

    for (size_t i = old_size; i < new_size; i++) {
        (*arr)[i] = 0.0f;
    }

    return 0;
}

int truncate_array(double** restrict arr, size_t new_size)
{
    *arr = realloc(*arr, new_size * sizeof(double));
    if (*arr == NULL) {
        fprintf(stderr, "\nUnable to reallocate input array.\n");

        return 1;
    }

    return 0;
}

void nextpow2(size_t* restrict num) 
{
    for (size_t i = 1; i < *num << 1; i <<= 1) {

        if (i >= *num) {
            *num = i;
        }

    }
}

int set_transform_size(dftt_config_t* restrict dftt_conf, double complex** restrict X, double** restrict x)
{
    /* If total samples wasn't specified then set as the detected amount */
    if (!dftt_conf->total_samples) {
        dftt_conf->total_samples = dftt_conf->detected_samples;
    }

    /* If using an FFT algo, round up size to next power of 2 */
    if (dftt_conf->fft_flag) {
        nextpow2(&dftt_conf->total_samples);
        if (!(dftt_conf->quiet_flag) && (dftt_conf->detected_samples != dftt_conf->total_samples)) {
            printf("Adjusted input data size to %lld.\n", dftt_conf->total_samples);
        }
    }

    /* Allocate DFT array */
    *X = calloc(dftt_conf->total_samples, sizeof(double complex));

    /* Check if the input array needs expanding/truncating before executing a DFT */
    if (dftt_conf->detected_samples < dftt_conf->total_samples) {
        CHECK_RET(zero_pad_array(x, dftt_conf->total_samples, dftt_conf->detected_samples));
        printf("Zero-padded input.\n");
    }
    if (dftt_conf->detected_samples > dftt_conf->total_samples) {
        CHECK_RET(truncate_array(x, dftt_conf->total_samples));
        printf("Truncated input.\n");
    }

    return 0;
}

void index_bit_reversal(size_t* restrict index_arr, size_t n) 
{
    index_arr[0] = 0;
    index_arr[1] = 1;

    for (int c = 4; c <= n; c <<= 1) {

        for (int i = 0; i < (c >> 1); i++) {
            index_arr[i] <<= 1;
        }

        for (int i = 0; i < (c >> 1); i++) {
            index_arr[i + (c >> 1)] = index_arr[i] + 1;
        }

    }

}

void reorder_data_dit(size_t* restrict index_arr, double* restrict data_arr, size_t data_size)
{
    double copy_arr[data_size];
    memcpy(copy_arr, data_arr, sizeof(copy_arr));

    for (size_t i = 0; i < data_size; i++) {
        data_arr[i] = copy_arr[index_arr[i]];
    }
}

void reorder_data_dif(size_t* restrict index_arr, double complex* restrict data_arr, size_t data_size)
{
    double complex copy_arr[data_size];
    memcpy(copy_arr, data_arr, sizeof(copy_arr));

    for (size_t i = 0; i < data_size; i++) {
        data_arr[index_arr[i]] = copy_arr[i];
    }
}

void convert_to_complex(double* restrict x, double complex* restrict X_complex, size_t size)
{
    for (size_t i = 0; i < size; i++){
        X_complex[i] = creal(x[i]);
    }
}

double complex get_twiddle_factor(size_t nk, size_t N)
{
    const double theta = 2 * M_PI * nk / N;
    double complex w = cos(theta) - (I * sin(theta)); 

    return w;
}

void dft(dftt_config_t* restrict dftt_conf, double complex* restrict X, double* restrict x)
{
    uint64_t n;                 // Sample index in time domain
    uint64_t k;                 // Sample index in frequency domain
    uint64_t N;                 // Total samples

    N = dftt_conf->total_samples;

    STATUS(dftt_conf->quiet_flag, "Calculating DFT.\n");

    /* Calculate DFT */
    for (k = 0; k < N; k++) {
        for (n = 0; n < N; n++) { 
            X[k] += (x[n] * cos(2 * M_PI * n * k / N)) - (I * x[n] * sin(2 * M_PI * n * k / N));
        }
    }
}

void butterfly_dit(double complex* restrict X, double complex* restrict X_copy, size_t k)
{
    double complex w, a, b;

    for (size_t N = 2; N <= k; N *= 2) {
        /* Counter j moves the point of where the N-size sequence starts */
        for (size_t j = 0; j < k; j += N) {
            for (size_t n = j, nk = 0; n < j + (N/2); n++, nk++) {
                a = X_copy[n];
                b = X_copy[n + (N/2)];
                w = get_twiddle_factor(nk, N);

                X[n] = a + (b * w);
                X[n + (N/2)] = a - (b * w);
            }

        }

        /* Using the input array to store the data to be used in the next loop */
        for (size_t i = 0; i < k; i++) {
            X_copy[i] = X[i];
        }

    }
}

void butterfly_dif(double complex* restrict X, double complex* restrict X_copy, size_t k)
{
    double complex w, a, b;

    for (size_t N = k; N >= 2; N /= 2) {
        for (size_t j = 0; j < k; j += N) {
            for (size_t n = j, nk = 0; n < j + (N/2); n++, nk++) {
                a = X_copy[n];
                b = X_copy[n + (N/2)];
                w = get_twiddle_factor(nk, N);

                X[n] = (a + b);
                X[n + (N/2)] = (a - b) * w;
            }

        }

        /* Using the input array to store the data to be used in the next loop */
        for (size_t i = 0; i < k; i++) {
            X_copy[i] = X[i];
        }

    }
}

void fft_radix2_dit(dftt_config_t* restrict dftt_conf, double complex* restrict X, double* restrict x)
{
    STATUS(dftt_conf->quiet_flag, "Calculating DFT using Radix-2 Decimation In Time.\n");

    /* Get the array of the bit-reversed indexes */
    size_t* index_arr = calloc(dftt_conf->total_samples, sizeof(size_t));
    index_bit_reversal(index_arr, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Generated bit-reversed index array.\n");

    /* Re-order the data based on the bit-reversed indexes */
    reorder_data_dit(index_arr, x, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Re-ordered data based on the new index array.\n");

    /* Create an array to convert the mono input to complex values */
    double complex* X_copy = calloc(dftt_conf->total_samples, sizeof(double complex));
    convert_to_complex(x, X_copy, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Converted mono input to complex values.\n");

    /* Execute butterfly and twiddle factor calculations */
    butterfly_dit(X, X_copy, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Executed butterfly procedure.\n");

    free(index_arr);
    free(X_copy);
}

void fft_radix2_dif(dftt_config_t* restrict dftt_conf, double complex* restrict X, double* restrict x)
{
    STATUS(dftt_conf->quiet_flag, "Calculating DFT using Radix-2 Decimation In Frequency.\n");

    /* Create an array to convert the mono input to complex values */
    double complex* X_copy = calloc(dftt_conf->total_samples, sizeof(double complex));
    convert_to_complex(x, X_copy, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Converted mono input to complex values.\n");

    /* Execute butterfly and twiddle factor calculations */
    butterfly_dif(X, X_copy, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Executed butterfly procedure.\n");

    /* Get the array of the bit-reversed indexes */
    size_t* index_arr = calloc(dftt_conf->total_samples, sizeof(size_t));
    index_bit_reversal(index_arr, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Generated bit-reversed index array.\n");

    /* Re-order the data based on the bit-reversed indexes */
    reorder_data_dif(index_arr, X, dftt_conf->total_samples);
    STATUS(dftt_conf->quiet_flag, "Re-ordered data based on the new index array.\n");

    free(X_copy);
    free(index_arr);
}

int get_freq_bins(double* restrict X_bins, size_t f_s, size_t size)
{
    if (!f_s) {
        fprintf(stderr, "\n To calculate frequency bins, specify the sampling frequency of the data with '-s/--sampling-frequency'.\n");
        return 1;
    }

    for (size_t i = 0; i < size; i++) {
        X_bins[i] = (double) i * f_s / size;
    }

    return 0;
}

void parse_complex_buff_to_RIB(double complex* restrict X, double*** restrict X_RIB, size_t size)
{
    /* Allocate 2D array */
    *X_RIB = malloc(3 * sizeof(double*));

    (*X_RIB)[FREQ_BINS_INDEX] = malloc(size * sizeof(double));
    (*X_RIB)[REAL_DATA_INDEX] = malloc(size * sizeof(double));
    (*X_RIB)[IMAG_DATA_INDEX] = malloc(size * sizeof(double));

    /* Copy over data to new 2D array */
    for (size_t i = 0; i < size; i++) {
        (*X_RIB)[FREQ_BINS_INDEX][i] = -1.0f;
        (*X_RIB)[REAL_DATA_INDEX][i] = creal(X[i]);
        (*X_RIB)[IMAG_DATA_INDEX][i] = cimag(X[i]);
    }
}

void set_precision_format(char format[9], uint8_t precision)
{
    sprintf(format, "%%.%dlf", precision);
}

void get_pow_spectrum(double* restrict X_real, double* restrict X_imag, size_t size)
{
    double magn_sq = 1.0f;
    for (size_t i = 0; i < size; i++) {
        magn_sq = (X_real[i] * X_real[i]) + (X_imag[i] * X_imag[i]);
        X_real[i] = magn_sq / size;
        X_imag[i] = -1.0f;    
    }
}

void normalise_data(double* restrict x, size_t size)
{
    double max_val = DBL_MIN;
    for (size_t i = 0; i < size; i++) {
        if (x[i] > max_val) {
            max_val = x[i];
        }
    }

    for (size_t i = 0; i < size; i++) {
        x[i] /= max_val;
    }
}

void fft_shift(double** restrict X_RIB, size_t size)
{
    double copy[3][size];
    for (int data_i = 0; data_i < 3; data_i++) {
        /* First copy the data over */
        for (size_t i = 0; i < size; i++) {
            copy[data_i][i] = X_RIB[data_i][i];
        }

        /* Check if even size */
        if (!(size % 2)) {
            for (size_t i = 0; i < size/2; i++) {
                X_RIB[data_i][i + size/2] = copy[data_i][i];
            }
            for (size_t i = size/2; i < size; i++) {
                X_RIB[data_i][i - size/2] = copy[data_i][i];
            }
        } else {
            for (size_t i = 0; i < size/2 + 1; i++) {
                X_RIB[data_i][i + size/2] = copy[data_i][i];
            }
            for (size_t i = size/2 - 1; i < size; i++) {
                X_RIB[data_i][i - size/2 - 1] = copy[data_i][i];
            }
        }
    }

    double f_s = 2 * X_RIB[FREQ_BINS_INDEX][0];
    for (size_t i = 0; i < size; i++) {
        /* If the loop reaches the 0 bin, exit */
        if (X_RIB[FREQ_BINS_INDEX][i] == 0) {
            break;
        } 
        X_RIB[FREQ_BINS_INDEX][i] -= f_s;
    }
}

void prep_outp(dftt_config_t* restrict dftt_conf, double** restrict X_RIB)
{
    set_precision_format(dftt_conf->format, dftt_conf->precision);

    /* Calculate power spectrum */
    if (dftt_conf->pow_flag) {
        get_pow_spectrum(X_RIB[REAL_DATA_INDEX], X_RIB[IMAG_DATA_INDEX], dftt_conf->total_samples);
        STATUS(dftt_conf->quiet_flag, "Calculated power spectrum.\n");
    }

    if (dftt_conf->norm_flag && dftt_conf->pow_flag) {
        normalise_data(X_RIB[REAL_DATA_INDEX], dftt_conf->total_samples);
        STATUS(dftt_conf->quiet_flag, "Normalised the data.\n");
    }

    if (dftt_conf->bins_flag) {
        CHECK_ERR(get_freq_bins(X_RIB[FREQ_BINS_INDEX], dftt_conf->sampling_freq, dftt_conf->total_samples));
        STATUS(dftt_conf->quiet_flag, "Calculated frequency bins.\n");
    }

    if (dftt_conf->shift_flag) {
        fft_shift(X_RIB, dftt_conf->total_samples);
        STATUS(dftt_conf->quiet_flag, "Shifted the result to be between -N/2 and N/2.\n");
    }

    if (dftt_conf->half_flag) {
        dftt_conf->total_samples = dftt_conf->total_samples / 2;
        STATUS(dftt_conf->quiet_flag, "Will only output up to N/2.\n");
    }
}

char* get_datetime_string()
{
    time_t time_since_epoch = time(NULL);
    struct tm* tm = localtime(&time_since_epoch);
    static char s[13];
    strftime(s, sizeof(s), "%d%m%y%H%M%S", tm);

    return s;
}

void generate_file_name(char* restrict ofile, char* restrict ifile, uint8_t input_flag)
{
    if (ofile[0] != '\0' ) {

        return;
    }

    if (!(input_flag)) {
        char ifile_no_extension[MIN_STR];

        /* Remove the extension in the input file name */
        strncpy(ifile_no_extension, ifile, strlen(ifile) - 4);

        /* Fix an issue when the copied string is not terminated correctly */
        ifile_no_extension[strlen(ifile) - 4] = '\0';

        /* Remove the path specifier */
        if (ifile_no_extension[0] == '.' && ifile_no_extension[1] == '\\') {
            memmove(ifile_no_extension, ifile_no_extension + 2, MIN_STR - 2);
        }

        sprintf(ofile, "dftt-%s-%s.txt", ifile_no_extension, get_datetime_string()); 
    } else {
        sprintf(ofile, "dftt-stringcsv-%s.txt", get_datetime_string()); 
    }
}

//TODO: Add optional precision for frequency bins
void print_freq_bin(FILE* restrict file, double freq_bin, uint8_t bins_flag, char separator[4])
{
    if (bins_flag) {
        fprintf(file, "%.0f%s", freq_bin, separator);
    }
}

void check_neg_zero(double* restrict x)
{
    if (*x == 0.0f && signbit(*x)) {
        *x = +0.0f;
    }
}

uint8_t print_csv_headings(FILE* restrict file, uint8_t headers_flag, uint8_t pow_flag, uint8_t bins_flag)
{
    uint8_t flag_check = (headers_flag << 2) | (pow_flag << 1) | bins_flag;

    switch (flag_check) {
        case (0b100):
            fprintf(file, "Real,Imag\n"); 
            break;
        case (0b110):
            fprintf(file, "Bins,Real,Imag\n"); 
            break;
        case (0b111):
            fprintf(file, "Bins,Pow\n"); 
            break;
        case (0b101):
            fprintf(file, "Pow\n"); 
            break;
        default:
            /* If the headers_flag is not set it goes to the default branch */
            flag_check = 0b0;
            break;
    }

    return flag_check;
}

int output_stdout(dftt_config_t* dftt_conf, double** X_RIB)
{
    FILE * file = stdout;
    if (!dftt_conf->quiet_flag) {
        fprintf(file, "\n");
    }

    for (size_t i = 0; i < dftt_conf->total_samples; i++) {
        print_freq_bin(file, X_RIB[FREQ_BINS_INDEX][i], dftt_conf->bins_flag, ":\t");
        fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][i]);
        if (!dftt_conf->pow_flag) {
            check_neg_zero(&X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, X_RIB[IMAG_DATA_INDEX][i] >= 0 ? " + j" : " - j");
            fprintf(file, dftt_conf->format, fabs(X_RIB[IMAG_DATA_INDEX][i]));
        }
        fprintf(file, "\n");
    }

    return 0;
}

int output_stdout_csv(dftt_config_t* dftt_conf, double** X_RIB)
{
    FILE * file = stdout;
    if (!dftt_conf->quiet_flag) {
        fprintf(file, "\n");
    }

    for (size_t i = 0; i < dftt_conf->total_samples; i++) {
        print_freq_bin(file, X_RIB[FREQ_BINS_INDEX][i], dftt_conf->bins_flag, ",");
        fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][i]);
        if (!dftt_conf->pow_flag) {
            fprintf(file, ",");
            check_neg_zero(&X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, dftt_conf->format, fabs(X_RIB[IMAG_DATA_INDEX][i]));
        }
        fprintf(file, ";");
    }
    fprintf(file, "\n");

    return 0;
}

int output_file_columns(dftt_config_t* dftt_conf, double** X_RIB)
{
    FILE* file = fopen(dftt_conf->ofile, "w");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    for (size_t i = 0; i < dftt_conf->total_samples; i++) {
        print_freq_bin(file, X_RIB[FREQ_BINS_INDEX][i], dftt_conf->bins_flag, ":\t");
        fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][i]);
        if (!dftt_conf->pow_flag) {
            check_neg_zero(&X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, X_RIB[IMAG_DATA_INDEX][i] >= 0 ? " + j" : " - j");
            fprintf(file, dftt_conf->format, fabs(X_RIB[IMAG_DATA_INDEX][i]));
        }
        fprintf(file, "\n");
    }

    if (!dftt_conf->quiet_flag) {
        printf("Outputted data to '%s'.\n", dftt_conf->ofile);
    }

    fclose(file);
    return 0;
}

int output_file_csv(dftt_config_t* dftt_conf, double** X_RIB)
{
    FILE* file = fopen(dftt_conf->ofile, "w");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    print_csv_headings(file, dftt_conf->headers_flag, dftt_conf->pow_flag, dftt_conf->bins_flag);

    for (size_t i = 0; i < dftt_conf->total_samples; i++) {
        print_freq_bin(file, X_RIB[FREQ_BINS_INDEX][i], dftt_conf->bins_flag, ",");
        fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][i]);
        if (!dftt_conf->pow_flag) {
            fprintf(file, ",");
            check_neg_zero(&X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, dftt_conf->format, X_RIB[IMAG_DATA_INDEX][i]);
        }
        fprintf(file, "\n");
    }

    if (!dftt_conf->quiet_flag) {
        printf("Outputted data to '%s'.\n", dftt_conf->ofile);
    }

    fclose(file);
    return 0;
}

int output_file_hex_dump(dftt_config_t* dftt_conf, double** X_RIB)
{
    uint8_t sections = 0;
    FILE* file = fopen(dftt_conf->ofile, "wb");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    if (dftt_conf->bins_flag) {
        fwrite(X_RIB[FREQ_BINS_INDEX], sizeof(double), dftt_conf->total_samples, file);
        sections++;
    }

    fwrite(X_RIB[REAL_DATA_INDEX], sizeof(double), dftt_conf->total_samples, file);
    sections++;

    if (dftt_conf->pow_flag) {
        fwrite(X_RIB[IMAG_DATA_INDEX], sizeof(double), dftt_conf->total_samples, file);
        sections++;
    }

    fprintf(stdout, "Data covers %d x %llud byte sections.\n", sections, sizeof(double) * dftt_conf->total_samples);

    fclose(file);
    return 0;
}

int output_file_c_array(dftt_config_t* dftt_conf, double** X_RIB)
{
    FILE* file = fopen(dftt_conf->ofile, "w");
    if(!(file)) {
        fprintf(stderr, "\nError, unable to open output file.\n\n");

        return 1;
    };

    fprintf(file, "#define DFT_ARR_SIZE\t%lld\n\n", dftt_conf->total_samples);

    if (dftt_conf->bins_flag) {
        fprintf(file, "X_Bins[DFT_ARR_SIZE] = [");
        for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
            fprintf(file, dftt_conf->format, X_RIB[FREQ_BINS_INDEX][i]);
            fprintf(file, ", ");
        }

        fprintf(file, dftt_conf->format, X_RIB[FREQ_BINS_INDEX][dftt_conf->total_samples]);
        fprintf(file, "];\n\n");
    }

    if (dftt_conf->pow_flag) {
        fprintf(file, "X_Pow[DFT_ARR_SIZE] = [");
    } else {
        fprintf(file, "X_real[DFT_ARR_SIZE] = [");
    }

    for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
        fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][i]);
        fprintf(file, ", ");
    }

    fprintf(file, dftt_conf->format, X_RIB[REAL_DATA_INDEX][dftt_conf->total_samples]);
    fprintf(file, "];\n\n");

    if (!dftt_conf->pow_flag) {
        fprintf(file, "X_imag[DFT_ARR_SIZE] = [");

        for (size_t i = 0; i < (dftt_conf->total_samples) - 1; i++){
            check_neg_zero(&X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, dftt_conf->format, X_RIB[IMAG_DATA_INDEX][i]);
            fprintf(file, ", ");
        }

        fprintf(file, dftt_conf->format, X_RIB[IMAG_DATA_INDEX][dftt_conf->total_samples]);
        fprintf(file, "];\n\n");
    }

    fclose(file);
    return 0;
}

void check_timer_end_output(dftt_config_t* dftt_conf)
{
    if (dftt_conf->timer_flag && !dftt_conf->quiet_flag) {
        double time_taken;
        timespec_get(&dftt_conf->end_time, TIME_UTC);
        time_taken = (dftt_conf->end_time.tv_sec - dftt_conf->start_time.tv_sec) + ((dftt_conf->end_time.tv_nsec - dftt_conf->start_time.tv_nsec) / 1e9);
        printf("Time taken: %.9lf seconds\n", time_taken);
    }
}

int output_help()
{
    printf( "\n"
            "Discrete Fourier Transform Tool (DFTT) help page.\n\n"
            "Basic usage 'dftt <Input audio file or CSV file or CSV string> [options]. For list of options see below.\n\n"
            "\t\t--fft <Algo>\t\t\t= Use an FFT algorithm to compute the DFT. Selecte between 'radix2-dit'.\n"
            "\t\t--dft\t\t\t\t= Regular DFT calculation using Euler's formula to expand the summation. Default behaviour, included for completion.\n"
            "\t\t--timer\t\t\t\t= Start a timer to see how long the calculation takes.\n"
            "\t\t--info\t\t\t\t= Output to stdout some info about the input file.\n"
            "\t-i,\t--input <File/String>\t= Accepts audio files and CSV files or strings. Make sure to separate string with commas, e.g. 1,0,0,1. Use the options below if you want to specify but DFTT implements auto-detection.\n"
            "\t\t--input-audio\n"
            "\t\t--input-csv\n"
            "\t-o,\t--output <File Name>\t\t= Path or name of the output file.\n"
            "\t-f,\t--output-format <Format>\t= Format of the output file. Select between: 'stdout', 'stdout-csv', 'columns', 'csv', 'hex-dump', and 'c-array'.\n"
            "\t-N,\t--total-samples <Number>\t= Set total number of samples to use when calculating. If using the FFT, it rounds up to the next power of 2 samples, zero-padding the signal if necessary.\n"
            "\t-p,\t--precision <Number>\t\t= Decimal number to define how many decimal places to output.\n"
            "\t-s,\t--sampling-frequency <Number>\t= Specify sampling frequency, only used when showing the frequency bins.\n"
            "\t-w,\t--window <Window>\t\t= Select a windowing function. Choose between 'rectangular', 'hann', 'hamming', and 'blackman'.\n"
            "\t-q,\t--quiet\t\t\t\t= Silence all status messages to stdout. Overwrites '--timer' and '--info'.\n"
            "\t-b,\t--bins\t\t\t\t= Show the frequency bins in the output.\n"
            "\t--pow,\t--power-spectrum\t\t= Output the power spectrum instead of the DFT itself.\n"
            "\t--norm,\t--normalise\t\t= Normalise the data. Only works wit --pow.\n"
            "\t--half,\t--output-half\t\t\t= Output only half of the result.\n"
            "\t--shift,--fft-shift\t\t\t= Shift the result between -N/2 and N/2.\n"
            "\t--no-headers\t\t\t= No headers in the output.\n"
            "\n"
            );

    return 0;
}

