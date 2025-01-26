#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
    uint8_t info_flag;
    long long dft_bins;
    size_t data_size;
    uint8_t channels;
    int (*outp)(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);
} dftt_config_t;

void set_defaults(dftt_config_t* dftt_conf);
int get_options(int* argc, char** argv, dftt_config_t* dftt_conf);
int open_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf);
int read_file_data(SNDFILE* file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double** x);
void set_config(SF_INFO* sf_info, dftt_config_t* dftt_conf);
int mix2mono(SF_INFO* sf_info, double* x, double** x_mono);
void dft(double* X_real, double* X_imag, double* Pow, long long* N, double* x);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
void output_info(SF_INFO* sf_info, dftt_config_t* dftt_conf);
int select_outp(char* strval, dftt_config_t* dftt_conf);
int output_file_stdout(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);
int output_file_line(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);
int output_file_csv(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);
int output_file_hex_dump(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);void output_help();
