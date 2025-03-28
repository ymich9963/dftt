#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <stdbool.h>

#define MAX_STR 999
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

extern char* snd_subtype_arr_desc[SND_SUBTYPE_NUM];
extern uint32_t snd_subtype_arr[SND_SUBTYPE_NUM];
extern char* snd_format_arr_desc[SND_MAJOR_FORMAT_NUM];
extern uint32_t snd_format_arr[SND_MAJOR_FORMAT_NUM];

typedef struct DFTT_Config dftt_config_t;

typedef struct DFTT_Config {
    char ibuff[MAX_STR];
    char ofile[MAX_STR];

    uint8_t channels;
    size_t total_samples; 
    size_t detected_samples; 
    size_t sampling_freq; 

    clock_t start_time;
    clock_t end_time;

    char format[9];         // Format string for the output precision
    uint8_t precision;

    bool info_flag;
    bool fft_flag;
    bool timer_flag;
    bool input_flag;
    bool quiet_flag;
    bool pow_flag;
    bool norm_flag;
    bool bins_flag;
    bool half_flag;
    bool shift_flag;

    int (*inp)(dftt_config_t* dftt_conf, double** x);
    int (*w)(dftt_config_t* dftt_conf, double* x);
    void (*dft)(dftt_config_t* dftt_conf, double _Complex* X, double* x_mono);
    int (*outp)(dftt_config_t* dftt_conf, double** X);
} dftt_config_t;

void set_defaults(dftt_config_t* dftt_conf);
int get_options(int* argc, char** argv, dftt_config_t* dftt_conf);
int read_audio_file_input(dftt_config_t* dftt_conf, double** x);
int open_audio_file(SNDFILE** file, SF_INFO* sf_info, char ibuff[MAX_STR]);
int get_audio_file_data(SNDFILE* file, SF_INFO* sf_info, double** x);
void output_audio_file_info(dftt_config_t* dftt_conf, SF_INFO* sf_info);
int select_outp(dftt_config_t* dftt_conf, char* strval);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
int read_csv_string_file_input(dftt_config_t* dftt_conf, double** x);
int open_csv_file(FILE** file, char ibuf[MAX_STR]);
int read_csv_file_data(FILE* file, char** data_string);
int get_data_from_string(char* data_string, double** x, size_t* detected_samples);
void output_csv_file_string_info(dftt_config_t* dftt_conf);
int select_windowing(dftt_config_t* dftt_conf, char* strval);
int window_rectangular(dftt_config_t* dftt_conf, double* x);
int window_hanning(dftt_config_t* dftt_conf, double* x);
int window_hamming(dftt_config_t* dftt_conf, double* x);
int window_blackman(dftt_config_t* dftt_conf, double* x);
int mix2mono(SF_INFO* sf_info, double* x, double** x_mono);
int select_fft_algo(dftt_config_t* dftt_conf, char* strval);
void check_start_timer(dftt_config_t* dftt_conf);
void zero_pad_array(double** arr, size_t new_size, size_t old_size);
void truncate_array(double** arr, size_t new_size);
void set_transform_size(dftt_config_t* dftt_conf, double _Complex** X, double** x);
void nextpow2(size_t size);
void index_bit_reversal(size_t* index_arr, size_t n);
void reorder_data_dit(size_t* index_arr, double* data_arr, size_t data_size);
void reorder_data_dif(size_t* index_arr, double _Complex* data_arr, size_t data_size);
void convert_to_complex(double* x, double _Complex* X_complex, size_t size);
double _Complex get_twiddle_factor(size_t nk, size_t N);
void dft(dftt_config_t* dftt_conf, double _Complex* X, double* x);
void butterfly_dit(double _Complex* X, double _Complex* x_mono_complex_copy, size_t k);
void fft_radix2_dit(dftt_config_t* dftt_conf, double _Complex* X, double* x);
void fft_radix2_dif(dftt_config_t* dftt_conf, double _Complex* X, double* x);
void dissect_complex_arr(double _Complex* X, double*** X_RIB, size_t size);
void prep_outp(dftt_config_t* dftt_conf, double** X_RIB);
int output_file_stdout(dftt_config_t* dftt_conf, double** X_RIB);
int output_file_txt_line(dftt_config_t* dftt_conf, double** X_RIB);
void print_csv_headings(FILE* file, bool bins_flag, bool pow_flag);
int output_file_csv(dftt_config_t* dftt_conf, double** X_RIB);
int output_file_hex_dump(dftt_config_t* dftt_conf, double** X_RIB);
int output_file_c_array(dftt_config_t* dftt_conf, double** X_RIB);
void check_end_timer_output(dftt_config_t* dftt_conf);
void output_help();
