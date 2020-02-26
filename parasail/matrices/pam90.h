/**
 * @file
 *
 * @author jeffrey.daily@gmail.com
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 *
 * This file was converted to C code from the raw file found at
 * ftp://ftp.cbi.pku.edu.cn/pub/software/blast/matrices/PAM90, the
 * Center for Bioinformatics, Peking University, China.
 */
#ifndef _PARASAIL_PAM90_H_
#define _PARASAIL_PAM90_H_

#include "parasail.h"
#include "pam_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/* # */
/* # This matrix was produced by "pam" Version 1.0.6 [28-Jul-93] */
/* # */
/* # PAM 90 substitution matrix, scale = ln(2)/2 = 0.346574 */
/* # */
/* # Expected score = -2.26, Entropy = 1.30 bits */
/* # */
/* # Lowest score = -10, Highest score = 13 */
/* # */

static const int parasail_pam90_[] = {
/*        A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */   4, -4, -1, -1, -3, -2,  0,  0, -4, -2, -3, -3, -2, -5,  0,  1,  1, -8, -5,  0, -1, -1, -1,-10,
/* R */  -4,  7, -2, -5, -5,  0, -4, -5,  1, -3, -5,  2, -2, -6, -1, -1, -3,  0, -6, -4, -3, -1, -2,-10,
/* N */  -1, -2,  5,  3, -6, -1,  0, -1,  2, -3, -4,  1, -4, -5, -2,  1,  0, -5, -2, -4,  4,  0, -1,-10,
/* D */  -1, -5,  3,  6, -8,  0,  4, -1, -1, -4, -7, -2, -5, -8, -4, -1, -2, -9, -6, -4,  5,  3, -2,-10,
/* C */  -3, -5, -6, -8,  9, -8, -8, -5, -5, -3, -9, -8, -8, -7, -5, -1, -4,-10, -1, -3, -7, -8, -5,-10,
/* Q */  -2,  0, -1,  0, -8,  6,  2, -3,  2, -4, -3, -1, -2, -7, -1, -2, -3, -7, -6, -4,  0,  5, -2,-10,
/* E */   0, -4,  0,  4, -8,  2,  6, -1, -1, -3, -5, -2, -4, -8, -2, -2, -2,-10, -6, -3,  2,  5, -2,-10,
/* G */   0, -5, -1, -1, -5, -3, -1,  5, -5, -5, -6, -4, -5, -6, -3,  0, -2, -9, -8, -3, -1, -2, -2,-10,
/* H */  -4,  1,  2, -1, -5,  2, -1, -5,  8, -5, -3, -2, -5, -3, -2, -3, -3, -4, -1, -4,  1,  1, -2,-10,
/* I */  -2, -3, -3, -4, -3, -4, -3, -5, -5,  6,  1, -3,  1,  0, -4, -3,  0, -8, -3,  3, -3, -3, -2,-10,
/* L */  -3, -5, -4, -7, -9, -3, -5, -6, -3,  1,  6, -5,  2,  0, -4, -5, -3, -3, -3,  0, -5, -4, -3,-10,
/* K */  -3,  2,  1, -2, -8, -1, -2, -4, -2, -3, -5,  5,  0, -8, -3, -1, -1, -6, -6, -5,  0, -1, -2,-10,
/* M */  -2, -2, -4, -5, -8, -2, -4, -5, -5,  1,  2,  0,  9, -1, -4, -3, -2, -7, -6,  1, -5, -3, -2,-10,
/* F */  -5, -6, -5, -8, -7, -7, -8, -6, -3,  0,  0, -8, -1,  8, -6, -4, -5, -2,  4, -4, -6, -8, -4,-10,
/* P */   0, -1, -2, -4, -5, -1, -2, -3, -2, -4, -4, -3, -4, -6,  7,  0, -1, -8, -8, -3, -3, -2, -2,-10,
/* S */   1, -1,  1, -1, -1, -2, -2,  0, -3, -3, -5, -1, -3, -4,  0,  4,  2, -3, -4, -3,  0, -2, -1,-10,
/* T */   1, -3,  0, -2, -4, -3, -2, -2, -3,  0, -3, -1, -2, -5, -1,  2,  5, -7, -4, -1, -1, -2, -1,-10,
/* W */  -8,  0, -5, -9,-10, -7,-10, -9, -4, -8, -3, -6, -7, -2, -8, -3, -7, 13, -2, -9, -7, -8, -6,-10,
/* Y */  -5, -6, -2, -6, -1, -6, -6, -8, -1, -3, -3, -6, -6,  4, -8, -4, -4, -2,  9, -4, -4, -6, -4,-10,
/* V */   0, -4, -4, -4, -3, -4, -3, -3, -4,  3,  0, -5,  1, -4, -3, -3, -1, -9, -4,  6, -4, -3, -2,-10,
/* B */  -1, -3,  4,  5, -7,  0,  2, -1,  1, -3, -5,  0, -5, -6, -3,  0, -1, -7, -4, -4,  4,  2, -2,-10,
/* Z */  -1, -1,  0,  3, -8,  5,  5, -2,  1, -3, -4, -1, -3, -8, -2, -2, -2, -8, -6, -3,  2,  5, -2,-10,
/* X */  -1, -2, -1, -2, -5, -2, -2, -2, -2, -2, -3, -2, -2, -4, -2, -1, -1, -6, -4, -2, -2, -2, -2,-10,
/* * */ -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,  1
};

static const parasail_matrix_t parasail_pam90 = {
    "pam90",
    parasail_pam90_,
    parasail_pam_map,
    24,
    13,
    -10,
    NULL
};

#ifdef __cplusplus
}
#endif

#endif /* _PARASAIL_PAM90_H_ */

