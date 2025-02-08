#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>

#define MAX_STR 30
#define WELCOME_STR ""
#define VERSION_STR ""
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
    char ifile[MAX_STR];
    char ofile[MAX_STR];
    sf_count_t sf_count;    // To count number of data read
    long long dft_bins;
    double tolerance;
    uint8_t channels;
    clock_t start_time;
    clock_t end_time;
    uint8_t info_flag;
    uint8_t fft_flag;
    uint8_t timer_flag;
    void (*fft)();
    int (*outp)(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
} dftt_config_t;

void set_defaults(dftt_config_t* dftt_conf);
int get_options(int* argc, char** argv, dftt_config_t* dftt_conf);
int open_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf);
int read_file_data(SNDFILE* file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double** x);
int mix2mono(SF_INFO* sf_info, double* x, double** x_mono);
void check_start_timer(dftt_config_t* dftt_conf);
void dft(double _Complex* X, long long* N, double* x);
void set_zeros(double _Complex* X, long long* N, dftt_config_t* dftt_conf);
void check_zero_tolerance(double* x, long long* N, dftt_config_t* dftt_conf);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
void output_info(SF_INFO* sf_info, dftt_config_t* dftt_conf);
int select_outp(char* strval, dftt_config_t* dftt_conf);
int select_fft_algo(char* strval, dftt_config_t* dftt_conf);
int output_file_stdout(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_line(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_csv(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_hex_dump(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
int output_file_c_array(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double _Complex* X);
void check_end_timer_output(dftt_config_t* dftt_conf);
void output_help();
