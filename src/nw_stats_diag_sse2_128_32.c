/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2014 Battelle Memorial Institute.
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#include "config.h"

#include <stdlib.h>

#include <emmintrin.h>

#include "parasail.h"
#include "parasail_internal.h"
#include "parasail_internal_sse.h"
#include "blosum/blosum_map.h"

#define NEG_INF_32 (INT32_MIN/(int32_t)(2))

/* sse2 does not have _mm_insert_epi32, emulate it */
static inline __m128i _mm_insert_epi32(__m128i a, int32_t i, int imm) {
    __m128i_32_t tmp;
    tmp.m = a;
    tmp.v[imm] = i;
    return tmp.m;
}

/* sse2 does not have _mm_extract_epi32, emulate it */
static inline int32_t _mm_extract_epi32(__m128i a, int imm) {
    __m128i_32_t tmp;
    tmp.m = a;
    return tmp.v[imm];
}

/* sse2 does not have _mm_max_epi32, emulate it */
static inline __m128i _mm_max_epi32(__m128i a, __m128i b) {
    __m128i mask = _mm_cmpgt_epi32(a,b);
    a = _mm_and_si128(a,mask);
    b = _mm_andnot_si128(mask,b);
    return _mm_or_si128(a,b);
}

/* shift given vector v, insert val, return shifted val */
static inline __m128i vshift32(const __m128i v, const int val)
{
    __m128i ret = _mm_srli_si128(v, 4);
    ret = _mm_insert_epi32(ret, val, 3);
    return ret;
}


#ifdef PARASAIL_TABLE
static inline void arr_store_si128(
        int *array,
        __m128i vWscore,
        int i,
        int s1Len,
        int j,
        int s2Len)
{
    if (0 <= i+0 && i+0 < s1Len && 0 <= j-0 && j-0 < s2Len) {
        array[(i+0)*s2Len + (j-0)] = (int32_t)_mm_extract_epi32(vWscore, 3);
    }
    if (0 <= i+1 && i+1 < s1Len && 0 <= j-1 && j-1 < s2Len) {
        array[(i+1)*s2Len + (j-1)] = (int32_t)_mm_extract_epi32(vWscore, 2);
    }
    if (0 <= i+2 && i+2 < s1Len && 0 <= j-2 && j-2 < s2Len) {
        array[(i+2)*s2Len + (j-2)] = (int32_t)_mm_extract_epi32(vWscore, 1);
    }
    if (0 <= i+3 && i+3 < s1Len && 0 <= j-3 && j-3 < s2Len) {
        array[(i+3)*s2Len + (j-3)] = (int32_t)_mm_extract_epi32(vWscore, 0);
    }
}
#endif


#ifdef PARASAIL_TABLE
#define FNAME nw_stats_table_diag_sse2_128_32
#else
#define FNAME nw_stats_diag_sse2_128_32
#endif

parasail_result_t* FNAME(
        const char * const restrict _s1, const int s1Len,
        const char * const restrict _s2, const int s2Len,
        const int open, const int gap, const int matrix[24][24])
{
    const int N = 4; /* number of values in vector */
    const int PAD = N-1; /* N 32-byte values in vector, so N - 1 */
    const int PAD2 = PAD*2;
    int * const restrict s1      = parasail_memalign_int(16, s1Len+PAD);
    int * const restrict s2B     = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict _tbl_pr = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict _del_pr = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict _mch_pr = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict _sim_pr = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict _len_pr = parasail_memalign_int(16, s2Len+PAD2);
    int * const restrict s2 = s2B+PAD; /* will allow later for negative indices */
    int * const restrict tbl_pr = _tbl_pr+PAD;
    int * const restrict del_pr = _del_pr+PAD;
    int * const restrict mch_pr = _mch_pr+PAD;
    int * const restrict sim_pr = _sim_pr+PAD;
    int * const restrict len_pr = _len_pr+PAD;
#ifdef PARASAIL_TABLE
    parasail_result_t *result = parasail_result_new_table3(s1Len, s2Len);
#else
    parasail_result_t *result = parasail_result_new();
#endif
    int i = 0;
    int j = 0;
    int score = NEG_INF_32;
    int matches = NEG_INF_32;
    int similar = NEG_INF_32;
    int length = NEG_INF_32;
    __m128i vNegInf = _mm_set1_epi32(NEG_INF_32);
    __m128i vOpen = _mm_set1_epi32(open);
    __m128i vGap  = _mm_set1_epi32(gap);
    __m128i vZero = _mm_set1_epi32(0);
    __m128i vOne = _mm_set1_epi32(1);
    __m128i vN = _mm_set1_epi32(N);
    __m128i vGapN = _mm_set1_epi32(gap*N);
    __m128i vNegOne = _mm_set1_epi32(-1);
    __m128i vI = _mm_set_epi32(0,1,2,3);
    __m128i vJreset = _mm_set_epi32(0,-1,-2,-3);
    __m128i vMaxScore = vNegInf;
    __m128i vMaxMatch = vNegInf;
    __m128i vMaxSimilar = vNegInf;
    __m128i vMaxLength = vNegInf;
    __m128i vILimit = _mm_set1_epi32(s1Len);
    __m128i vILimit1 = _mm_sub_epi32(vILimit, vOne);
    __m128i vJLimit = _mm_set1_epi32(s2Len);
    __m128i vJLimit1 = _mm_sub_epi32(vJLimit, vOne);
    __m128i vIBoundary = _mm_set_epi32(
            -open-0*gap,
            -open-1*gap,
            -open-2*gap,
            -open-3*gap);

    /* convert _s1 from char to int in range 0-23 */
    for (i=0; i<s1Len; ++i) {
        s1[i] = MAP_BLOSUM_[(unsigned char)_s1[i]];
    }
    /* pad back of s1 with dummy values */
    for (i=s1Len; i<s1Len+PAD; ++i) {
        s1[i] = 0; /* point to first matrix row because we don't care */
    }

    /* convert _s2 from char to int in range 0-23 */
    for (j=0; j<s2Len; ++j) {
        s2[j] = MAP_BLOSUM_[(unsigned char)_s2[j]];
    }
    /* pad front of s2 with dummy values */
    for (j=-PAD; j<0; ++j) {
        s2[j] = 0; /* point to first matrix row because we don't care */
    }
    /* pad back of s2 with dummy values */
    for (j=s2Len; j<s2Len+PAD; ++j) {
        s2[j] = 0; /* point to first matrix row because we don't care */
    }

    /* set initial values for stored row */
    for (j=0; j<s2Len; ++j) {
        tbl_pr[j] = -open - j*gap;
        del_pr[j] = NEG_INF_32;
        mch_pr[j] = 0;
        sim_pr[j] = 0;
        len_pr[j] = 0;
    }
    /* pad front of stored row values */
    for (j=-PAD; j<0; ++j) {
        tbl_pr[j] = NEG_INF_32;
        del_pr[j] = NEG_INF_32;
        mch_pr[j] = 0;
        sim_pr[j] = 0;
        len_pr[j] = 0;
    }
    /* pad back of stored row values */
    for (j=s2Len; j<s2Len+PAD; ++j) {
        tbl_pr[j] = NEG_INF_32;
        del_pr[j] = NEG_INF_32;
        mch_pr[j] = 0;
        sim_pr[j] = 0;
        len_pr[j] = 0;
    }
    tbl_pr[-1] = 0; /* upper left corner */

    /* iterate over query sequence */
    for (i=0; i<s1Len; i+=N) {
        __m128i vNscore = vNegInf;
        __m128i vNmatch = vZero;
        __m128i vNsimilar = vZero;
        __m128i vNlength = vZero;
        __m128i vWscore = vNegInf;
        __m128i vWmatch = vZero;
        __m128i vWsimilar = vZero;
        __m128i vWlength = vZero;
        __m128i vIns = vNegInf;
        __m128i vDel = vNegInf;
        __m128i vJ = vJreset;
        __m128i vs1 = _mm_set_epi32(
                s1[i+0],
                s1[i+1],
                s1[i+2],
                s1[i+3]);
        __m128i vs2 = vNegInf;
        const int * const restrict matrow0 = matrix[s1[i+0]];
        const int * const restrict matrow1 = matrix[s1[i+1]];
        const int * const restrict matrow2 = matrix[s1[i+2]];
        const int * const restrict matrow3 = matrix[s1[i+3]];
        vNscore = vshift32(vNscore, tbl_pr[-1]);
        vNmatch = vshift32(vNmatch, 0);
        vNsimilar = vshift32(vNsimilar, 0);
        vNlength = vshift32(vNlength, 0);
        vWscore = vshift32(vWscore, -open - i*gap);
        vWmatch = vshift32(vWmatch, 0);
        vWsimilar = vshift32(vWsimilar, 0);
        vWlength = vshift32(vWlength, 0);
        tbl_pr[-1] = -open - (i+N)*gap;
        /* iterate over database sequence */
        for (j=0; j<s2Len+PAD; ++j) {
            __m128i vMat;
            __m128i vNWscore = vNscore;
            __m128i vNWmatch = vNmatch;
            __m128i vNWsimilar = vNsimilar;
            __m128i vNWlength = vNlength;
            vNscore = vshift32(vWscore, tbl_pr[j]);
            vNmatch = vshift32(vWmatch, mch_pr[j]);
            vNsimilar = vshift32(vWsimilar, sim_pr[j]);
            vNlength = vshift32(vWlength, len_pr[j]);
            vDel = vshift32(vDel, del_pr[j]);
            vDel = _mm_max_epi32(
                    _mm_sub_epi32(vNscore, vOpen),
                    _mm_sub_epi32(vDel, vGap));
            vIns = _mm_max_epi32(
                    _mm_sub_epi32(vWscore, vOpen),
                    _mm_sub_epi32(vIns, vGap));
            vs2 = vshift32(vs2, s2[j]);
            vMat = _mm_set_epi32(
                    matrow0[s2[j-0]],
                    matrow1[s2[j-1]],
                    matrow2[s2[j-2]],
                    matrow3[s2[j-3]]
                    );
            vNWscore = _mm_add_epi32(vNWscore, vMat);
            vWscore = _mm_max_epi32(vNWscore, vIns);
            vWscore = _mm_max_epi32(vWscore, vDel);
            /* conditional block */
            {
                __m128i case1not;
                __m128i case2not;
                __m128i case2;
                __m128i case3;
                __m128i vCmatch;
                __m128i vCsimilar;
                __m128i vClength;
                case1not = _mm_or_si128(
                        _mm_cmplt_epi32(vNWscore,vDel),
                        _mm_cmplt_epi32(vNWscore,vIns));
                case2not = _mm_cmplt_epi32(vDel,vIns);
                case2 = _mm_andnot_si128(case2not,case1not);
                case3 = _mm_and_si128(case1not,case2not);
                vCmatch = _mm_andnot_si128(case1not,
                        _mm_add_epi32(vNWmatch, _mm_and_si128(
                                _mm_cmpeq_epi32(vs1,vs2),vOne)));
                vCmatch = _mm_or_si128(vCmatch, _mm_and_si128(case2, vNmatch));
                vCmatch = _mm_or_si128(vCmatch, _mm_and_si128(case3, vWmatch));
                vCsimilar = _mm_andnot_si128(case1not,
                        _mm_add_epi32(vNWsimilar, _mm_and_si128(
                                _mm_cmpgt_epi32(vMat,vZero),vOne)));
                vCsimilar = _mm_or_si128(vCsimilar, _mm_and_si128(case2, vNsimilar));
                vCsimilar = _mm_or_si128(vCsimilar, _mm_and_si128(case3, vWsimilar));
                vClength= _mm_andnot_si128(case1not,
                        _mm_add_epi32(vNWlength, vOne));
                vClength= _mm_or_si128(vClength,_mm_and_si128(case2,
                            _mm_add_epi32(vNlength, vOne)));
                vClength= _mm_or_si128(vClength,_mm_and_si128(case3,
                            _mm_add_epi32(vWlength, vOne)));
                vWmatch = vCmatch;
                vWsimilar = vCsimilar;
                vWlength = vClength;
            }

            /* as minor diagonal vector passes across the j=-1 boundary,
             * assign the appropriate boundary conditions */
            {
                __m128i cond = _mm_cmpeq_epi32(vJ,vNegOne);
                vWscore = _mm_andnot_si128(cond, vWscore); /* all but j=-1 */
                vWscore = _mm_or_si128(vWscore,
                        _mm_and_si128(cond, vIBoundary));
                vWmatch = _mm_andnot_si128(cond, vWmatch);
                vWsimilar = _mm_andnot_si128(cond, vWsimilar);
                vWlength = _mm_andnot_si128(cond, vWlength);
                vDel = _mm_andnot_si128(cond, vDel);
                vDel = _mm_or_si128(vDel, _mm_and_si128(cond, vNegInf));
                vIns = _mm_andnot_si128(cond, vIns);
                vIns = _mm_or_si128(vIns, _mm_and_si128(cond, vNegInf));
            }
#ifdef PARASAIL_TABLE
            arr_store_si128(result->score_table, vWscore, i, s1Len, j, s2Len);
            arr_store_si128(result->matches_table, vWmatch, i, s1Len, j, s2Len);
            arr_store_si128(result->similar_table, vWsimilar, i, s1Len, j, s2Len);
            arr_store_si128(result->length_table, vWlength, i, s1Len, j, s2Len);
#endif
            tbl_pr[j-3] = (int32_t)_mm_extract_epi32(vWscore,0);
            mch_pr[j-3] = (int32_t)_mm_extract_epi32(vWmatch,0);
            sim_pr[j-3] = (int32_t)_mm_extract_epi32(vWsimilar,0);
            len_pr[j-3] = (int32_t)_mm_extract_epi32(vWlength,0);
            del_pr[j-3] = (int32_t)_mm_extract_epi32(vDel,0);
            /* as minor diagonal vector passes across table, extract
               last table value at the i,j bound */
            {
                __m128i cond_valid_I = _mm_cmpeq_epi32(vI, vILimit1);
                __m128i cond_valid_J = _mm_cmpeq_epi32(vJ, vJLimit1);
                __m128i cond_all = _mm_and_si128(cond_valid_I, cond_valid_J);
                vMaxScore = _mm_andnot_si128(cond_all, vMaxScore);
                vMaxScore = _mm_or_si128(vMaxScore,
                        _mm_and_si128(cond_all, vWscore));
                vMaxMatch = _mm_andnot_si128(cond_all, vMaxMatch);
                vMaxMatch = _mm_or_si128(vMaxMatch,
                        _mm_and_si128(cond_all, vWmatch));
                vMaxSimilar = _mm_andnot_si128(cond_all, vMaxSimilar);
                vMaxSimilar = _mm_or_si128(vMaxSimilar,
                        _mm_and_si128(cond_all, vWsimilar));
                vMaxLength = _mm_andnot_si128(cond_all, vMaxLength);
                vMaxLength = _mm_or_si128(vMaxLength,
                        _mm_and_si128(cond_all, vWlength));
            }
            vJ = _mm_add_epi32(vJ, vOne);
        }
        vI = _mm_add_epi32(vI, vN);
        vIBoundary = _mm_sub_epi32(vIBoundary, vGapN);
    }

    /* max in vMaxScore */
    for (i=0; i<N; ++i) {
        int32_t value;
        value = (int32_t) _mm_extract_epi32(vMaxScore, 3);
        if (value > score) {
            score = value;
            matches = (int32_t) _mm_extract_epi32(vMaxMatch, 3);
            similar = (int32_t) _mm_extract_epi32(vMaxSimilar, 3);
            length= (int32_t) _mm_extract_epi32(vMaxLength,3);
        }
        vMaxScore = _mm_slli_si128(vMaxScore, 4);
        vMaxMatch = _mm_slli_si128(vMaxMatch, 4);
        vMaxSimilar = _mm_slli_si128(vMaxSimilar, 4);
        vMaxLength = _mm_slli_si128(vMaxLength, 4);
    }

    result->score = score;
    result->matches = matches;
    result->similar = similar;
    result->length = length;

    parasail_free(_len_pr);
    parasail_free(_sim_pr);
    parasail_free(_mch_pr);
    parasail_free(_del_pr);
    parasail_free(_tbl_pr);
    parasail_free(s2B);
    parasail_free(s1);

    return result;
}

