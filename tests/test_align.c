#include "config.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "kseq.h"
KSEQ_INIT(int, read)

#include "parasail.h"
#include "parasail_internal.h"
#include "parasail_cpuid.h"
#include "blosum/blosum40.h"
#include "blosum/blosum45.h"
#include "blosum/blosum50.h"
#include "blosum/blosum62.h"
#include "blosum/blosum75.h"
#include "blosum/blosum80.h"
#include "blosum/blosum90.h"
#include "stats.h"
#include "timer.h"
#include "timer_real.h"

#define USE_TIMER_REAL 0

typedef struct blosum {
    const char * name;
    const int (*blosum)[24];
} blosum_t;

blosum_t blosums[] = {
    {"blosum40",blosum40},
    {"blosum45",blosum45},
    {"blosum50",blosum50},
    {"blosum62",blosum62},
    {"blosum75",blosum75},
    {"blosum80",blosum80},
    {"blosum90",blosum90},
    {"NULL",NULL},
};

static double pctf(double orig, double new)
{
    return orig / new;
}

#ifdef __MIC__
static const char *get_user_name()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return pw->pw_name;
    }
    return "";
}
#endif

static void print_array(
        const char * filename_,
        const int * const restrict array,
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len)
{
    int i;
    int j;
    FILE *f = NULL;
#ifdef __MIC__
    const char *username = get_user_name();
    char filename[4096] = {0};
    strcat(filename, "/tmp/");
    if (username[0] != '\0') {
        strcat(filename, username);
        strcat(filename, "/");
    }
    strcat(filename, filename_);
#else
    const char *filename = filename_;
#endif
    f = fopen(filename, "w");
    if (NULL == f) {
        printf("fopen(\"%s\") error: %s\n", filename, strerror(errno));
        exit(-1);
    }
    fprintf(f, " ");
    for (j=0; j<s2Len; ++j) {
        fprintf(f, "%4c", s2[j]);
    }
    fprintf(f, "\n");
    for (i=0; i<s1Len; ++i) {
        fprintf(f, "%c", s1[i]);
        for (j=0; j<s2Len; ++j) {
            fprintf(f, "%4d", array[i*s2Len + j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

static inline void parse_sequences(
        const char *filename,
        char ***strings_,
        unsigned long **sizes_,
        unsigned long *count_)
{
    FILE* fp;
    kseq_t *seq = NULL;
    int l = 0;
    char **strings = NULL;
    unsigned long *sizes = NULL;
    unsigned long count = 0;
    unsigned long memory = 1000;
    unsigned long i = 0;

    errno = 0;
    fp = fopen(filename, "r");
    if(fp == NULL) {
        perror("fopen");
        exit(1);
    }
    strings = malloc(sizeof(char*) * memory);
    sizes = malloc(sizeof(unsigned long) * memory);
    seq = kseq_init(fileno(fp));
    while ((l = kseq_read(seq)) >= 0) {
        errno = 0;
        strings[count] = strdup(seq->seq.s);
        if (NULL == strings[count]) {
            perror("strdup");
            exit(1);
        }
        sizes[count] = seq->seq.l;
        ++count;
        if (count >= memory) {
            char **new_strings = NULL;
            unsigned long *new_sizes = NULL;
            memory *= 2;
            errno = 0;
            new_strings = realloc(strings, sizeof(char*) * memory);
            if (NULL == new_strings) {
                perror("realloc");
                exit(1);
            }
            strings = new_strings;
            errno = 0;
            new_sizes = realloc(sizes, sizeof(unsigned long) * memory);
            if (NULL == new_sizes) {
                perror("realloc");
                exit(1);
            }
            sizes = new_sizes;
        }
    }
    kseq_destroy(seq);
    fclose(fp);

    *strings_ = strings;
    *sizes_ = sizes;
    *count_ = count;
}

typedef struct func {
    parasail_result_t* (*f)(const char * const restrict s1, const int s1Len,
                            const char * const restrict s2, const int s2Len,
                            const int open, const int gap,
                            const int matrix[24][24]);
    const char * alg;
    const char * type;
    const char * isa;
    const char * bits;
    const char * width;
    char is_table;
    char is_stats;
    char is_ref;
} func_t;


static inline int elem(func_t f) {
    int i_bits = atoi(f.bits);
    int i_width = atoi(f.width);
    return i_bits / i_width;
}


int main(int argc, char **argv)
{
    long seqA_index = LONG_MAX;
    long seqB_index = LONG_MAX;
    const char *seqA = NULL;
    const char *seqB = NULL;
    int lena = 0;
    int lenb = 0;
    int score = 0;
    int matches = 0;
    int similar = 0;
    int length = 0;
    unsigned long long timer_rdtsc = 0;
    unsigned long long timer_rdtsc_single = 0;
    double timer_rdtsc_ref_mean = 0.0;
    double timer_nsecs = 0.0;
    double timer_nsecs_single = 0.0;
#if USE_TIMER_REAL
    double timer_nsecs_ref_mean = 0.0;
#endif
    int limit = 2;
    int i = 0;
    int index = 0;
    func_t f;
    parasail_result_t *result = NULL;
    stats_t stats_rdtsc;
    stats_t stats_nsecs;
    int c = 0;
    char *filename = NULL;
    char **sequences = NULL;
    unsigned long *sizes = NULL;
    unsigned long seq_count = 0;
    unsigned long s = 0;
    char *endptr = NULL;
    char *blosumname = NULL;
    blosum_t b;
    int open = 10;
    int extend = 1;

    while ((c = getopt(argc, argv, "a:b:f:m:n:o:e:")) != -1) {
        switch (c) {
            case 'a':
                errno = 0;
                seqA_index = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol seqA_index");
                    fprintf(stderr, "invalid seqA index\n");
                    exit(1);
                }
                break;
            case 'b':
                errno = 0;
                seqB_index = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol seqB_index");
                    fprintf(stderr, "invalid seqB index\n");
                    exit(1);
                }
                break;
            case 'f':
                filename = optarg;
                break;
            case 'm':
                blosumname = optarg;
                break;
            case 'n':
                errno = 0;
                limit = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol limit");
                    exit(1);
                }
                break;
            case 'o':
                errno = 0;
                open = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol open");
                    exit(1);
                }
                break;
            case 'e':
                errno = 0;
                extend = strtol(optarg, &endptr, 10);
                if (errno) {
                    perror("strtol extend");
                    exit(1);
                }
                break;
            case '?':
                if (optopt == 'a'
                        || optopt == 'b'
                        || optopt == 'f'
                        || optopt == 'n') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n",
                            optopt);
                }
                else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n",
                            optopt);
                }
                else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                }
                exit(1);
            default:
                fprintf(stderr, "default case in getopt\n");
                exit(1);
        }
    }

    if (filename) {
        parse_sequences(filename, &sequences, &sizes, &seq_count);
    }
    else {
        fprintf(stderr, "no filename specified\n");
        exit(1);
    }

    /* select the blosum matrix */
    if (blosumname) {
        int index = 0;
        b = blosums[index++];
        while (b.blosum) {
            if (0 == strcmp(blosumname, b.name)) {
                break;
            }
            b = blosums[index++];
        }
        if (NULL == b.blosum) {
            fprintf(stderr, "Specified blosum matrix not found.\n");
            fprintf(stderr, "Choices are {"
                    "blosum62,"
                    "blosum40,"
                    "blosum45,"
                    "blosum50,"
                    "blosum62,"
                    "blosum75,"
                    "blosum80,"
                    "blosum90}\n");
            exit(1);
        }
    }
    else {
        fprintf(stderr, "No blosum matrix specified.\n");
        exit(1);
    }

    if (seqA_index == LONG_MAX) {
        fprintf(stderr, "seqA index not specified\n");
        exit(1);
    }

    if (seqB_index == LONG_MAX) {
        fprintf(stderr, "seqB index not specified\n");
        exit(1);
    }

    if (seqA_index >= seq_count) {
        fprintf(stderr, "seqA index out of bounds\n");
        exit(1);
    }

    if (seqB_index >= seq_count) {
        fprintf(stderr, "seqB index out of bounds\n");
        exit(1);
    }

    seqA = sequences[seqA_index];
    seqB = sequences[seqB_index];
    lena = strlen(seqA);
    lenb = strlen(seqB);

    func_t functions[] = {
        {nw,                        "nw", "orig",    "NA",    "32",  "32", 0, 0, 1},
        {nw_scan,                   "nw", "scan",    "NA",    "32",  "32", 0, 0, 0},
#if HAVE_SSE2
        {nw_scan_sse2_128_32,       "nw", "scan",    "sse2",  "128", "32", 0, 0, 0},
        {nw_scan_sse2_128_16,       "nw", "scan",    "sse2",  "128", "16", 0, 0, 0},
        {nw_scan_sse2_128_8,        "nw", "scan",    "sse2",  "128", "8",  0, 0, 0},
        {nw_diag_sse2_128_32,       "nw", "diag",    "sse2",  "128", "32", 0, 0, 0},
        {nw_diag_sse2_128_16,       "nw", "diag",    "sse2",  "128", "16", 0, 0, 0},
        {nw_diag_sse2_128_8,        "nw", "diag",    "sse2",  "128", "8",  0, 0, 0},
        {nw_striped_sse2_128_32,    "nw", "striped", "sse2",  "128", "32", 0, 0, 0},
        {nw_striped_sse2_128_16,    "nw", "striped", "sse2",  "128", "16", 0, 0, 0},
        {nw_striped_sse2_128_8,     "nw", "striped", "sse2",  "128", "8",  0, 0, 0},
#endif
#if HAVE_SSE41
        {nw_scan_sse41_128_32,      "nw", "scan",    "sse41", "128", "32", 0, 0, 0},
        {nw_scan_sse41_128_16,      "nw", "scan",    "sse41", "128", "16", 0, 0, 0},
        {nw_scan_sse41_128_8,       "nw", "scan",    "sse41", "128", "8",  0, 0, 0},
        {nw_diag_sse41_128_32,      "nw", "diag",    "sse41", "128", "32", 0, 0, 0},
        {nw_diag_sse41_128_16,      "nw", "diag",    "sse41", "128", "16", 0, 0, 0},
        {nw_diag_sse41_128_8,       "nw", "diag",    "sse41", "128", "8",  0, 0, 0},
        {nw_striped_sse41_128_32,   "nw", "striped", "sse41", "128", "32", 0, 0, 0},
        {nw_striped_sse41_128_16,   "nw", "striped", "sse41", "128", "16", 0, 0, 0},
        {nw_striped_sse41_128_8,    "nw", "striped", "sse41", "128", "8",  0, 0, 0},
#endif
#if HAVE_AVX2
        {nw_scan_avx2_256_32,       "nw", "scan",    "avx2",  "256", "32", 0, 0, 0},
        {nw_scan_avx2_256_16,       "nw", "scan",    "avx2",  "256", "16", 0, 0, 0},
        {nw_scan_avx2_256_8,        "nw", "scan",    "avx2",  "256", "8",  0, 0, 0},
        {nw_diag_avx2_256_32,       "nw", "diag",    "avx2",  "256", "32", 0, 0, 0},
        {nw_diag_avx2_256_16,       "nw", "diag",    "avx2",  "256", "16", 0, 0, 0},
        {nw_diag_avx2_256_8,        "nw", "diag",    "avx2",  "256", "8",  0, 0, 0},
        {nw_striped_avx2_256_32,    "nw", "striped", "avx2",  "256", "32", 0, 0, 0},
        {nw_striped_avx2_256_16,    "nw", "striped", "avx2",  "256", "16", 0, 0, 0},
        {nw_striped_avx2_256_8,     "nw", "striped", "avx2",  "256", "8",  0, 0, 0},
#endif
#if HAVE_KNC
        {nw_scan_knc_512_32,        "nw", "scan",    "knc",   "512", "32", 0, 0, 0},
        {nw_diag_knc_512_32,        "nw", "diag",    "knc",   "512", "32", 0, 0, 0},
        {nw_striped_knc_512_32,     "nw", "striped", "knc",   "512", "32", 0, 0, 0},
#endif

        {sg,                        "sg", "orig",    "NA",    "32",  "32", 0, 0, 1},
        {sg_scan,                   "sg", "scan",    "NA",    "32",  "32", 0, 0, 0},
#if HAVE_SSE2
        {sg_scan_sse2_128_32,       "sg", "scan",    "sse2",  "128", "32", 0, 0, 0},
        {sg_scan_sse2_128_16,       "sg", "scan",    "sse2",  "128", "16", 0, 0, 0},
        {sg_scan_sse2_128_8,        "sg", "scan",    "sse2",  "128", "8",  0, 0, 0},
        {sg_diag_sse2_128_32,       "sg", "diag",    "sse2",  "128", "32", 0, 0, 0},
        {sg_diag_sse2_128_16,       "sg", "diag",    "sse2",  "128", "16", 0, 0, 0},
        {sg_diag_sse2_128_8,        "sg", "diag",    "sse2",  "128", "8",  0, 0, 0},
        {sg_striped_sse2_128_32,    "sg", "striped", "sse2",  "128", "32", 0, 0, 0},
        {sg_striped_sse2_128_16,    "sg", "striped", "sse2",  "128", "16", 0, 0, 0},
        {sg_striped_sse2_128_8,     "sg", "striped", "sse2",  "128", "8",  0, 0, 0},
#endif
#if HAVE_SSE41
        {sg_scan_sse41_128_32,      "sg", "scan",    "sse41", "128", "32", 0, 0, 0},
        {sg_scan_sse41_128_16,      "sg", "scan",    "sse41", "128", "16", 0, 0, 0},
        {sg_scan_sse41_128_8,       "sg", "scan",    "sse41", "128", "8",  0, 0, 0},
        {sg_diag_sse41_128_32,      "sg", "diag",    "sse41", "128", "32", 0, 0, 0},
        {sg_diag_sse41_128_16,      "sg", "diag",    "sse41", "128", "16", 0, 0, 0},
        {sg_diag_sse41_128_8,       "sg", "diag",    "sse41", "128", "8",  0, 0, 0},
        {sg_striped_sse41_128_32,   "sg", "striped", "sse41", "128", "32", 0, 0, 0},
        {sg_striped_sse41_128_16,   "sg", "striped", "sse41", "128", "16", 0, 0, 0},
        {sg_striped_sse41_128_8,    "sg", "striped", "sse41", "128", "8",  0, 0, 0},
#endif
#if HAVE_AVX2
        {sg_scan_avx2_256_32,       "sg", "scan",    "avx2",  "256", "32", 0, 0, 0},
        {sg_scan_avx2_256_16,       "sg", "scan",    "avx2",  "256", "16", 0, 0, 0},
        {sg_scan_avx2_256_8,        "sg", "scan",    "avx2",  "256", "8",  0, 0, 0},
        {sg_diag_avx2_256_32,       "sg", "diag",    "avx2",  "256", "32", 0, 0, 0},
        {sg_diag_avx2_256_16,       "sg", "diag",    "avx2",  "256", "16", 0, 0, 0},
        {sg_diag_avx2_256_8,        "sg", "diag",    "avx2",  "256", "8",  0, 0, 0},
        {sg_striped_avx2_256_32,    "sg", "striped", "avx2",  "256", "32", 0, 0, 0},
        {sg_striped_avx2_256_16,    "sg", "striped", "avx2",  "256", "16", 0, 0, 0},
        {sg_striped_avx2_256_8,     "sg", "striped", "avx2",  "256", "8",  0, 0, 0},
#endif
#if HAVE_KNC
        {sg_scan_knc_512_32,        "sg", "scan",    "knc",   "512", "32", 0, 0, 0},
        {sg_diag_knc_512_32,        "sg", "diag",    "knc",   "512", "32", 0, 0, 0},
        {sg_striped_knc_512_32,     "sg", "striped", "knc",   "512", "32", 0, 0, 0},
#endif

        {sw,                        "sw", "orig",    "NA",    "32",  "32", 0, 0, 1},
        {sw_scan,                   "sw", "scan",    "NA",    "32",  "32", 0, 0, 0},
#if HAVE_SSE2
        {sw_scan_sse2_128_32,       "sw", "scan",    "sse2",  "128", "32", 0, 0, 0},
        {sw_scan_sse2_128_16,       "sw", "scan",    "sse2",  "128", "16", 0, 0, 0},
        {sw_scan_sse2_128_8,        "sw", "scan",    "sse2",  "128", "8",  0, 0, 0},
        {sw_diag_sse2_128_32,       "sw", "diag",    "sse2",  "128", "32", 0, 0, 0},
        {sw_diag_sse2_128_16,       "sw", "diag",    "sse2",  "128", "16", 0, 0, 0},
        {sw_diag_sse2_128_8,        "sw", "diag",    "sse2",  "128", "8",  0, 0, 0},
        {sw_striped_sse2_128_32,    "sw", "striped", "sse2",  "128", "32", 0, 0, 0},
        {sw_striped_sse2_128_16,    "sw", "striped", "sse2",  "128", "16", 0, 0, 0},
        {sw_striped_sse2_128_8,     "sw", "striped", "sse2",  "128", "8",  0, 0, 0},
#endif
#if HAVE_SSE41
        {sw_scan_sse41_128_32,      "sw", "scan",    "sse41", "128", "32", 0, 0, 0},
        {sw_scan_sse41_128_16,      "sw", "scan",    "sse41", "128", "16", 0, 0, 0},
        {sw_scan_sse41_128_8,       "sw", "scan",    "sse41", "128", "8",  0, 0, 0},
        {sw_diag_sse41_128_32,      "sw", "diag",    "sse41", "128", "32", 0, 0, 0},
        {sw_diag_sse41_128_16,      "sw", "diag",    "sse41", "128", "16", 0, 0, 0},
        {sw_diag_sse41_128_8,       "sw", "diag",    "sse41", "128", "8",  0, 0, 0},
        {sw_striped_sse41_128_32,   "sw", "striped", "sse41", "128", "32", 0, 0, 0},
        {sw_striped_sse41_128_16,   "sw", "striped", "sse41", "128", "16", 0, 0, 0},
        {sw_striped_sse41_128_8,    "sw", "striped", "sse41", "128", "8",  0, 0, 0},
#endif
#if HAVE_AVX2
        {sw_scan_avx2_256_32,       "sw", "scan",    "avx2",  "256", "32", 0, 0, 0},
        {sw_scan_avx2_256_16,       "sw", "scan",    "avx2",  "256", "16", 0, 0, 0},
        {sw_scan_avx2_256_8,        "sw", "scan",    "avx2",  "256", "8",  0, 0, 0},
        {sw_diag_avx2_256_32,       "sw", "diag",    "avx2",  "256", "32", 0, 0, 0},
        {sw_diag_avx2_256_16,       "sw", "diag",    "avx2",  "256", "16", 0, 0, 0},
        {sw_diag_avx2_256_8,        "sw", "diag",    "avx2",  "256", "8",  0, 0, 0},
        {sw_striped_avx2_256_32,    "sw", "striped", "avx2",  "256", "32", 0, 0, 0},
        {sw_striped_avx2_256_16,    "sw", "striped", "avx2",  "256", "16", 0, 0, 0},
        {sw_striped_avx2_256_8,     "sw", "striped", "avx2",  "256", "8",  0, 0, 0},
#endif
#if HAVE_KNC
        {sw_scan_knc_512_32,        "sw", "scan",    "knc",   "512", "32", 0, 0, 0},
        {sw_diag_knc_512_32,        "sw", "diag",    "knc",   "512", "32", 0, 0, 0},
        {sw_striped_knc_512_32,     "sw", "striped", "knc",   "512", "32", 0, 0, 0},
#endif
                                   
        {nw_table,                     "nw", "orig",    "NA",    "32",  "32", 1, 0, 1},
        {nw_table_scan,                "nw", "scan",    "NA",    "32",  "32", 1, 0, 0},
#if HAVE_SSE2
        {nw_table_scan_sse2_128_32,    "nw", "scan",    "sse2",  "128", "32", 1, 0, 0},
        {nw_table_scan_sse2_128_16,    "nw", "scan",    "sse2",  "128", "16", 1, 0, 0},
        {nw_table_scan_sse2_128_8,     "nw", "scan",    "sse2",  "128", "8",  1, 0, 0},
        {nw_table_diag_sse2_128_32,    "nw", "diag",    "sse2",  "128", "32", 1, 0, 0},
        {nw_table_diag_sse2_128_16,    "nw", "diag",    "sse2",  "128", "16", 1, 0, 0},
        {nw_table_diag_sse2_128_8,     "nw", "diag",    "sse2",  "128", "8",  1, 0, 0},
        {nw_table_striped_sse2_128_32, "nw", "striped", "sse2",  "128", "32", 1, 0, 0},
        {nw_table_striped_sse2_128_16, "nw", "striped", "sse2",  "128", "16", 1, 0, 0},
        {nw_table_striped_sse2_128_8,  "nw", "striped", "sse2",  "128", "8",  1, 0, 0},
#endif
#if HAVE_SSE41
        {nw_table_scan_sse41_128_32,    "nw", "scan",    "sse41", "128", "32", 1, 0, 0},
        {nw_table_scan_sse41_128_16,    "nw", "scan",    "sse41", "128", "16", 1, 0, 0},
        {nw_table_scan_sse41_128_8,     "nw", "scan",    "sse41", "128", "8",  1, 0, 0},
        {nw_table_diag_sse41_128_32,    "nw", "diag",    "sse41", "128", "32", 1, 0, 0},
        {nw_table_diag_sse41_128_16,    "nw", "diag",    "sse41", "128", "16", 1, 0, 0},
        {nw_table_diag_sse41_128_8,     "nw", "diag",    "sse41", "128", "8",  1, 0, 0},
        {nw_table_striped_sse41_128_32, "nw", "striped", "sse41", "128", "32", 1, 0, 0},
        {nw_table_striped_sse41_128_16, "nw", "striped", "sse41", "128", "16", 1, 0, 0},
        {nw_table_striped_sse41_128_8,  "nw", "striped", "sse41", "128", "8",  1, 0, 0},
#endif
#if HAVE_AVX2
        {nw_table_scan_avx2_256_32,       "nw", "scan",    "avx2",  "256", "32", 1, 0, 0},
        {nw_table_scan_avx2_256_16,       "nw", "scan",    "avx2",  "256", "16", 1, 0, 0},
        {nw_table_scan_avx2_256_8,        "nw", "scan",    "avx2",  "256", "8",  1, 0, 0},
        {nw_table_diag_avx2_256_32,       "nw", "diag",    "avx2",  "256", "32", 1, 0, 0},
        {nw_table_diag_avx2_256_16,       "nw", "diag",    "avx2",  "256", "16", 1, 0, 0},
        {nw_table_diag_avx2_256_8,        "nw", "diag",    "avx2",  "256", "8",  1, 0, 0},
        {nw_table_striped_avx2_256_32,    "nw", "striped", "avx2",  "256", "32", 1, 0, 0},
        {nw_table_striped_avx2_256_16,    "nw", "striped", "avx2",  "256", "16", 1, 0, 0},
        {nw_table_striped_avx2_256_8,     "nw", "striped", "avx2",  "256", "8",  1, 0, 0},
#endif
#if HAVE_KNC
        {nw_table_scan_knc_512_32,        "nw", "scan",    "knc",   "512", "32", 1, 0, 0},
        {nw_table_diag_knc_512_32,        "nw", "diag",    "knc",   "512", "32", 1, 0, 0},
        {nw_table_striped_knc_512_32,     "nw", "striped", "knc",   "512", "32", 1, 0, 0},
#endif

        {sg_table,                     "sg", "orig",    "NA",   "32",  "32", 1, 0, 1},
        {sg_table_scan,                "sg", "scan",    "NA",   "32",  "32", 1, 0, 0},
#if HAVE_SSE2
        {sg_table_scan_sse2_128_32,    "sg", "scan",    "sse2", "128", "32", 1, 0, 0},
        {sg_table_scan_sse2_128_16,    "sg", "scan",    "sse2", "128", "16", 1, 0, 0},
        {sg_table_scan_sse2_128_8,     "sg", "scan",    "sse2", "128", "8",  1, 0, 0},
        {sg_table_diag_sse2_128_32,    "sg", "diag",    "sse2", "128", "32", 1, 0, 0},
        {sg_table_diag_sse2_128_16,    "sg", "diag",    "sse2", "128", "16", 1, 0, 0},
        {sg_table_diag_sse2_128_8,     "sg", "diag",    "sse2", "128", "8",  1, 0, 0},
        {sg_table_striped_sse2_128_32, "sg", "striped", "sse2", "128", "32", 1, 0, 0},
        {sg_table_striped_sse2_128_16, "sg", "striped", "sse2", "128", "16", 1, 0, 0},
        {sg_table_striped_sse2_128_8,  "sg", "striped", "sse2", "128", "8",  1, 0, 0},
#endif
#if HAVE_SSE41
        {sg_table_scan_sse41_128_32,   "sg", "scan",    "sse41", "128", "32", 1, 0, 0},
        {sg_table_scan_sse41_128_16,   "sg", "scan",    "sse41", "128", "16", 1, 0, 0},
        {sg_table_scan_sse41_128_8,    "sg", "scan",    "sse41", "128", "8",  1, 0, 0},
        {sg_table_diag_sse41_128_32,   "sg", "diag",    "sse41", "128", "32", 1, 0, 0},
        {sg_table_diag_sse41_128_16,   "sg", "diag",    "sse41", "128", "16", 1, 0, 0},
        {sg_table_diag_sse41_128_8,    "sg", "diag",    "sse41", "128", "8",  1, 0, 0},
        {sg_table_striped_sse41_128_32,"sg", "striped", "sse41", "128", "32", 1, 0, 0},
        {sg_table_striped_sse41_128_16,"sg", "striped", "sse41", "128", "16", 1, 0, 0},
        {sg_table_striped_sse41_128_8, "sg", "striped", "sse41", "128", "8",  1, 0, 0},
#endif
#if HAVE_AVX2
        {sg_table_scan_avx2_256_32,       "sg", "scan",    "avx2",  "256", "32", 1, 0, 0},
        {sg_table_scan_avx2_256_16,       "sg", "scan",    "avx2",  "256", "16", 1, 0, 0},
        {sg_table_scan_avx2_256_8,        "sg", "scan",    "avx2",  "256", "8",  1, 0, 0},
        {sg_table_diag_avx2_256_32,       "sg", "diag",    "avx2",  "256", "32", 1, 0, 0},
        {sg_table_diag_avx2_256_16,       "sg", "diag",    "avx2",  "256", "16", 1, 0, 0},
        {sg_table_diag_avx2_256_8,        "sg", "diag",    "avx2",  "256", "8",  1, 0, 0},
        {sg_table_striped_avx2_256_32,    "sg", "striped", "avx2",  "256", "32", 1, 0, 0},
        {sg_table_striped_avx2_256_16,    "sg", "striped", "avx2",  "256", "16", 1, 0, 0},
        {sg_table_striped_avx2_256_8,     "sg", "striped", "avx2",  "256", "8",  1, 0, 0},
#endif
#if HAVE_KNC
        {sg_table_scan_knc_512_32,        "sg", "scan",    "knc",   "512", "32", 1, 0, 0},
        {sg_table_diag_knc_512_32,        "sg", "diag",    "knc",   "512", "32", 1, 0, 0},
        {sg_table_striped_knc_512_32,     "sg", "striped", "knc",   "512", "32", 1, 0, 0},
#endif

        {sw_table,                     "sw", "orig",    "NA",   "32",  "32", 1, 0, 1},
        {sw_table_scan,                "sw", "scan",    "NA",   "32",  "32", 1, 0, 0},
#if HAVE_SSE2
        {sw_table_scan_sse2_128_32,    "sw", "scan",    "sse2", "128", "32", 1, 0, 0},
        {sw_table_scan_sse2_128_16,    "sw", "scan",    "sse2", "128", "16", 1, 0, 0},
        {sw_table_scan_sse2_128_8,     "sw", "scan",    "sse2", "128", "8",  1, 0, 0},
        {sw_table_diag_sse2_128_32,    "sw", "diag",    "sse2", "128", "32", 1, 0, 0},
        {sw_table_diag_sse2_128_16,    "sw", "diag",    "sse2", "128", "16", 1, 0, 0},
        {sw_table_diag_sse2_128_8,     "sw", "diag",    "sse2", "128", "8",  1, 0, 0},
        {sw_table_striped_sse2_128_32, "sw", "striped", "sse2", "128", "32", 1, 0, 0},
        {sw_table_striped_sse2_128_16, "sw", "striped", "sse2", "128", "16", 1, 0, 0},
        {sw_table_striped_sse2_128_8,  "sw", "striped", "sse2", "128", "8",  1, 0, 0},
#endif
#if HAVE_SSE41
        {sw_table_scan_sse41_128_32,    "sw", "scan",    "sse41", "128", "32", 1, 0, 0},
        {sw_table_scan_sse41_128_16,    "sw", "scan",    "sse41", "128", "16", 1, 0, 0},
        {sw_table_scan_sse41_128_8,     "sw", "scan",    "sse41", "128", "8",  1, 0, 0},
        {sw_table_diag_sse41_128_32,    "sw", "diag",    "sse41", "128", "32", 1, 0, 0},
        {sw_table_diag_sse41_128_16,    "sw", "diag",    "sse41", "128", "16", 1, 0, 0},
        {sw_table_diag_sse41_128_8,     "sw", "diag",    "sse41", "128", "8",  1, 0, 0},
        {sw_table_striped_sse41_128_32, "sw", "striped", "sse41", "128", "32", 1, 0, 0},
        {sw_table_striped_sse41_128_16, "sw", "striped", "sse41", "128", "16", 1, 0, 0},
        {sw_table_striped_sse41_128_8,  "sw", "striped", "sse41", "128", "8",  1, 0, 0},
#endif
#if HAVE_AVX2
        {sw_table_scan_avx2_256_32,     "sw", "scan",    "avx2",  "256", "32", 1, 0, 0},
        {sw_table_scan_avx2_256_16,     "sw", "scan",    "avx2",  "256", "16", 1, 0, 0},
        {sw_table_scan_avx2_256_8,      "sw", "scan",    "avx2",  "256", "8",  1, 0, 0},
        {sw_table_diag_avx2_256_32,     "sw", "diag",    "avx2",  "256", "32", 1, 0, 0},
        {sw_table_diag_avx2_256_16,     "sw", "diag",    "avx2",  "256", "16", 1, 0, 0},
        {sw_table_diag_avx2_256_8,      "sw", "diag",    "avx2",  "256", "8",  1, 0, 0},
        {sw_table_striped_avx2_256_32,  "sw", "striped", "avx2",  "256", "32", 1, 0, 0},
        {sw_table_striped_avx2_256_16,  "sw", "striped", "avx2",  "256", "16", 1, 0, 0},
        {sw_table_striped_avx2_256_8,   "sw", "striped", "avx2",  "256", "8",  1, 0, 0},
#endif
#if HAVE_KNC
        {sw_table_scan_knc_512_32,        "sw", "scan",    "knc",   "512", "32", 1, 0, 0},
        {sw_table_diag_knc_512_32,        "sw", "diag",    "knc",   "512", "32", 1, 0, 0},
        {sw_table_striped_knc_512_32,     "sw", "striped", "knc",   "512", "32", 1, 0, 0},
#endif

        {nw_stats,                        "nw_stats", "orig",    "NA",    "32",  "32", 0, 1, 1},
        {nw_stats_scan,                   "nw_stats", "scan",    "NA",    "32",  "32", 0, 1, 0},
#if HAVE_SSE2
        {nw_stats_scan_sse2_128_32,       "nw_stats", "scan",    "sse2",  "128", "32", 0, 1, 0},
        {nw_stats_scan_sse2_128_16,       "nw_stats", "scan",    "sse2",  "128", "16", 0, 1, 0},
        {nw_stats_scan_sse2_128_8,        "nw_stats", "scan",    "sse2",  "128", "8",  0, 1, 0},
        {nw_stats_diag_sse2_128_32,       "nw_stats", "diag",    "sse2",  "128", "32", 0, 1, 0},
        {nw_stats_diag_sse2_128_16,       "nw_stats", "diag",    "sse2",  "128", "16", 0, 1, 0},
        {nw_stats_diag_sse2_128_8,        "nw_stats", "diag",    "sse2",  "128", "8",  0, 1, 0},
        {nw_stats_striped_sse2_128_32,    "nw_stats", "striped", "sse2",  "128", "32", 0, 1, 0},
        {nw_stats_striped_sse2_128_16,    "nw_stats", "striped", "sse2",  "128", "16", 0, 1, 0},
        {nw_stats_striped_sse2_128_8,     "nw_stats", "striped", "sse2",  "128", "8",  0, 1, 0},
#endif
#if HAVE_SSE41
        {nw_stats_scan_sse41_128_32,      "nw_stats", "scan",    "sse41", "128", "32", 0, 1, 0},
        {nw_stats_scan_sse41_128_16,      "nw_stats", "scan",    "sse41", "128", "16", 0, 1, 0},
        {nw_stats_scan_sse41_128_8,       "nw_stats", "scan",    "sse41", "128", "8",  0, 1, 0},
        {nw_stats_diag_sse41_128_32,      "nw_stats", "diag",    "sse41", "128", "32", 0, 1, 0},
        {nw_stats_diag_sse41_128_16,      "nw_stats", "diag",    "sse41", "128", "16", 0, 1, 0},
        {nw_stats_diag_sse41_128_8,       "nw_stats", "diag",    "sse41", "128", "8",  0, 1, 0},
        {nw_stats_striped_sse41_128_32,   "nw_stats", "striped", "sse41", "128", "32", 0, 1, 0},
        {nw_stats_striped_sse41_128_16,   "nw_stats", "striped", "sse41", "128", "16", 0, 1, 0},
        {nw_stats_striped_sse41_128_8,    "nw_stats", "striped", "sse41", "128", "8",  0, 1, 0},
#endif
#if HAVE_AVX2
        {nw_stats_scan_avx2_256_32,      "nw_stats", "scan",    "avx2", "256", "32", 0, 1, 0},
        {nw_stats_scan_avx2_256_16,      "nw_stats", "scan",    "avx2", "256", "16", 0, 1, 0},
        {nw_stats_scan_avx2_256_8,       "nw_stats", "scan",    "avx2", "256", "8",  0, 1, 0},
        {nw_stats_diag_avx2_256_32,      "nw_stats", "diag",    "avx2", "256", "32", 0, 1, 0},
        {nw_stats_diag_avx2_256_16,      "nw_stats", "diag",    "avx2", "256", "16", 0, 1, 0},
        {nw_stats_diag_avx2_256_8,       "nw_stats", "diag",    "avx2", "256", "8",  0, 1, 0},
        {nw_stats_striped_avx2_256_32,   "nw_stats", "striped", "avx2", "256", "32", 0, 1, 0},
        {nw_stats_striped_avx2_256_16,   "nw_stats", "striped", "avx2", "256", "16", 0, 1, 0},
        {nw_stats_striped_avx2_256_8,    "nw_stats", "striped", "avx2", "256", "8",  0, 1, 0},
#endif
#if HAVE_KNC
        {nw_stats_scan_knc_512_32,        "nw_stats", "scan",    "knc",   "512", "32", 0, 1, 0},
        {nw_stats_diag_knc_512_32,        "nw_stats", "diag",    "knc",   "512", "32", 0, 1, 0},
        {nw_stats_striped_knc_512_32,     "nw_stats", "striped", "knc",   "512", "32", 0, 1, 0},
#endif
                                   
        {sg_stats,                        "sg_stats", "orig",    "NA",    "32",  "32", 0, 1, 1},
        {sg_stats_scan,                   "sg_stats", "scan",    "NA",    "32",  "32", 0, 1, 0},
#if HAVE_SSE2
        {sg_stats_scan_sse2_128_32,       "sg_stats", "scan",    "sse2",  "128", "32", 0, 1, 0},
        {sg_stats_scan_sse2_128_16,       "sg_stats", "scan",    "sse2",  "128", "16", 0, 1, 0},
        {sg_stats_scan_sse2_128_8,        "sg_stats", "scan",    "sse2",  "128", "8",  0, 1, 0},
        {sg_stats_diag_sse2_128_32,       "sg_stats", "diag",    "sse2",  "128", "32", 0, 1, 0},
        {sg_stats_diag_sse2_128_16,       "sg_stats", "diag",    "sse2",  "128", "16", 0, 1, 0},
        {sg_stats_diag_sse2_128_8,        "sg_stats", "diag",    "sse2",  "128", "8",  0, 1, 0},
        {sg_stats_striped_sse2_128_32,    "sg_stats", "striped", "sse2",  "128", "32", 0, 1, 0},
        {sg_stats_striped_sse2_128_16,    "sg_stats", "striped", "sse2",  "128", "16", 0, 1, 0},
        {sg_stats_striped_sse2_128_8,     "sg_stats", "striped", "sse2",  "128", "8",  0, 1, 0},
#endif
#if HAVE_SSE41
        {sg_stats_scan_sse41_128_32,      "sg_stats", "scan",    "sse41", "128", "32", 0, 1, 0},
        {sg_stats_scan_sse41_128_16,      "sg_stats", "scan",    "sse41", "128", "16", 0, 1, 0},
        {sg_stats_scan_sse41_128_8,       "sg_stats", "scan",    "sse41", "128", "8",  0, 1, 0},
        {sg_stats_diag_sse41_128_32,      "sg_stats", "diag",    "sse41", "128", "32", 0, 1, 0},
        {sg_stats_diag_sse41_128_16,      "sg_stats", "diag",    "sse41", "128", "16", 0, 1, 0},
        {sg_stats_diag_sse41_128_8,       "sg_stats", "diag",    "sse41", "128", "8",  0, 1, 0},
        {sg_stats_striped_sse41_128_32,   "sg_stats", "striped", "sse41", "128", "32", 0, 1, 0},
        {sg_stats_striped_sse41_128_16,   "sg_stats", "striped", "sse41", "128", "16", 0, 1, 0},
        {sg_stats_striped_sse41_128_8,    "sg_stats", "striped", "sse41", "128", "8",  0, 1, 0},
#endif
#if HAVE_AVX2
        {sg_stats_scan_avx2_256_32,       "sg_stats", "scan",    "avx2",  "256", "32", 0, 1, 0},
        {sg_stats_scan_avx2_256_16,       "sg_stats", "scan",    "avx2",  "256", "16", 0, 1, 0},
        {sg_stats_scan_avx2_256_8,        "sg_stats", "scan",    "avx2",  "256", "8",  0, 1, 0},
        {sg_stats_diag_avx2_256_32,       "sg_stats", "diag",    "avx2",  "256", "32", 0, 1, 0},
        {sg_stats_diag_avx2_256_16,       "sg_stats", "diag",    "avx2",  "256", "16", 0, 1, 0},
        {sg_stats_diag_avx2_256_8,        "sg_stats", "diag",    "avx2",  "256", "8",  0, 1, 0},
        {sg_stats_striped_avx2_256_32,    "sg_stats", "striped", "avx2",  "256", "32", 0, 1, 0},
        {sg_stats_striped_avx2_256_16,    "sg_stats", "striped", "avx2",  "256", "16", 0, 1, 0},
        {sg_stats_striped_avx2_256_8,     "sg_stats", "striped", "avx2",  "256", "8",  0, 1, 0},
#endif
#if HAVE_KNC
        {sg_stats_scan_knc_512_32,        "sg_stats", "scan",    "knc",   "512", "32", 0, 1, 0},
        {sg_stats_diag_knc_512_32,        "sg_stats", "diag",    "knc",   "512", "32", 0, 1, 0},
        {sg_stats_striped_knc_512_32,     "sg_stats", "striped", "knc",   "512", "32", 0, 1, 0},
#endif
                                   
        {sw_stats,                        "sw_stats", "orig",    "NA",    "32",  "32", 0, 1, 1},
        {sw_stats_scan,                   "sw_stats", "scan",    "NA",    "32",  "32", 0, 1, 0},
#if HAVE_SSE2
        {sw_stats_scan_sse2_128_32,       "sw_stats", "scan",    "sse2",  "128", "32", 0, 1, 0},
        {sw_stats_scan_sse2_128_16,       "sw_stats", "scan",    "sse2",  "128", "16", 0, 1, 0},
        {sw_stats_scan_sse2_128_8,        "sw_stats", "scan",    "sse2",  "128", "8",  0, 1, 0},
        {sw_stats_diag_sse2_128_32,       "sw_stats", "diag",    "sse2",  "128", "32", 0, 1, 0},
        {sw_stats_diag_sse2_128_16,       "sw_stats", "diag",    "sse2",  "128", "16", 0, 1, 0},
        {sw_stats_diag_sse2_128_8,        "sw_stats", "diag",    "sse2",  "128", "8",  0, 1, 0},
        {sw_stats_striped_sse2_128_32,    "sw_stats", "striped", "sse2",  "128", "32", 0, 1, 0},
        {sw_stats_striped_sse2_128_16,    "sw_stats", "striped", "sse2",  "128", "16", 0, 1, 0},
        {sw_stats_striped_sse2_128_8,     "sw_stats", "striped", "sse2",  "128", "8",  0, 1, 0},
#endif
#if HAVE_SSE41
        {sw_stats_scan_sse41_128_32,      "sw_stats", "scan",    "sse41", "128", "32", 0, 1, 0},
        {sw_stats_scan_sse41_128_16,      "sw_stats", "scan",    "sse41", "128", "16", 0, 1, 0},
        {sw_stats_scan_sse41_128_8,       "sw_stats", "scan",    "sse41", "128", "8",  0, 1, 0},
        {sw_stats_diag_sse41_128_32,      "sw_stats", "diag",    "sse41", "128", "32", 0, 1, 0},
        {sw_stats_diag_sse41_128_16,      "sw_stats", "diag",    "sse41", "128", "16", 0, 1, 0},
        {sw_stats_diag_sse41_128_8,       "sw_stats", "diag",    "sse41", "128", "8",  0, 1, 0},
        {sw_stats_striped_sse41_128_32,   "sw_stats", "striped", "sse41", "128", "32", 0, 1, 0},
        {sw_stats_striped_sse41_128_16,   "sw_stats", "striped", "sse41", "128", "16", 0, 1, 0},
        {sw_stats_striped_sse41_128_8,    "sw_stats", "striped", "sse41", "128", "8",  0, 1, 0},
#endif
#if HAVE_AVX2
        {sw_stats_scan_avx2_256_32,      "sw_stats", "scan",    "avx2", "256", "32", 0, 1, 0},
        {sw_stats_scan_avx2_256_16,      "sw_stats", "scan",    "avx2", "256", "16", 0, 1, 0},
        {sw_stats_scan_avx2_256_8,       "sw_stats", "scan",    "avx2", "256", "8",  0, 1, 0},
        {sw_stats_diag_avx2_256_32,      "sw_stats", "diag",    "avx2", "256", "32", 0, 1, 0},
        {sw_stats_diag_avx2_256_16,      "sw_stats", "diag",    "avx2", "256", "16", 0, 1, 0},
        {sw_stats_diag_avx2_256_8,       "sw_stats", "diag",    "avx2", "256", "8",  0, 1, 0},
        {sw_stats_striped_avx2_256_32,   "sw_stats", "striped", "avx2", "256", "32", 0, 1, 0},
        {sw_stats_striped_avx2_256_16,   "sw_stats", "striped", "avx2", "256", "16", 0, 1, 0},
        {sw_stats_striped_avx2_256_8,    "sw_stats", "striped", "avx2", "256", "8",  0, 1, 0},
#endif
#if HAVE_KNC
        {sw_stats_scan_knc_512_32,        "sw_stats", "scan",    "knc",   "512", "32", 0, 1, 0},
        {sw_stats_diag_knc_512_32,        "sw_stats", "diag",    "knc",   "512", "32", 0, 1, 0},
        {sw_stats_striped_knc_512_32,     "sw_stats", "striped", "knc",   "512", "32", 0, 1, 0},
#endif
                                   
        {nw_stats_table,                        "nw_stats", "orig",    "NA",    "32",  "32", 1, 1, 1},
        {nw_stats_table_scan,                   "nw_stats", "scan",    "NA",    "32",  "32", 1, 1, 0},
#if HAVE_SSE2
        {nw_stats_table_scan_sse2_128_32,       "nw_stats", "scan",    "sse2",  "128", "32", 1, 1, 0},
        {nw_stats_table_scan_sse2_128_16,       "nw_stats", "scan",    "sse2",  "128", "16", 1, 1, 0},
        {nw_stats_table_scan_sse2_128_8,        "nw_stats", "scan",    "sse2",  "128", "8",  1, 1, 0},
        {nw_stats_table_diag_sse2_128_32,       "nw_stats", "diag",    "sse2",  "128", "32", 1, 1, 0},
        {nw_stats_table_diag_sse2_128_16,       "nw_stats", "diag",    "sse2",  "128", "16", 1, 1, 0},
        {nw_stats_table_diag_sse2_128_8,        "nw_stats", "diag",    "sse2",  "128", "8",  1, 1, 0},
        {nw_stats_table_striped_sse2_128_32,    "nw_stats", "striped", "sse2",  "128", "32", 1, 1, 0},
        {nw_stats_table_striped_sse2_128_16,    "nw_stats", "striped", "sse2",  "128", "16", 1, 1, 0},
        {nw_stats_table_striped_sse2_128_8,     "nw_stats", "striped", "sse2",  "128", "8",  1, 1, 0},
#endif
#if HAVE_SSE41
        {nw_stats_table_scan_sse41_128_32,      "nw_stats", "scan",    "sse41", "128", "32", 1, 1, 0},
        {nw_stats_table_scan_sse41_128_16,      "nw_stats", "scan",    "sse41", "128", "16", 1, 1, 0},
        {nw_stats_table_scan_sse41_128_8,       "nw_stats", "scan",    "sse41", "128", "8",  1, 1, 0},
        {nw_stats_table_diag_sse41_128_32,      "nw_stats", "diag",    "sse41", "128", "32", 1, 1, 0},
        {nw_stats_table_diag_sse41_128_16,      "nw_stats", "diag",    "sse41", "128", "16", 1, 1, 0},
        {nw_stats_table_diag_sse41_128_8,       "nw_stats", "diag",    "sse41", "128", "8",  1, 1, 0},
        {nw_stats_table_striped_sse41_128_32,   "nw_stats", "striped", "sse41", "128", "32", 1, 1, 0},
        {nw_stats_table_striped_sse41_128_16,   "nw_stats", "striped", "sse41", "128", "16", 1, 1, 0},
        {nw_stats_table_striped_sse41_128_8,    "nw_stats", "striped", "sse41", "128", "8",  1, 1, 0},
#endif
#if HAVE_AVX2
        {nw_stats_table_scan_avx2_256_32,       "nw_stats", "scan",    "avx2",  "256", "32", 1, 1, 0},
        {nw_stats_table_scan_avx2_256_16,       "nw_stats", "scan",    "avx2",  "256", "16", 1, 1, 0},
        {nw_stats_table_scan_avx2_256_8,        "nw_stats", "scan",    "avx2",  "256", "8",  1, 1, 0},
        {nw_stats_table_diag_avx2_256_32,       "nw_stats", "diag",    "avx2",  "256", "32", 1, 1, 0},
        {nw_stats_table_diag_avx2_256_16,       "nw_stats", "diag",    "avx2",  "256", "16", 1, 1, 0},
        {nw_stats_table_diag_avx2_256_8,        "nw_stats", "diag",    "avx2",  "256", "8",  1, 1, 0},
        {nw_stats_table_striped_avx2_256_32,    "nw_stats", "striped", "avx2",  "256", "32", 1, 1, 0},
        {nw_stats_table_striped_avx2_256_16,    "nw_stats", "striped", "avx2",  "256", "16", 1, 1, 0},
        {nw_stats_table_striped_avx2_256_8,     "nw_stats", "striped", "avx2",  "256", "8",  1, 1, 0},
#endif
#if HAVE_KNC
        {nw_stats_table_scan_knc_512_32,        "nw_stats", "scan",    "knc",   "512", "32", 1, 1, 0},
        {nw_stats_table_diag_knc_512_32,        "nw_stats", "diag",    "knc",   "512", "32", 1, 1, 0},
        {nw_stats_table_striped_knc_512_32,     "nw_stats", "striped", "knc",   "512", "32", 1, 1, 0},
#endif

        {sg_stats_table,                        "sg_stats", "orig",    "NA",    "32",  "32", 1, 1, 1},
        {sg_stats_table_scan,                   "sg_stats", "scan",    "NA",    "32",  "32", 1, 1, 0},
#if HAVE_SSE2
        {sg_stats_table_scan_sse2_128_32,       "sg_stats", "scan",    "sse2",  "128", "32", 1, 1, 0},
        {sg_stats_table_scan_sse2_128_16,       "sg_stats", "scan",    "sse2",  "128", "16", 1, 1, 0},
        {sg_stats_table_scan_sse2_128_8,        "sg_stats", "scan",    "sse2",  "128", "8",  1, 1, 0},
        {sg_stats_table_diag_sse2_128_32,       "sg_stats", "diag",    "sse2",  "128", "32", 1, 1, 0},
        {sg_stats_table_diag_sse2_128_16,       "sg_stats", "diag",    "sse2",  "128", "16", 1, 1, 0},
        {sg_stats_table_diag_sse2_128_8,        "sg_stats", "diag",    "sse2",  "128", "8",  1, 1, 0},
        {sg_stats_table_striped_sse2_128_32,    "sg_stats", "striped", "sse2",  "128", "32", 1, 1, 0},
        {sg_stats_table_striped_sse2_128_16,    "sg_stats", "striped", "sse2",  "128", "16", 1, 1, 0},
        {sg_stats_table_striped_sse2_128_8,     "sg_stats", "striped", "sse2",  "128", "8",  1, 1, 0},
#endif
#if HAVE_SSE41
        {sg_stats_table_scan_sse41_128_32,      "sg_stats", "scan",    "sse41", "128", "32", 1, 1, 0},
        {sg_stats_table_scan_sse41_128_16,      "sg_stats", "scan",    "sse41", "128", "16", 1, 1, 0},
        {sg_stats_table_scan_sse41_128_8,       "sg_stats", "scan",    "sse41", "128", "8",  1, 1, 0},
        {sg_stats_table_diag_sse41_128_32,      "sg_stats", "diag",    "sse41", "128", "32", 1, 1, 0},
        {sg_stats_table_diag_sse41_128_16,      "sg_stats", "diag",    "sse41", "128", "16", 1, 1, 0},
        {sg_stats_table_diag_sse41_128_8,       "sg_stats", "diag",    "sse41", "128", "8",  1, 1, 0},
        {sg_stats_table_striped_sse41_128_32,   "sg_stats", "striped", "sse41", "128", "32", 1, 1, 0},
        {sg_stats_table_striped_sse41_128_16,   "sg_stats", "striped", "sse41", "128", "16", 1, 1, 0},
        {sg_stats_table_striped_sse41_128_8,    "sg_stats", "striped", "sse41", "128", "8",  1, 1, 0},
#endif
#if HAVE_AVX2
        {sg_stats_table_scan_avx2_256_32,       "sg_stats", "scan",    "avx2",  "256", "32", 1, 1, 0},
        {sg_stats_table_scan_avx2_256_16,       "sg_stats", "scan",    "avx2",  "256", "16", 1, 1, 0},
        {sg_stats_table_scan_avx2_256_8,        "sg_stats", "scan",    "avx2",  "256", "8",  1, 1, 0},
        {sg_stats_table_diag_avx2_256_32,       "sg_stats", "diag",    "avx2",  "256", "32", 1, 1, 0},
        {sg_stats_table_diag_avx2_256_16,       "sg_stats", "diag",    "avx2",  "256", "16", 1, 1, 0},
        {sg_stats_table_diag_avx2_256_8,        "sg_stats", "diag",    "avx2",  "256", "8",  1, 1, 0},
        {sg_stats_table_striped_avx2_256_32,    "sg_stats", "striped", "avx2",  "256", "32", 1, 1, 0},
        {sg_stats_table_striped_avx2_256_16,    "sg_stats", "striped", "avx2",  "256", "16", 1, 1, 0},
        {sg_stats_table_striped_avx2_256_8,     "sg_stats", "striped", "avx2",  "256", "8",  1, 1, 0},
#endif
#if HAVE_KNC
        {sg_stats_table_scan_knc_512_32,        "sg_stats", "scan",    "knc",   "512", "32", 1, 1, 0},
        {sg_stats_table_diag_knc_512_32,        "sg_stats", "diag",    "knc",   "512", "32", 1, 1, 0},
        {sg_stats_table_striped_knc_512_32,     "sg_stats", "striped", "knc",   "512", "32", 1, 1, 0},
#endif

        {sw_stats_table,                        "sw_stats", "orig",    "NA",    "32",  "32", 1, 1, 1},
        {sw_stats_table_scan,                   "sw_stats", "scan",    "NA",    "32",  "32", 1, 1, 0},
#if HAVE_SSE2
        {sw_stats_table_scan_sse2_128_32,       "sw_stats", "scan",    "sse2",  "128", "32", 1, 1, 0},
        {sw_stats_table_scan_sse2_128_16,       "sw_stats", "scan",    "sse2",  "128", "16", 1, 1, 0},
        {sw_stats_table_scan_sse2_128_8,        "sw_stats", "scan",    "sse2",  "128", "8",  1, 1, 0},
        {sw_stats_table_diag_sse2_128_32,       "sw_stats", "diag",    "sse2",  "128", "32", 1, 1, 0},
        {sw_stats_table_diag_sse2_128_16,       "sw_stats", "diag",    "sse2",  "128", "16", 1, 1, 0},
        {sw_stats_table_diag_sse2_128_8,        "sw_stats", "diag",    "sse2",  "128", "8",  1, 1, 0},
        {sw_stats_table_striped_sse2_128_32,    "sw_stats", "striped", "sse2",  "128", "32", 1, 1, 0},
        {sw_stats_table_striped_sse2_128_16,    "sw_stats", "striped", "sse2",  "128", "16", 1, 1, 0},
        {sw_stats_table_striped_sse2_128_8,     "sw_stats", "striped", "sse2",  "128", "8",  1, 1, 0},
#endif
#if HAVE_SSE41
        {sw_stats_table_scan_sse41_128_32,      "sw_stats", "scan",    "sse41", "128", "32", 1, 1, 0},
        {sw_stats_table_scan_sse41_128_16,      "sw_stats", "scan",    "sse41", "128", "16", 1, 1, 0},
        {sw_stats_table_scan_sse41_128_8,       "sw_stats", "scan",    "sse41", "128", "8",  1, 1, 0},
        {sw_stats_table_diag_sse41_128_32,      "sw_stats", "diag",    "sse41", "128", "32", 1, 1, 0},
        {sw_stats_table_diag_sse41_128_16,      "sw_stats", "diag",    "sse41", "128", "16", 1, 1, 0},
        {sw_stats_table_diag_sse41_128_8,       "sw_stats", "diag",    "sse41", "128", "8",  1, 1, 0},
        {sw_stats_table_striped_sse41_128_32,   "sw_stats", "striped", "sse41", "128", "32", 1, 1, 0},
        {sw_stats_table_striped_sse41_128_16,   "sw_stats", "striped", "sse41", "128", "16", 1, 1, 0},
        {sw_stats_table_striped_sse41_128_8,    "sw_stats", "striped", "sse41", "128", "8",  1, 1, 0},
#endif
#if HAVE_AVX2
        {sw_stats_table_scan_avx2_256_32,       "sw_stats", "scan",    "avx2",  "256", "32", 1, 1, 0},
        {sw_stats_table_scan_avx2_256_16,       "sw_stats", "scan",    "avx2",  "256", "16", 1, 1, 0},
        {sw_stats_table_scan_avx2_256_8,        "sw_stats", "scan",    "avx2",  "256", "8",  1, 1, 0},
        {sw_stats_table_diag_avx2_256_32,       "sw_stats", "diag",    "avx2",  "256", "32", 1, 1, 0},
        {sw_stats_table_diag_avx2_256_16,       "sw_stats", "diag",    "avx2",  "256", "16", 1, 1, 0},
        {sw_stats_table_diag_avx2_256_8,        "sw_stats", "diag",    "avx2",  "256", "8",  1, 1, 0},
        {sw_stats_table_striped_avx2_256_32,    "sw_stats", "striped", "avx2",  "256", "32", 1, 1, 0},
        {sw_stats_table_striped_avx2_256_16,    "sw_stats", "striped", "avx2",  "256", "16", 1, 1, 0},
        {sw_stats_table_striped_avx2_256_8,     "sw_stats", "striped", "avx2",  "256", "8",  1, 1, 0},
#endif
#if HAVE_KNC
        {sw_stats_table_scan_knc_512_32,        "sw_stats", "scan",    "knc",   "512", "32", 1, 1, 0},
        {sw_stats_table_diag_knc_512_32,        "sw_stats", "diag",    "knc",   "512", "32", 1, 1, 0},
        {sw_stats_table_striped_knc_512_32,     "sw_stats", "striped", "knc",   "512", "32", 1, 1, 0},
#endif

        {NULL, "", "", "", "", "", 0, 0, 0}
    };

    timer_init();
    printf("%s timer\n", timer_name());
    printf("file: %s\n", filename);
    printf("blosum: %s\n", blosumname);
    printf("gap open: %d\n", open);
    printf("gap extend: %d\n", extend);
    printf("seq pair %lu,%lu\n", seqA_index, seqB_index);

    printf("%-15s %8s %6s %4s %5s %5s %8s %8s %8s %8s %8s %5s %8s %8s %8s\n",
            "name", "type", "isa", "bits", "width", "elem",
            "score", "matches", "similar", "length",
            "avg", "imp", "stddev", "min", "max");

    stats_clear(&stats_rdtsc);
    stats_clear(&stats_nsecs);
    index = 0;
    f = functions[index++];
    while (f.f) {
        char name[16] = {'\0'};
        int new_limit = f.is_table ? 1 : limit;
#if 0
        if (f.is_table && HAVE_KNC) {
            f = functions[index++];
            continue;
        }
#endif
        if ((0 == strncmp(f.isa, "sse2",  4) && 0 == parasail_can_use_sse2()) 
                || (0 == strncmp(f.isa, "sse41", 5) && 0 == parasail_can_use_sse41())
                || (0 == strncmp(f.isa, "avx2",  4) && 0 == parasail_can_use_avx2())) {
            f = functions[index++];
            continue;
        }
        stats_clear(&stats_rdtsc);
        timer_rdtsc = timer_start();
        timer_nsecs = timer_real();
        for (i=0; i<new_limit; ++i) {
            timer_rdtsc_single = timer_start();
            timer_nsecs_single = timer_real();
            result = f.f(seqA, lena, seqB, lenb, open, extend, b.blosum);
            timer_rdtsc_single = timer_end(timer_rdtsc_single);
            timer_nsecs_single = timer_real() - timer_nsecs_single;
            stats_sample_value(&stats_rdtsc, timer_rdtsc_single);
            stats_sample_value(&stats_nsecs, timer_nsecs_single);
            score = result->score;
            similar = result->similar;
            matches = result->matches;
            length = result->length;
            parasail_result_free(result);
        }
        timer_rdtsc = timer_end(timer_rdtsc);
        timer_nsecs = timer_real() - timer_nsecs;
        if (f.is_ref) {
            timer_rdtsc_ref_mean = stats_rdtsc._mean;
#if USE_TIMER_REAL
            timer_nsecs_ref_mean = stats_nsecs._mean;
#endif
        }
        strcpy(name, f.alg);
        /* xeon phi was unable to perform I/O running natively */
        if (f.is_table) {
            char suffix[256] = {0};
            if (strlen(f.type)) {
                strcat(suffix, "_");
                strcat(suffix, f.type);
            }
            if (strlen(f.isa)) {
                strcat(suffix, "_");
                strcat(suffix, f.isa);
            }
            if (strlen(f.bits)) {
                strcat(suffix, "_");
                strcat(suffix, f.bits);
            }
            if (strlen(f.width)) {
                strcat(suffix, "_");
                strcat(suffix, f.width);
            }
            strcat(suffix, ".txt");
            result = f.f(seqA, lena, seqB, lenb, open, extend, b.blosum);
            {
                char filename[256] = {'\0'};
                strcpy(filename, f.alg);
                strcat(filename, "_scr");
                strcat(filename, suffix);
                print_array(filename, result->score_table, seqA, lena, seqB, lenb);
            }
            if (f.is_stats) {
                char filename[256] = {'\0'};
                strcpy(filename, f.alg);
                strcat(filename, "_mch");
                strcat(filename, suffix);
                print_array(filename, result->matches_table, seqA, lena, seqB, lenb);
            }
            if (f.is_stats) {
                char filename[256] = {'\0'};
                strcpy(filename, f.alg);
                strcat(filename, "_sim");
                strcat(filename, suffix);
                print_array(filename, result->similar_table, seqA, lena, seqB, lenb);
            }
            if (f.is_stats) {
                char filename[256] = {'\0'};
                strcpy(filename, f.alg);
                strcat(filename, "_len");
                strcat(filename, suffix);
                print_array(filename, result->length_table, seqA, lena, seqB, lenb);
            }
            parasail_result_free(result);
        }
        if (f.is_table) {
            strcat(name, "_table");
        }
#if USE_TIMER_REAL
        printf("%-15s %8s %6s %4s %5s %5d %8d %8d %8d %8d %8.2f %5.2f %8.2f %8.2f %8.2f %8.7f %5.7f %8.7f %8.7f %8.7f\n",
                name, f.type, f.isa, f.bits, f.width, elem(f),
                score, matches, similar, length,
                stats_rdtsc._mean, pctf(timer_rdtsc_ref_mean, stats_rdtsc._mean),
                stats_stddev(&stats_rdtsc), stats_rdtsc._min, stats_rdtsc._max,
                stats_nsecs._mean, pctf(timer_nsecs_ref_mean, stats_nsecs._mean),
                stats_stddev(&stats_nsecs), stats_nsecs._min, stats_nsecs._max);
#else
        printf("%-15s %8s %6s %4s %5s %5d %8d %8d %8d %8d %8.2f %5.2f %8.2f %8.2f %8.2f\n",
                name, f.type, f.isa, f.bits, f.width, elem(f),
                score, matches, similar, length,
                stats_rdtsc._mean, pctf(timer_rdtsc_ref_mean, stats_rdtsc._mean),
                stats_stddev(&stats_rdtsc), stats_rdtsc._min, stats_rdtsc._max);
#endif
        f = functions[index++];
    }

    if (filename) {
        for (s=0; s<seq_count; ++s) {
            free(sequences[s]);
        }
        free(sequences);
        free(sizes);
    }

    return 0;
}
