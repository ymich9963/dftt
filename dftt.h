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

extern char* snd_subtype_arr_desc[SND_SUBTYPE_NUM];
extern uint32_t snd_subtype_arr[SND_SUBTYPE_NUM];
extern char* snd_format_arr_desc[SND_MAJOR_FORMAT_NUM];
extern uint32_t snd_format_arr[SND_MAJOR_FORMAT_NUM];

typedef struct DFTT_Config dftt_config_t;

typedef struct DFTT_Config {
    char ibuff[MAX_STR];
    char ofile[MAX_STR];
    uint8_t precision;
    uint8_t channels;
    size_t total_samples; 
    size_t detected_samples; 
    size_t sampling_freq; 

    clock_t start_time;
    clock_t end_time;

    bool info_flag;
    bool fft_flag;
    bool timer_flag;
    bool input_flag;
    bool quiet_flag;
    bool pow_flag;
    bool bins_flag;

    int (*inp)(double** x, dftt_config_t* dftt_conf);
    void (*dft)(double _Complex* X, double* x_mono, dftt_config_t* dftt_conf);
    int (*outp)(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
} dftt_config_t;

void set_defaults(dftt_config_t* dftt_conf);
int get_options(int* argc, char** argv, dftt_config_t* dftt_conf);
int read_audio_file_input(double** x, dftt_config_t* dftt_conf);
int read_csv_string_file_input(double** x, dftt_config_t* dftt_conf);
int open_audio_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf);
int open_csv_file(FILE** file, dftt_config_t* dftt_conf);
int read_audio_file_data(SNDFILE* file, SF_INFO* sf_info, double** x);
int read_csv_file_data(FILE* file, dftt_config_t* dftt_conf, char** data_string);
int get_data_from_string(char* data_string, double** x, dftt_config_t* dftt_conf);
int mix2mono(SF_INFO* sf_info, double* x, double** x_mono);
void convert_to_complex(double* x, double _Complex* X_complex, size_t* size);
void check_start_timer(dftt_config_t* dftt_conf);
void set_transform_size(dftt_config_t* dftt_conf, double _Complex** X, double** x);
void set_adjusted_size(size_t* size);
void index_bit_reversal(size_t* n, size_t* arr);
void reorder_data(size_t* index_arr, double* data_arr, size_t* data_size);
void dft(double _Complex* X, double* x, dftt_config_t* dftt_conf);
double _Complex get_twiddle_factor(size_t* nk, size_t* N);
void fft_radix2_dit(double _Complex* X, double* x, dftt_config_t* dftt_conf);
void pow_spec(double _Complex* X, dftt_config_t* dftt_conf);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
void output_audio_file_info(SF_INFO* sf_info, dftt_config_t* dftt_conf);
void output_csv_file_string_info(dftt_config_t* dftt_conf);
int select_outp(char* strval, dftt_config_t* dftt_conf);
int select_fft_algo(char* strval, dftt_config_t* dftt_conf);
int output_file_stdout(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_txt_line(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_csv(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_hex_dump(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_c_array(FILE** file, dftt_config_t* dftt_conf, double _Complex* X);
void check_end_timer_output(dftt_config_t* dftt_conf);
void output_help();
