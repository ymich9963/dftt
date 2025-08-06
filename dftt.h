#pragma once

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <sndfile.h>

#define MAX_STR 999
#define MIN_STR 200
#define WELCOME_STR "\nDiscrete Fourier Transform Tool (DFTT). Created by Yiannis Michael (ymich9963), 2025.\n\nUse '--version' for version information, or '--help' for the list of options.\n\nSimple usage, 'dftt <Audio File>'.\n\nUsing FFT, 'dftt <Audio File> --fft <FFT algorithm>.\n\n"
#define VERSION_STR "\nDFTT Version 0.5.0, 2025.\n\n"
#define SND_MAJOR_FORMAT_NUM 27
#define SND_SUBTYPE_NUM 36
#define REAL_DATA_INDEX 0
#define IMAG_DATA_INDEX 1
#define FREQ_BINS_INDEX 2

/* Check macros */
/* Check response from sscanf */
#define	CHECK_RES(x) ({ if (!(x)) { \
							fprintf(stderr, "Argument entered was wrong...\n"); \
							return 1; \
						} \
					  })

/* Check if an error occured to exit program */
#define	CHECK_ERR(x) ({ if ((x)) { \
							exit(EXIT_FAILURE); \
						} \
					  })

/* Check if a function returns failure */
#define	CHECK_RET(x) ({ if ((x)) { \
							return 1; \
						} \
					  })

/* Check the quiet flag and output a simple sting */
#define	STATUS(x, string) ({ if ((!x)) { \
                            printf(string); \
						} \
					  })

/* Check string length */
#define	CHECK_STR_LEN(x) ({ if (strlen((x)) > MAX_STR) { \
							fprintf(stderr, "Argument string length was too large. Max is %d.\n", MAX_STR); \
							return 1; \
						} \
					  })

typedef struct DFTT_Config dftt_config_t;

typedef struct DFTT_Config {
    /* Buffers */
    char ibuff[MAX_STR];
    char ofile[MAX_STR];

    /* Audio file vars */
    uint8_t channels;
    size_t total_samples; 
    size_t detected_samples; 
    size_t sampling_freq; 

    /* Timers */
    struct timespec start_time;
    struct timespec end_time;

    /* Format specifier vars */
    char format[9];         // Format string for the output precision
    uint8_t precision;

    /* Flags */
    uint8_t info_flag;
    uint8_t fft_flag;
    uint8_t timer_flag;
    uint8_t input_flag;
    uint8_t quiet_flag;
    uint8_t pow_flag;
    uint8_t norm_flag;
    uint8_t bins_flag;
    uint8_t headers_flag;
    uint8_t half_flag;
    uint8_t shift_flag;

    /* Function pointers */
    int (*inp)(dftt_config_t* dftt_conf, double** x);
    void (*w)(dftt_config_t* dftt_conf, double* x);
    void (*dft)(dftt_config_t* dftt_conf, double _Complex* X, double* x_mono);
    int (*outp)(dftt_config_t* dftt_conf, double** X);
} dftt_config_t;

/**
 * @brief Set default values to make sure DFTT runs correctly.
 *
 * @param dftt_conf DFTT Config struct.
 */
void set_defaults(dftt_config_t* dftt_conf);

/**
 * @brief Get the options specified from the CLI.
 *
 * @param argc Option count.
 * @param argv Option string array.
 * @param dftt_conf DFTT Config struct.
 * @return Success or failure.
 */
int get_options(int argc, char** argv, dftt_config_t* dftt_conf);

/**
 * @brief Read the input and decide how to treat it.
 *
 * @param dftt_conf DFTT Config struct.
 * @return Success or failure.
 */
int read_input(dftt_config_t* dftt_conf);

/**
 * @brief Select the output format.
 *
 * @param dftt_conf DFTT Config struct.
 * @param strval Option value.
 * @return Success or failure.
 */
int select_output_format(dftt_config_t* dftt_conf, char* strval);

/**
 * @brief Read the input as an audio file and place it into a new buffer.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_audio_file_input(dftt_config_t* dftt_conf, double** x);

/**
 * @brief Open the audio file.
 *
 * @param file Pointer to SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param ibuff Input buffer of size MAX_STR 
 * @return Success or failure.
 */
int open_audio_file(SNDFILE** file, SF_INFO* sf_info, char* ibuff);

/**
 * @brief Get the data from the audio file.
 *
 * @param file SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int get_audio_file_data(SNDFILE* file, SF_INFO* sf_info, double** x);

/**
 * @brief Output some info about the inputted audio file. 
 *
 * @param dftt_conf DFTT Config struct.
 * @param sf_info SF_INFO pointer.
 * @return Success or failure.
 */
int output_audio_file_info(dftt_config_t* dftt_conf, SF_INFO* sf_info);

/**
 * @brief Get the SNDFILE major format string. Same as descriptions given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Major format string
 */
const char* get_sndfile_major_format(SF_INFO* sf_info);

/**
 * @brief Get the SNDFILE subtype string. Same as subtypes given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Subtype string.
 */
const char* get_sndfile_subtype(SF_INFO* sf_info);

/**
 * @brief Check the string if is in CSV format.
 *
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int check_csv_string(char* ibuff);

/**
 * @brief Check the extension in the input buffer if it can be read like a CSV.
 *
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int check_csv_extension(char* ibuff);

/**
 * @brief Read the input as a CSV file or CSV string.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_csv_string_file_input(dftt_config_t* dftt_conf, double** x);

/**
 * @brief Read the input as a CSV file or CSV string.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_csv_string_file_input(dftt_config_t* dftt_conf, double** x);

/**
 * @brief Open the input file as a CSV file. If the function fails it means it's a CSV string.
 *
 * @param file Pointer to FILE pointer.
 * @param ibuff Input buffer.
 * @return Success or failure.
 */
int open_csv_file(FILE** file, char* ibuff);

/**
 * @brief Read the data from the CSV file.
 *
 * @param file FILE pointer.
 * @param data_string Pointer to a string buffer.
 * @return Success or failure.
 */
int read_csv_file_data(FILE* file, char** data_string);

/**
 * @brief Get the data from a CSV string and store it in a buffer.
 *
 * @param data_string String containing the data.
 * @param x Pointer to buffer to store the data.
 * @param detected_samples Variable to store the detected samples in the string.
 * @return Success or failure.
 */
int get_data_from_string(char* data_string, double** x, size_t* detected_samples);

/**
 * @brief Output some info about the input.
 *
 * @param dftt_conf DFTT Config struct.
 * @return Success or failure.
 */
int output_input_info(dftt_config_t* dftt_conf);

/**
 * @brief Select the windowing functions based on the input string.
 *
 * @param dftt_conf DFTT Config struct.
 * @param strval Option value.
 * @return Success or failure.
 */
int select_windowing(dftt_config_t* dftt_conf, char* strval);

/**
 * @brief Rectangular window function.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Data buffer.
 */
void window_rectangular(dftt_config_t* dftt_conf, double* x);

/**
 * @brief Hann window function.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Data buffer.
 */
void window_hann(dftt_config_t* dftt_conf, double* x);

/**
 * @brief Hamming window function.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Data buffer.
 */
void window_hamming(dftt_config_t* dftt_conf, double* x);

/**
 * @brief Blackman window function.
 *
 * @param dftt_conf DFTT Config struct.
 * @param x Data buffer.
 */
void window_blackman(dftt_config_t* dftt_conf, double* x);

/**
 * @brief Mix the data to mono.
 *
 * @param sf_info Pointer to SF_INFO struct.
 * @param x Data buffer.
 * @param x_mono Pointer to store the processed data buffer.
 */
void mix2mono(SF_INFO* sf_info, double* x, double** x_mono);

/**
 * @brief Select the FFT algorithm based on the input string.
 *
 * @param dftt_conf DFTT Config struct.
 * @param strval Option value.
 * @return Success or failure.
 */
int select_fft_algo(dftt_config_t* dftt_conf, char* strval);

/**
 * @brief Check if the timer should be started.
 *
 * @param dftt_conf DFTT Config struct.
 */
void check_timer_start(dftt_config_t* dftt_conf);

/**
 * @brief Append the input array with zeros.
 *
 * @param arr Pointer to data buffer.
 * @param new_size New size of the array.
 * @param old_size Old size of the array.
 * @return Success or failure.
 */
int zero_pad_array(double** arr, size_t new_size, size_t old_size);

/**
 * @brief Truncate the input array.
 *
 * @param arr Pointer to data buffer.
 * @param new_size New size of the array.
 * @return Success or failure.
 */
int truncate_array(double** arr, size_t new_size);

/**
 * @brief Find the next power of two of a number.
 *
 * @param size Pointer to the number to find the next power of two for.
 */
void nextpow2(size_t* num);

/**
 * @brief Set the size of the arrays and corresponding variables before executing any DFT.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X Pointer to complex data buffer. 
 * @param x Pointer to input data buffer.
 * @return Success or failure.
 */
int set_transform_size(dftt_config_t* dftt_conf, double _Complex** X, double** x);

/**
 * @brief Generate the index array containing bit-reversed positions that will be used at indexes.
 *
 * @param index_arr Array containing the index data.
 * @param n Size of the index array.
 */
void index_bit_reversal(size_t* index_arr, size_t n);

/**
 * @brief Reorder data to the indexes in the index array.
 *
 * @param index_arr Array containing the index data.
 * @param data_arr Data buffer of type double.
 * @param data_size Size of data buffer.
 */
void reorder_data_dit(size_t* index_arr, double* data_arr, size_t data_size);

/**
 * @brief Reorder data to the indexes in the index array.
 *
 * @param index_arr Array containing the index data.
 * @param data_arr Data buffer of type complex.
 * @param data_size Size of data buffer.
 */
void reorder_data_dif(size_t* index_arr, double _Complex* data_arr, size_t data_size);

/**
 * @brief Convert a buffer of type double to type _Complex.
 *
 * @param x Buffer of type double.
 * @param X_complex Buffer of type _Complex.
 * @param size Size of the buffers.
 */
void convert_to_complex(double* x, double _Complex* X_complex, size_t size);

/**
 * @brief Get the twiddle factor based on the indexes.
 *
 * @param nk Index of current sequence.
 * @param N Size of current sequence.
 * @return Complex double.
 */
double _Complex get_twiddle_factor(size_t nk, size_t N);

/**
 * @brief DFT.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X Data buffer of type complex double. 
 * @param x Data buffer.
 */
void dft(dftt_config_t* dftt_conf, double _Complex* X, double* x);

/**
 * @brief Execute butterfly for the Discrete In Time FFT.
 *
 * @param X Data buffer of type complex double to store the result.
 * @param X_copy Complex copy of the input.
 * @param k Size of the data buffer.
 */
void butterfly_dit(double _Complex* X, double _Complex* X_copy, size_t k);

/**
 * @brief Execute butterfly for the Discrete In Frequency FFT.
 *
 * @param X Data buffer of type double to store the result.
 * @param X_copy Complex copy of the input.
 * @param k Size of the data buffer.
 */
void butterfly_dif(double _Complex* X, double _Complex* X_copy, size_t k);

/**
 * @brief Execute FFT Radix-2 Decimation In Time.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X Data buffer of type complex double. 
 * @param x Data buffer.
 */
void fft_radix2_dit(dftt_config_t* dftt_conf, double _Complex* X, double* x);

/**
 * @brief Execute FFT Radix-2 Decimation In Frequency.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X Data buffer of type complex double. 
 * @param x Data buffer.
 */
void fft_radix2_dif(dftt_config_t* dftt_conf, double _Complex* X, double* x);

/**
 * @brief Parse the complex data buffer into a 3D array which contains Real, Imaginary, and Frequency Bins (RIB). Required to centralise the data for outputing.
 *
 * @param X Complex data buffer 
 * @param X_RIB Pointer to data buffer containing the RIB data.
 * @param size Size of the data buffer. 
 */
void parse_complex_buff_to_RIB(double _Complex* X, double*** X_RIB, size_t size);

/**
 * @brief Get a buffer containing the frequency bin values.
 *
 * @param X_bins Data buffer to store the bin values.
 * @param f_s Sampling frequency.
 * @param size Size of the data buffer that will store the values.
 * @return Success or failure.
 */
int get_freq_bins(double*  X_bins, size_t f_s, size_t size);

/**
 * @brief Set the precision to be used when outputing the data.
 *
 * @param format Format string.
 * @param precision Precision amount.
 */
void set_precision_format(char format[9], uint8_t precision);

/**
 * @brief Get the power spectrum of the data, Gets stored in the Real part of the RIB array.
 *
 * @param X_real Real part of data.
 * @param X_imag Imaginary part of data.
 * @param size Size of data.
 */
void get_pow_spectrum(double*  X_real, double*  X_imag, size_t size);

/**
 * @brief Normalise the data. Only used in the power output.
 *
 * @param x Data buffer.
 * @param size Size of data buffer.
 */
void normalise_data(double*  x, size_t size);

/**
 * @brief Shift the data so that it's centered around 0.
 *
 * @param X_RIB RIB data buffer.
 * @param size Size of data buffer.
 */
void fft_shift(double**  X_RIB, size_t size);

/**
 * @brief Prepare the data to be outputed. Set the precision, get the power spectrum, get the frequency bins, FFT-shift the data, normalise, or output only half.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 */
void prep_outp(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Get the date/time string to be used in the file name.
 *
 * @return Date/time string in ddmmyyHHMMSS format.
 */
char* get_datetime_string();

/**
 * @brief Generate the output file name based on the input file name and the current date/time.
 *
 * @param ofile Output file name.
 * @param ifile Input file name.
 * @param input_flag Flag previously set that specifies the type of input.
 */
void generate_file_name(char* ofile, char* ibuff, uint8_t input_flag);

/**
 * @brief Output column headings. Used in CSV outputs.
 *
 * @param file FILE buffer.
 * @param bins_flag Flag to check if bins are outputed.
 * @param pow_flag Flag to check if power spectrum is outputed.
 * @param pow_flag Flag to output column headers.
 */
void print_csv_headings(FILE* file, uint8_t bins_flag, uint8_t pow_flag, uint8_t headers_flag);

/**
 * @brief Output to stdout in columns.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_stdout(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Output to stdout as CSV. Values are separated with ',' and each entry is separated by ';'.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_stdout_csv(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Output to a file buffer as columns.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_file_columns(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Output to a file buffer as CSV.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_file_csv(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Output to a file buffer as a hex dump.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_file_hex_dump(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Output to a file buffer as a C-style array.
 *
 * @param dftt_conf DFTT Config struct.
 * @param X_RIB RIB data buffer.
 * @return Success or failure.
 */
int output_file_c_array(dftt_config_t* dftt_conf, double** X_RIB);

/**
 * @brief Check if the timer should be stopped and outputed.
 *
 * @param dftt_conf DFTT Config struct.
 */
void check_timer_end_output(dftt_config_t* dftt_conf);

/**
 * @brief Output the '--help' option.
 * @return Success or failure.
 */
int output_help();
