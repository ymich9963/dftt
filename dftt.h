#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_STR 30
#define WELCOME_STR ""
#define VERSION_STR ""

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

typedef struct DFTT_Config {
    char ifile[MAX_STR];
    char ofile[MAX_STR];
    sf_count_t sf_count;    // To count number of data read
    uint8_t info_flag;
}dftt_config_t;

int get_options(int* argc, char** argv, dftt_config_t* dftt_conf);
int open_file(SNDFILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf);
int read_file(SNDFILE* file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double** x);
void dft(double* X_real, double* X_imag, double* Pow, long long* N, double* x);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
void output_info(SF_INFO* sf_info, dftt_config_t* dftt_conf);
int output_file_double(FILE** file, SF_INFO* sf_info, dftt_config_t* dftt_conf, double* data);
void output_help();
