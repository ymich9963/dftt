#include "unity/unity.h"
#include "unity/unity_internals.h"
#include "../dftt.h"
#include <unistd.h>


void setUp() {}

void tearDown() {
    fflush(stdout);
}

void test_set_defaults() {
    dftt_config_t dftt_conf;

    set_defaults(&dftt_conf);

    TEST_ASSERT_EQUAL_INT(0, dftt_conf.info_flag);
    TEST_ASSERT_EQUAL_INT(6, dftt_conf.precision);
}

void test_output_help() {
    TEST_ASSERT_EQUAL_INT(0, output_help());
}

void split(char* cmd, char** argv, int* argc) {
    char* token = strtok(cmd," "); 
    *argc = 0;
    while(token != NULL) {
        argv[*argc] = token; 
        token = strtok(NULL," "); 
        *argc = *argc + 1;
    }
}

void reset(char** argv, dftt_config_t* dftt_conf, int size) {
    for(int i = 0; i < size; i++) {
        argv[i] = "\0";
    }
    // dftt_config_t x = { 0 };
    // *dftt_conf = x;
}

void test_get_options() {
    dftt_config_t dftt_conf;
    int argc;
    char* argv[40];

    char cmd0[] = "first";
    split(cmd0, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);

    char cmd1[] = "first --version";
    split(cmd1, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);

    char cmd2[] = "first --help";
    split(cmd2, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);

    char cmd3[] = "first --test test";
    split(cmd3, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);

    char cmd4[] = "first test.wav --input-audio test.wav --input-csv 1,0,1,0 --output otest.txt --output-format csv --total-samples 1 --sampling-frequency 1 --precision 1 --window rectangular --fft radix2-dit --dft --timer --info --quiet --show-bins --power-spectrum --output-half --normalise --shift";
    split(cmd4, argv, &argc);
    TEST_ASSERT_EQUAL_INT(0, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);

    char cmd5[] = "first -i test.wav -o otest.txt -f csv -N 1 -s 1 -p 1 -w rectangular -q -b --pow --half --shift --no-headers";
    split(cmd5, argv, &argc);
    TEST_ASSERT_EQUAL_INT(0, get_options(argc, argv, &dftt_conf));
    reset(argv, &dftt_conf, argc);
}

void test_select_output_format() {
    dftt_config_t dftt_conf;

    TEST_ASSERT_EQUAL_INT(1, select_output_format(&dftt_conf, "test"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "stdout"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "stdout-csv"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "columns"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "csv"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "hex-dump"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&dftt_conf, "c-array"));
}

void test_read_input() {
    dftt_config_t dftt_conf = {
        .ibuff = "test.wav"
    };

    read_input(&dftt_conf);
    TEST_ASSERT_EQUAL_PTR(read_audio_file_input, dftt_conf.inp);

    strcpy(dftt_conf.ibuff, "test.txt");
    read_input(&dftt_conf);
    TEST_ASSERT_EQUAL_PTR(read_csv_string_file_input, dftt_conf.inp);

    strcpy(dftt_conf.ibuff, "1,0,0,1");
    read_input(&dftt_conf);
    TEST_ASSERT_EQUAL_PTR(read_csv_string_file_input, dftt_conf.inp);

    strcpy(dftt_conf.ibuff, "test");
    TEST_ASSERT_EQUAL_INT(read_input(&dftt_conf), 1);
}

void test_read_audio_file_input() {
    double* x;
    dftt_config_t dftt_conf = {
        .ibuff = "test.wav",
        .info_flag = 1,
    };

    TEST_ASSERT_EQUAL_INT(0, read_audio_file_input(&dftt_conf, &x));
    TEST_ASSERT_EQUAL_INT(48000, dftt_conf.sampling_freq);
    TEST_ASSERT_EQUAL_INT(48000, dftt_conf.detected_samples);
}

void test_open_audio_file() {
    SNDFILE* sndfile;
    SF_INFO sf_info;
    char ibuff[] = "test.wav";

    TEST_ASSERT_EQUAL_INT(0, open_audio_file(&sndfile, &sf_info, ibuff));

    ibuff[0] = 'b';
    TEST_ASSERT_EQUAL_INT(1, open_audio_file(&sndfile, &sf_info, ibuff));
}

void test_get_audio_file_data() {
    SNDFILE* sndfile;
    SF_INFO sf_info;
    char ibuff[] = "test.wav";
    double* x;

    open_audio_file(&sndfile, &sf_info, ibuff);

    TEST_ASSERT_EQUAL_INT(0, get_audio_file_data(sndfile, &sf_info, &x));
}

void test_output_audio_file_info() {
    dftt_config_t dftt_conf = {
        .info_flag = 1,
        .quiet_flag = 0,
        .ibuff = "test.wav",
        .sampling_freq = 1,
    };
    SF_INFO sf_info = {
        .frames = 1,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8,
    };

    TEST_ASSERT_EQUAL_INT(0, output_audio_file_info(&dftt_conf, &sf_info));
}

void test_get_sndfile_major_format() {
    SF_INFO sf_info = {
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8,
    };

    TEST_ASSERT_EQUAL_STRING("WAV (Microsoft)", get_sndfile_major_format(&sf_info));

    sf_info.format = 0xFFFFFF;
    TEST_ASSERT_EQUAL_STRING("N/A", get_sndfile_major_format(&sf_info));
}

void test_get_sndfile_subtype() {
    SF_INFO sf_info = {
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8,
    };

    TEST_ASSERT_EQUAL_STRING("Signed 8 bit PCM", get_sndfile_subtype(&sf_info));

    sf_info.format = 0xFFFFFF;
    TEST_ASSERT_EQUAL_STRING("N/A", get_sndfile_subtype(&sf_info));
}

void test_check_csv_string() {
    TEST_ASSERT_EQUAL_INT(0, check_csv_string("1,0,0,1"));
    TEST_ASSERT_EQUAL_INT(1, check_csv_extension("1"));
    TEST_ASSERT_EQUAL_INT(1, check_csv_extension("text.exe"));
}

void test_check_csv_extension() {
    TEST_ASSERT_EQUAL_INT(0, check_csv_extension("text.csv"));
    TEST_ASSERT_EQUAL_INT(0, check_csv_extension("text.txt"));
    TEST_ASSERT_EQUAL_INT(1, check_csv_extension("text.exe"));
}

void test_read_csv_string_file_input() {
    double* x1;
    dftt_config_t dftt_conf1 = {
        .ibuff = "test.txt",
    }; 

    TEST_ASSERT_EQUAL_INT(0, read_csv_string_file_input(&dftt_conf1, &x1));
    TEST_ASSERT_EQUAL_INT(4, dftt_conf1.detected_samples);
    TEST_ASSERT_EQUAL_INT(1, x1[0]);
    TEST_ASSERT_EQUAL_INT(0, x1[1]);
    TEST_ASSERT_EQUAL_INT(0, x1[2]);
    TEST_ASSERT_EQUAL_INT(1, x1[3]);

    double* x2;
    dftt_config_t dftt_conf2 = {
        .ibuff = "1,0,0,1",
    }; 

    TEST_ASSERT_EQUAL_INT(0, read_csv_string_file_input(&dftt_conf2, &x2));
    TEST_ASSERT_EQUAL_INT(4, dftt_conf2.detected_samples);
    TEST_ASSERT_EQUAL_INT(1, x2[0]);
    TEST_ASSERT_EQUAL_INT(0, x2[1]);
    TEST_ASSERT_EQUAL_INT(0, x2[2]);
    TEST_ASSERT_EQUAL_INT(1, x2[3]);
}

void test_open_csv_file() {
    FILE* file;
    char ibuff[] = "test.txt";

    TEST_ASSERT_EQUAL_INT(0, open_csv_file(&file, ibuff));

    ibuff[0] = 'b';
    TEST_ASSERT_EQUAL_INT(1, open_csv_file(&file, ibuff));
}

void test_read_csv_file_data() {
    FILE* file;
    char ibuff[] = "test.txt";
    char* data_string;

    open_csv_file(&file, ibuff);

    TEST_ASSERT_EQUAL_INT(0, read_csv_file_data(file, &data_string));

    TEST_ASSERT_EQUAL_STRING("1,0,0,1", data_string);
}

void test_get_data_from_string() {
    double* x;
    char data[] = "1,0,0,1";
    size_t detected_samples;

    TEST_ASSERT_EQUAL_INT(0, get_data_from_string(data, &x, &detected_samples));
    TEST_ASSERT_EQUAL_INT(4, detected_samples);
    TEST_ASSERT_EQUAL_INT(1, x[0]);
    TEST_ASSERT_EQUAL_INT(0, x[1]);
    TEST_ASSERT_EQUAL_INT(0, x[2]);
    TEST_ASSERT_EQUAL_INT(1, x[3]);
}

void test_output_input_info() {
    dftt_config_t dftt_conf = {
        .info_flag = 1,
        .quiet_flag = 0,
        .ibuff = "test.txt",
        .detected_samples = 4,
        .input_flag = 1,
    };

    TEST_ASSERT_EQUAL_INT(0, output_input_info(&dftt_conf));
}

void test_select_windowing() {
    dftt_config_t dftt_conf;

    TEST_ASSERT_EQUAL_INT(0, select_windowing(&dftt_conf, "rectangular"));
    TEST_ASSERT_EQUAL_INT(0, select_windowing(&dftt_conf, "hann"));
    TEST_ASSERT_EQUAL_INT(0, select_windowing(&dftt_conf, "hamming"));
    TEST_ASSERT_EQUAL_INT(0, select_windowing(&dftt_conf, "blackman"));
    TEST_ASSERT_EQUAL_INT(1, select_windowing(&dftt_conf, "test"));
}

void test_windowing() {
    dftt_config_t dftt_conf = {
        .detected_samples = 4,
        .quiet_flag = 1,
    };

    double actual1[] = {1,1,1,1};
    double expected1[] = {1,1,1,1};
    window_rectangular(&dftt_conf, actual1);
    for (int i = 0; i < dftt_conf.detected_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-9,expected1[i], actual1[i]);
    }

    double actual2[] = {1,1,1,1};
    double expected2[] = {0,0.75,0.75,0};
    window_hann(&dftt_conf, actual2);
    for (int i = 0; i < dftt_conf.detected_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-9, expected2[i], actual2[i]);
    }

    double actual3[] = {1,1,1,1};
    double expected3[] = {0.08,0.77,0.77,0.08};
    window_hamming(&dftt_conf, actual3);
    for (int i = 0; i < dftt_conf.detected_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-9, expected3[i], actual3[i]);
    }

    double actual4[] = {1,1,1,1};
    double expected4[] = {-1.387778780781446e-17,0.63,0.63,-1.387778780781446e-17};
    window_blackman(&dftt_conf, actual4);
    for (int i = 0; i < dftt_conf.detected_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-9, expected4[i], actual4[i]);
    }
}

void test_mix2mono() {
    double x[] = {2,2,2,2,2,2,2,2};
    double expected[] = {2,2,2,2};
    double* x_mono;
    SF_INFO sf_info = {
        .channels = 2,
        .frames = 8,
    };

    mix2mono(&sf_info, x, &x_mono);

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_UINT8(expected[i], ((double*)x_mono)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_UINT8(1, ((double*)x_mono)[5]);
}

void test_select_fft_algo() {
    dftt_config_t dftt_conf;

    TEST_ASSERT_EQUAL_INT(1, select_fft_algo(&dftt_conf, "test"));
    TEST_ASSERT_EQUAL_INT(0, select_fft_algo(&dftt_conf, "radix2-dit"));
    TEST_ASSERT_EQUAL_INT(0, select_fft_algo(&dftt_conf, "radix2-dif"));
}

void test_timer() {
    dftt_config_t dftt_conf = {
        .timer_flag = 1,
        .quiet_flag = 0,
    };

    check_timer_start(&dftt_conf);
    usleep(1 * 1000);
    check_timer_end_output(&dftt_conf);
    double time_taken = (dftt_conf.end_time.tv_sec - dftt_conf.start_time.tv_sec) + ((dftt_conf.end_time.tv_nsec - dftt_conf.start_time.tv_nsec) / 1e9);
    TEST_ASSERT(time_taken > 0);
}

void test_zero_pad_array() {
    double* arr;
    size_t old_size = 4;
    size_t new_size = 8;
    double expected[] = {1,1,1,1,0,0,0,0};

    arr = malloc(sizeof(double) * old_size);
    for (size_t i = 0; i < old_size; i++) {
        arr[i] = 1.0f;
    }

    zero_pad_array(&arr, new_size, old_size);

    for (int i = 0; i < new_size; i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], arr[i]);
    }
}

void test_truncate_array() {
    double* arr;
    size_t old_size = 8;
    size_t new_size = 4;
    double expected[] = {1,1,1,1};

    arr = malloc(sizeof(double) * old_size);
    for (size_t i = 0; i < old_size; i++) {
        arr[i] = 1.0f;
    }

    truncate_array(&arr, new_size);

    for (int i = 0; i < new_size; i++) {
        TEST_ASSERT_EQUAL_INT(expected[i], arr[i]);
    }
}

void test_nextpow2() {
    size_t num = 1000;

    nextpow2(&num);

    TEST_ASSERT_EQUAL_INT(1024, num);

    num = 4;
    nextpow2(&num);
    TEST_ASSERT_EQUAL_INT(4, num);
}

void test_set_transform_size() {
    double complex* X;
    double* x;
    double expected_zero_padded[8] = {1,1,1,1,1,0,0,0};
    double expected_truncated[8] = {1,1,1,1,1,1,1,1};
    dftt_config_t dftt_conf = {
        .total_samples = 0,
        .fft_flag = 1,
        .quiet_flag = 0,
        .detected_samples = 5,
    };

    /* Initialise array */
    x = malloc(sizeof(double) * dftt_conf.detected_samples);
    for (size_t i = 0; i < dftt_conf.detected_samples; i++) {
        x[i] = 1.0f;
    }
    
    /* Test that the array will be zero padded */
    set_transform_size(&dftt_conf, &X, &x);
    TEST_ASSERT_EQUAL_INT(8, dftt_conf.total_samples);
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_EQUAL_INT(expected_zero_padded[i], x[i]);
    }

    /* Test that the array will be truncated */
    dftt_conf.total_samples = 5;
    dftt_conf.detected_samples = 10;

    /* Initialise array */
    x = malloc(sizeof(double) * dftt_conf.detected_samples);
    for (size_t i = 0; i < dftt_conf.detected_samples; i++) {
        x[i] = 1.0f;
    }

    set_transform_size(&dftt_conf, &X, &x);
    TEST_ASSERT_EQUAL_INT(8, dftt_conf.total_samples);
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_EQUAL_INT(expected_truncated[i], x[i]);
    }
}

void test_index_bit_reversal() {
    size_t expected_index_arr[] = {0,4,2,6,1,5,3,7};
    const size_t index_arr_size = 8;

    size_t* index_arr = calloc(index_arr_size, sizeof(size_t));
    index_bit_reversal(index_arr, index_arr_size);
    
    for (int i = 0; i < index_arr_size; i++) {
        TEST_ASSERT_EQUAL_INT(expected_index_arr[i], index_arr[i]);
    }
}

void test_reorder_data_dit() {
    const int data_size = 8;
    size_t index_arr[8] = {0,4,2,6,1,5,3,7};
    double data_arr[8] = {10,11,12,13,14,15,16,17};
    double expected_arr[8] = {10,14,12,16,11,15,13,17};

    reorder_data_dit(index_arr, data_arr, data_size);

    for (int i = 0; i < data_size; i++) {
        TEST_ASSERT_EQUAL_INT(expected_arr[i], data_arr[i]);
    }
}

void test_reorder_data_dif() {
    const int data_size = 8;
    size_t index_arr[8] = {0,4,2,6,1,5,3,7};
    double complex data_arr[8] = {10,14,12,16,11,15,13,17};
    double complex expected_arr[8] = {10,11,12,13,14,15,16,17};

    reorder_data_dif(index_arr, data_arr, data_size);

    for (int i = 0; i < data_size; i++) {
        TEST_ASSERT_EQUAL_INT(expected_arr[i], data_arr[i]);
    }
}

void test_convert_to_complex() {
    size_t size = 4;
    double x[] = {1,1,1,1};
    double complex* X = calloc(size, sizeof(double complex));

    convert_to_complex(x, X, size);
    
    for (int i = 0; i < size; i++) {
        TEST_ASSERT_EQUAL_INT(1, creal(X[i]));
        TEST_ASSERT_EQUAL_INT(0, cimag(X[i]));
    }
}

void test_get_twiddle_factor() {
    double complex w;
    size_t N = 4;

    w = get_twiddle_factor(0, N);
    TEST_ASSERT_EQUAL_INT(1, creal(w));

    w = get_twiddle_factor(N/2, N);
    TEST_ASSERT_EQUAL_INT(-1, creal(w));
}

void test_dft() {
    dftt_config_t dftt_conf = {
        .total_samples = 4,
        .quiet_flag = 0,
    };
    double x[] = {1,0,0,1};
    double complex* X = calloc(dftt_conf.total_samples, sizeof(double complex));
    double X_real_expected[] = {2.0,1.0,0.0,1.0};
    double X_imag_expected[] = {0.0,1.0,0.0,-1.0};

    dft(&dftt_conf, X, x);
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_real_expected[i], creal(X[i]));
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_imag_expected[i], cimag(X[i]));
    }
}

void test_butterfly_dit() {
    dftt_config_t dftt_conf = {
        .total_samples = 4,
    };

    double complex* X = calloc(dftt_conf.total_samples, sizeof(double complex));

    double complex* X_copy = malloc(sizeof(double complex) * dftt_conf.total_samples);
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        X_copy[i] = creal(1);
    }

    butterfly_dit(X, X_copy, dftt_conf.total_samples);

    double X_real_expected[] = {4,0,0,0};
    double X_imag_expected[] = {0,0,0,0};
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_real_expected[i], creal(X[i]));
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_imag_expected[i], cimag(X[i]));
    }
}

void test_butterfly_dif() {
    dftt_config_t dftt_conf = {
        .total_samples = 4,
    };

    double complex* X = calloc(dftt_conf.total_samples, sizeof(double complex));

    double complex* X_copy = malloc(sizeof(double complex) * dftt_conf.total_samples);
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        X_copy[i] = creal(1);
    }

    butterfly_dif(X, X_copy, dftt_conf.total_samples);

    double X_real_expected[] = {4,0,0,0};
    double X_imag_expected[] = {0,0,0,0};
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_real_expected[i], creal(X[i]));
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_imag_expected[i], cimag(X[i]));
    }
}

void test_fft_radix2_dit() {
    dftt_config_t dftt_conf = {
       .total_samples = 4,
       .quiet_flag = 0,
    };

    double x[] = {1,0,0,1};
    double complex* X = calloc(dftt_conf.total_samples, sizeof(double complex));

    fft_radix2_dit(&dftt_conf, X, x);

    double X_real_expected[] = {2.0,1.0,0.0,1.0};
    double X_imag_expected[] = {0.0,1.0,0.0,-1.0};
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_real_expected[i], creal(X[i]));
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_imag_expected[i], cimag(X[i]));
    }
}

void test_fft_radix2_dif() {
    dftt_config_t dftt_conf = {
       .total_samples = 4,
       .quiet_flag = 0,
    };

    double data[] = {1,0,0,1};
    double* x = malloc(dftt_conf.total_samples * sizeof(double));
    memcpy(x, data, 4 * sizeof(double));
    double complex* X = calloc(dftt_conf.total_samples, sizeof(double complex));

    fft_radix2_dif(&dftt_conf, X, x);

    double X_real_expected[] = {2.0,1.0,0.0,1.0};
    double X_imag_expected[] = {0.0,1.0,0.0,-1.0};
    for (int i = 0; i < dftt_conf.total_samples; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_real_expected[i], creal(X[i]));
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_imag_expected[i], cimag(X[i]));
    }
}

void test_get_freq_bins() {
    size_t f_s = 48000;
    size_t size = 4;
    double* X_bins = malloc(sizeof(double) * size);

    get_freq_bins(X_bins, f_s, size);

    double X_bins_expected[] = {0,12000,24000,36000};
    for (int i = 0; i < size; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, X_bins_expected[i], X_bins[i]);
    }
}

void test_parse_complex_buff_to_RIB() {

}
 
void test_set_precision_format() {
    char format[9];

    set_precision_format(format, 5);

    TEST_ASSERT_EQUAL_STRING("%.5lf", format);
}

void test_get_datetime_string() {
    TEST_ASSERT_EQUAL_INT(12, strlen(get_datetime_string()));
}

void test_generate_file_name() {
    char ofile[MAX_STR];
    char ifile[MIN_STR] = ".\\test.wav";

    memset(ofile, '\0', MAX_STR);
    generate_file_name(ofile, ifile, 0);

    TEST_ASSERT_EQUAL_INT(0, strncmp(ofile, "adx-test-", 9));
    TEST_ASSERT_EQUAL_INT(0, strncmp(strrev(ofile), "txt.", 4));
    TEST_ASSERT_EQUAL_INT(strlen("adx-test-ddmmyyHHMMSS.txt"), strlen(ofile));

    strcpy(ofile, "test");
    generate_file_name(ofile, ifile, 0);
    TEST_ASSERT_EQUAL_STRING("test", ofile);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_set_defaults);
    RUN_TEST(test_output_help);
    RUN_TEST(test_get_options);
    RUN_TEST(test_select_output_format);
    RUN_TEST(test_read_input);
    RUN_TEST(test_read_audio_file_input);
    RUN_TEST(test_open_audio_file);
    RUN_TEST(test_get_audio_file_data);
    RUN_TEST(test_output_audio_file_info);
    RUN_TEST(test_get_sndfile_major_format);
    RUN_TEST(test_get_sndfile_subtype);
    RUN_TEST(test_check_csv_string);
    RUN_TEST(test_check_csv_extension);
    RUN_TEST(test_read_csv_string_file_input);
    RUN_TEST(test_open_csv_file);
    RUN_TEST(test_read_csv_file_data);
    RUN_TEST(test_get_data_from_string);
    RUN_TEST(test_output_input_info);
    RUN_TEST(test_select_windowing);
    RUN_TEST(test_windowing);
    RUN_TEST(test_mix2mono);
    RUN_TEST(test_select_fft_algo);
    RUN_TEST(test_timer);
    RUN_TEST(test_zero_pad_array);
    RUN_TEST(test_truncate_array);
    RUN_TEST(test_nextpow2);
    RUN_TEST(test_set_transform_size);
    RUN_TEST(test_index_bit_reversal);
    RUN_TEST(test_reorder_data_dit);
    RUN_TEST(test_reorder_data_dif);
    RUN_TEST(test_convert_to_complex);
    RUN_TEST(test_get_twiddle_factor);
    RUN_TEST(test_dft);
    RUN_TEST(test_butterfly_dit);
    RUN_TEST(test_butterfly_dif);
    RUN_TEST(test_fft_radix2_dit);
    RUN_TEST(test_fft_radix2_dif);
    RUN_TEST(test_get_freq_bins);

    RUN_TEST(test_set_precision_format);
    RUN_TEST(test_get_datetime_string);
    RUN_TEST(test_generate_file_name);

    return UNITY_END();
}
