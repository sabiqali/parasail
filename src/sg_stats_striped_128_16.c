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

#include <stdint.h>
#include <stdlib.h>

#include <emmintrin.h>

#ifdef PARASAIL_TABLE
#include "align_striped_128_16_table.h"
#else
#include "align_striped_128_16.h"
#endif
#include "blosum/blosum_map.h"


#if PARASAIL_TABLE
static inline void arr_store_si128(
        int *array,
        __m128i vH,
        int32_t t,
        int32_t seglen,
        int32_t d,
        int32_t dlen)
{
    array[(0*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 0);
    array[(1*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 1);
    array[(2*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 2);
    array[(3*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 3);
    array[(4*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 4);
    array[(5*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 5);
    array[(6*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 6);
    array[(7*seglen+t)*dlen + d] = (int16_t)_mm_extract_epi16(vH, 7);
}
#endif

#ifdef PARASAIL_TABLE
#define FNAME sg_stats_striped_128_16_table
#else
#define FNAME sg_stats_striped_128_16
#endif

int FNAME(
        const char * const restrict s1, const int s1Len,
        const char * const restrict s2, const int s2Len,
        const int open, const int gap,
        const int8_t * const restrict matrix,
        int * const restrict matches, int * const restrict length
#ifdef PARASAIL_TABLE
        , int * const restrict score_table
        , int * const restrict match_table
        , int * const restrict length_table
#endif
        )
{
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    int32_t nt = 0;
    int32_t segNum = 0;
    const int32_t n = 24; /* number of amino acids in table */
    int32_t segLen = (s1Len + 7) / 8;
    __m128i* vProfile = (__m128i*)malloc(n * segLen * sizeof(__m128i));
    __m128i* vProfileS = (__m128i*)malloc(n * segLen * sizeof(__m128i));

    int32_t offset = (s1Len - 1) % segLen;
    int32_t position = 7 - (s1Len - 1) / segLen;

    /* the max alignment score */
    int max = NEG_INF_16;

    /* Define 16 byte 0 vector. */
    __m128i vZero = _mm_setzero_si128();
    __m128i vOne = _mm_set1_epi16(1);

    __m128i* pvHStore = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvHLoad = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvHMStore = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvHMLoad = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvHLStore = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvHLLoad = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvEStore = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvELoad = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvEM = (__m128i*) calloc(segLen, sizeof(__m128i));
    __m128i* pvEL = (__m128i*) calloc(segLen, sizeof(__m128i));

    /* 16 byte insertion begin vector */
    __m128i vGapO = _mm_set1_epi16(open);

    /* 16 byte insertion extension vector */
    __m128i vGapE = _mm_set1_epi16(gap);

    __m128i initialF = _mm_set_epi16(
            -open-7*segLen*gap,
            -open-6*segLen*gap,
            -open-5*segLen*gap,
            -open-4*segLen*gap,
            -open-3*segLen*gap,
            -open-2*segLen*gap,
            -open-1*segLen*gap,
            -open-0*segLen*gap);

    /* Generate query profile rearrange query sequence & calculate the weight
     * of match/mismatch */
    {
        int16_t *t = (int16_t*)vProfile;
        int16_t *s = (int16_t*)vProfileS;
        for (nt=0; nt<n; ++nt) {
            for (i=0; i<segLen; ++i) {
                int32_t j = i;
                for (segNum=0; segNum<8; ++segNum) {
                    *t++ = matrix[nt*n + MAP_BLOSUM_[(unsigned char)s1[j]]];
                    *s++ = (nt == MAP_BLOSUM_[(unsigned char)s1[j]]);
                    j += segLen;
                }
            }
        }
    }

    /* initialize E */
    {
        int16_t *e = (int16_t*)pvEStore;
        for (i=0; i<segLen; ++i) {
            for (segNum=0; segNum<8; ++segNum) {
                *e++ = -open;
            }
        }
    }

    /* outer loop over database sequence */
    for (j=0; j<s2Len; ++j) {
        __m128i vE;
        __m128i vEM;
        __m128i vEL;
        /* Initialize F value to 0.  Any errors to vH values will be corrected
         * in the Lazy_F loop.  */
        __m128i vF = initialF;
        __m128i vFM = vZero;
        __m128i vFL = vZero;

        /* load final segment of pvHStore and shift left by 2 bytes */
        __m128i vH = _mm_slli_si128(pvHStore[segLen - 1], 2);
        __m128i vHM = _mm_slli_si128(pvHMStore[segLen - 1], 2);
        __m128i vHL = _mm_slli_si128(pvHLStore[segLen - 1], 2);

        /* Correct part of the vProfile */
        const __m128i* vP = vProfile + MAP_BLOSUM_[(unsigned char)s2[j]] * segLen;
        const __m128i* vPS = vProfileS + MAP_BLOSUM_[(unsigned char)s2[j]] * segLen;

        /* Swap the 2 H buffers. */
        __m128i* pv = pvHLoad;
        pvHLoad = pvHStore;
        pvHStore = pv;
        pv = pvHMLoad;
        pvHMLoad = pvHMStore;
        pvHMStore = pv;
        pv = pvHLLoad;
        pvHLLoad = pvHLStore;
        pvHLStore = pv;
        pv = pvELoad;
        pvELoad = pvEStore;
        pvEStore = pv;

        /* inner loop to process the query sequence */
        for (i=0; i<segLen; ++i) {
            __m128i case1not;
            __m128i case2not;
            __m128i case2;
            __m128i case3;

            vH = _mm_adds_epi16(vH, _mm_load_si128(vP + i));
            vE = _mm_load_si128(pvELoad + i);

            /* determine which direction of length and match to
             * propagate, before vH is finished calculating */
            case1not = _mm_or_si128(
                    _mm_cmplt_epi16(vH,vF),_mm_cmplt_epi16(vH,vE));
            case2not = _mm_cmplt_epi16(vF,vE);
            case2 = _mm_andnot_si128(case2not,case1not);
            case3 = _mm_and_si128(case1not,case2not);

            /* Get max from vH, vE and vF. */
            vH = _mm_max_epi16(vH, vE);
            vH = _mm_max_epi16(vH, vF);
            /* Save vH values. */
            _mm_store_si128(pvHStore + i, vH);

            /* calculate vM */
            vEM = _mm_load_si128(pvEM + i);
            vHM = _mm_andnot_si128(case1not,
                    _mm_add_epi16(vHM, _mm_load_si128(vPS + i)));
            vHM = _mm_or_si128(vHM, _mm_and_si128(case2, vFM));
            vHM = _mm_or_si128(vHM, _mm_and_si128(case3, vEM));
            _mm_store_si128(pvHMStore + i, vHM);

            /* calculate vL */
            vEL = _mm_load_si128(pvEL + i);
            vHL = _mm_andnot_si128(case1not, _mm_add_epi16(vHL, vOne));
            vHL = _mm_or_si128(vHL, _mm_and_si128(case2,
                        _mm_add_epi16(vFL, vOne)));
            vHL = _mm_or_si128(vHL, _mm_and_si128(case3,
                        _mm_add_epi16(vEL, vOne)));
            _mm_store_si128(pvHLStore + i, vHL);

#ifdef PARASAIL_TABLE
            arr_store_si128(match_table, vHM, i, segLen, j, s2Len);
            arr_store_si128(length_table, vHL, i, segLen, j, s2Len);
            arr_store_si128(score_table, vH, i, segLen, j, s2Len);
#endif

            /* Update vE value. */
            vH = _mm_subs_epi16(vH, vGapO);
            vE = _mm_subs_epi16(vE, vGapE);
            vE = _mm_max_epi16(vE, vH);
            _mm_store_si128(pvEStore + i, vE);
            _mm_store_si128(pvEM + i, vHM);
            _mm_store_si128(pvEL + i, vHL);

            /* Update vF value. */
            vF = _mm_subs_epi16(vF, vGapE);
            vF = _mm_max_epi16(vF, vH);
            vFM = vHM;
            vFL = vHL;

            /* Load the next vH. */
            vH = _mm_load_si128(pvHLoad + i);
            vHM = _mm_load_si128(pvHMLoad + i);
            vHL = _mm_load_si128(pvHLLoad + i);
        }

        /* Lazy_F loop: has been revised to disallow adjecent insertion and
         * then deletion, so don't update E(i, i), learn from SWPS3 */
        for (k=0; k<8; ++k) {
            __m128i vHp = _mm_slli_si128(pvHLoad[segLen - 1], 2);
            vF = _mm_slli_si128(vF, 2);
            vF = _mm_insert_epi16(vF, -open, 0);
            vFM = _mm_slli_si128(vFM, 2);
            vFL = _mm_slli_si128(vFL, 2);
            for (i=0; i<segLen; ++i) {
                __m128i case1not;
                __m128i case2not;
                __m128i case2;

                /* need to know where match and length come from so
                 * recompute the cases as in the main loop */
                vHp = _mm_adds_epi16(vHp, _mm_load_si128(vP + i));
                vE = _mm_load_si128(pvELoad + i);
                case1not = _mm_or_si128(
                        _mm_cmplt_epi16(vHp,vF),_mm_cmplt_epi16(vHp,vE));
                case2not = _mm_cmplt_epi16(vF,vE);
                case2 = _mm_andnot_si128(case2not,case1not);

                vHM = _mm_load_si128(pvHMStore + i);
                vHM = _mm_andnot_si128(case2, vHM);
                vHM = _mm_or_si128(vHM, _mm_and_si128(case2, vFM));
                _mm_store_si128(pvHMStore + i, vHM);
                _mm_store_si128(pvEM + i, vHM);

                vHL = _mm_load_si128(pvHLStore + i);
                vHL = _mm_andnot_si128(case2, vHL);
                vHL = _mm_or_si128(vHL, _mm_and_si128(case2,
                            _mm_add_epi16(vFL,vOne)));
                _mm_store_si128(pvHLStore + i, vHL);
                _mm_store_si128(pvEL + i, vHL);

                vH = _mm_load_si128(pvHStore + i);
                vH = _mm_max_epi16(vH,vF);
                _mm_store_si128(pvHStore + i, vH);
#ifdef PARASAIL_TABLE
                arr_store_si128(match_table, vHM, i, segLen, j, s2Len);
                arr_store_si128(length_table, vHL, i, segLen, j, s2Len);
                arr_store_si128(score_table, vH, i, segLen, j, s2Len);
#endif
                vH = _mm_subs_epi16(vH, vGapO);
                vF = _mm_subs_epi16(vF, vGapE);
                if (! _mm_movemask_epi8(_mm_cmpgt_epi16(vF, vH))) goto end;
                vF = _mm_max_epi16(vF, vH);
                vFM = vHM;
                vFL = vHL;
                vHp = _mm_load_si128(pvHLoad + i);
            }
        }
end:
        {
            int16_t tmp;
            /* extract last value from the column */
            vH = _mm_load_si128(pvHStore + offset);
            vHM = _mm_load_si128(pvHMStore + offset);
            vHL = _mm_load_si128(pvHLStore + offset);
            for (k=0; k<position; ++k) {
                vH = _mm_slli_si128 (vH, 2);
                vHM = _mm_slli_si128 (vHM, 2);
                vHL = _mm_slli_si128 (vHL, 2);
            }
            /* max of last value in each column */
            tmp = (int16_t) _mm_extract_epi16 (vH, 7);
            if (tmp > max) {
                max = tmp;
                *matches = (int16_t)_mm_extract_epi16(vHM, 7);
                *length = (int16_t)_mm_extract_epi16(vHL, 7);
            }
        }
    }

    /* max of last column */
    {
        __m128i vNegInf = _mm_set1_epi16(NEG_INF_16);
        __m128i vMaxLastColH = vNegInf;
        __m128i vMaxLastColHM = vNegInf;
        __m128i vMaxLastColHL = vNegInf;
        __m128i vQIndex = _mm_set_epi16(
                7*segLen,
                6*segLen,
                5*segLen,
                4*segLen,
                3*segLen,
                2*segLen,
                1*segLen,
                0*segLen);
        __m128i vQLimit = _mm_set1_epi16(s1Len);

        for (i=0; i<segLen; ++i) {
            /* load the last stored values */
            __m128i vH = _mm_load_si128(pvHStore + i);
            __m128i vHM = _mm_load_si128(pvHMStore + i);
            __m128i vHL = _mm_load_si128(pvHLStore + i);
            /* mask off the values that were padded */
            __m128i cond_lmt = _mm_cmplt_epi16(vQIndex, vQLimit);
            __m128i cond_max = _mm_cmpgt_epi16(vH, vMaxLastColH);
            __m128i cond_all = _mm_and_si128(cond_max, cond_lmt);
            vMaxLastColH = _mm_andnot_si128(cond_all, vMaxLastColH);
            vMaxLastColH = _mm_or_si128(vMaxLastColH,
                    _mm_and_si128(cond_all, vH));
            vMaxLastColHM = _mm_andnot_si128(cond_all, vMaxLastColHM);
            vMaxLastColHM = _mm_or_si128(vMaxLastColHM,
                    _mm_and_si128(cond_all, vHM));
            vMaxLastColHL = _mm_andnot_si128(cond_all, vMaxLastColHL);
            vMaxLastColHL = _mm_or_si128(vMaxLastColHL,
                    _mm_and_si128(cond_all, vHL));
            vQIndex = _mm_adds_epi16(vQIndex, vOne);
        }

        /* max in vec */
        for (j=0; j<8; ++j) {
            int16_t value = (int16_t) _mm_extract_epi16(vMaxLastColH, 7);
            if (value > max) {
                max = value;
                *matches = (int16_t)_mm_extract_epi16(vMaxLastColHM, 7);
                *length = (int16_t)_mm_extract_epi16(vMaxLastColHL, 7);
            }
            vMaxLastColH = _mm_slli_si128(vMaxLastColH, 2);
            vMaxLastColHM = _mm_slli_si128(vMaxLastColHM, 2);
            vMaxLastColHL = _mm_slli_si128(vMaxLastColHL, 2);
        }
    }

    free(vProfile);
    free(vProfileS);
    free(pvHStore);
    free(pvHLoad);
    free(pvHMStore);
    free(pvHMLoad);
    free(pvHLStore);
    free(pvHLLoad);
    free(pvEStore);
    free(pvELoad);
    free(pvEM);
    free(pvEL);

    return max;
}
