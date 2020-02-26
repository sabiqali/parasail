/**
 * @file
 *
 * @author jeffrey.daily@gmail.com
 *
 * Copyright (c) 2015 Battelle Memorial Institute.
 *
 * This file was converted to C code from the raw file found at
 * ftp://ftp.cbi.pku.edu.cn/pub/software/blast/matrices/PAM110, the
 * Center for Bioinformatics, Peking University, China.
 */
#ifndef _PARASAIL_PAM110_H_
#define _PARASAIL_PAM110_H_

#include "parasail.h"
#include "pam_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/* # */
/* # This matrix was produced by "pam" Version 1.0.6 [28-Jul-93] */
/* # */
/* # PAM 110 substitution matrix, scale = ln(2)/2 = 0.346574 */
/* # */
/* # Expected score = -1.81, Entropy = 1.07 bits */
/* # */
/* # Lowest score = -9, Highest score = 12 */
/* # */

static const int parasail_pam110_[] = {
/*        A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */   3, -3, -1, -1, -3, -1,  0,  1, -3, -1, -3, -3, -2, -4,  1,  1,  1, -7, -4,  0, -1, -1, -1, -9,
/* R */  -3,  7, -1, -4, -4,  1, -3, -4,  1, -3, -4,  2, -1, -5, -1, -1, -2,  1, -5, -4, -2, -1, -2, -9,
/* N */  -1, -1,  4,  2, -5,  0,  1,  0,  2, -2, -4,  1, -3, -4, -2,  1,  0, -5, -2, -3,  3,  0, -1, -9,
/* D */  -1, -4,  2,  5, -7,  1,  4,  0,  0, -3, -6, -1, -5, -7, -3, -1, -1, -8, -5, -4,  4,  3, -2, -9,
/* C */  -3, -4, -5, -7,  9, -7, -7, -5, -4, -3, -8, -7, -7, -6, -4, -1, -3, -9, -1, -3, -6, -7, -4, -9,
/* Q */  -1,  1,  0,  1, -7,  6,  2, -3,  3, -3, -2,  0, -1, -6,  0, -2, -2, -6, -6, -3,  0,  4, -1, -9,
/* E */   0, -3,  1,  4, -7,  2,  5, -1, -1, -3, -5, -1, -3, -7, -2, -1, -2, -9, -5, -3,  3,  4, -2, -9,
/* G */   1, -4,  0,  0, -5, -3, -1,  5, -4, -4, -6, -3, -4, -5, -2,  0, -2, -8, -7, -2,  0, -2, -2, -9,
/* H */  -3,  1,  2,  0, -4,  3, -1, -4,  7, -4, -3, -2, -4, -3, -1, -2, -3, -4, -1, -3,  1,  1, -2, -9,
/* I */  -1, -3, -2, -3, -3, -3, -3, -4, -4,  6,  1, -3,  1,  0, -4, -3,  0, -7, -2,  3, -3, -3, -2, -9,
/* L */  -3, -4, -4, -6, -8, -2, -5, -6, -3,  1,  6, -4,  3,  0, -4, -4, -3, -3, -3,  1, -5, -3, -2, -9,
/* K */  -3,  2,  1, -1, -7,  0, -1, -3, -2, -3, -4,  5,  0, -7, -3, -1, -1, -5, -5, -4,  0, -1, -2, -9,
/* M */  -2, -1, -3, -5, -7, -1, -3, -4, -4,  1,  3,  0,  8, -1, -4, -2, -1, -6, -5,  1, -4, -2, -2, -9,
/* F */  -4, -5, -4, -7, -6, -6, -7, -5, -3,  0,  0, -7, -1,  8, -6, -4, -4, -1,  4, -3, -6, -7, -4, -9,
/* P */   1, -1, -2, -3, -4,  0, -2, -2, -1, -4, -4, -3, -4, -6,  6,  0, -1, -7, -7, -2, -2, -1, -2, -9,
/* S */   1, -1,  1, -1, -1, -2, -1,  0, -2, -3, -4, -1, -2, -4,  0,  3,  2, -3, -3, -2,  0, -1, -1, -9,
/* T */   1, -2,  0, -1, -3, -2, -2, -2, -3,  0, -3, -1, -1, -4, -1,  2,  5, -6, -3,  0, -1, -2, -1, -9,
/* W */  -7,  1, -5, -8, -9, -6, -9, -8, -4, -7, -3, -5, -6, -1, -7, -3, -6, 12, -2, -8, -6, -7, -5, -9,
/* Y */  -4, -5, -2, -5, -1, -6, -5, -7, -1, -2, -3, -5, -5,  4, -7, -3, -3, -2,  8, -4, -3, -5, -3, -9,
/* V */   0, -4, -3, -4, -3, -3, -3, -2, -3,  3,  1, -4,  1, -3, -2, -2,  0, -8, -4,  5, -3, -3, -2, -9,
/* B */  -1, -2,  3,  4, -6,  0,  3,  0,  1, -3, -5,  0, -4, -6, -2,  0, -1, -6, -3, -3,  4,  2, -1, -9,
/* Z */  -1, -1,  0,  3, -7,  4,  4, -2,  1, -3, -3, -1, -2, -7, -1, -1, -2, -7, -5, -3,  2,  4, -2, -9,
/* X */  -1, -2, -1, -2, -4, -1, -2, -2, -2, -2, -2, -2, -2, -4, -2, -1, -1, -5, -3, -2, -1, -2, -2, -9,
/* * */  -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9,  1
};

static const parasail_matrix_t parasail_pam110 = {
    "pam110",
    parasail_pam110_,
    parasail_pam_map,
    24,
    12,
    -9,
    NULL
};

#ifdef __cplusplus
}
#endif

#endif /* _PARASAIL_PAM110_H_ */

