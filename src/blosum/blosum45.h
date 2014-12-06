/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2014 Battelle Memorial Institute.
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#ifndef _PARASAIL_BLOSUM45_H_
#define _PARASAIL_BLOSUM45_H_

static const int8_t blosum45__[] = {
/*       A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */  5, -2, -1, -2, -1, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -2, -2,  0, -1, -1,  0, -5,
/* R */ -2,  7,  0, -1, -3,  1,  0, -2,  0, -3, -2,  3, -1, -2, -2, -1, -1, -2, -1, -2, -1,  0, -1, -5,
/* N */ -1,  0,  6,  2, -2,  0,  0,  0,  1, -2, -3,  0, -2, -2, -2,  1,  0, -4, -2, -3,  4,  0, -1, -5,
/* D */ -2, -1,  2,  7, -3,  0,  2, -1,  0, -4, -3,  0, -3, -4, -1,  0, -1, -4, -2, -3,  5,  1, -1, -5,
/* C */ -1, -3, -2, -3, 12, -3, -3, -3, -3, -3, -2, -3, -2, -2, -4, -1, -1, -5, -3, -1, -2, -3, -2, -5,
/* Q */ -1,  1,  0,  0, -3,  6,  2, -2,  1, -2, -2,  1,  0, -4, -1,  0, -1, -2, -1, -3,  0,  4, -1, -5,
/* E */ -1,  0,  0,  2, -3,  2,  6, -2,  0, -3, -2,  1, -2, -3,  0,  0, -1, -3, -2, -3,  1,  4, -1, -5,
/* G */  0, -2,  0, -1, -3, -2, -2,  7, -2, -4, -3, -2, -2, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -5,
/* H */ -2,  0,  1,  0, -3,  1,  0, -2, 10, -3, -2, -1,  0, -2, -2, -1, -2, -3,  2, -3,  0,  0, -1, -5,
/* I */ -1, -3, -2, -4, -3, -2, -3, -4, -3,  5,  2, -3,  2,  0, -2, -2, -1, -2,  0,  3, -3, -3, -1, -5,
/* L */ -1, -2, -3, -3, -2, -2, -2, -3, -2,  2,  5, -3,  2,  1, -3, -3, -1, -2,  0,  1, -3, -2, -1, -5,
/* K */ -1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -3,  5, -1, -3, -1, -1, -1, -2, -1, -2,  0,  1, -1, -5,
/* M */ -1, -1, -2, -3, -2,  0, -2, -2,  0,  2,  2, -1,  6,  0, -2, -2, -1, -2,  0,  1, -2, -1, -1, -5,
/* F */ -2, -2, -2, -4, -2, -4, -3, -3, -2,  0,  1, -3,  0,  8, -3, -2, -1,  1,  3,  0, -3, -3, -1, -5,
/* P */ -1, -2, -2, -1, -4, -1,  0, -2, -2, -2, -3, -1, -2, -3,  9, -1, -1, -3, -3, -3, -2, -1, -1, -5,
/* S */  1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -3, -1, -2, -2, -1,  4,  2, -4, -2, -1,  0,  0,  0, -5,
/* T */  0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -1, -1,  2,  5, -3, -1,  0,  0, -1,  0, -5,
/* W */ -2, -2, -4, -4, -5, -2, -3, -2, -3, -2, -2, -2, -2,  1, -3, -4, -3, 15,  3, -3, -4, -2, -2, -5,
/* Y */ -2, -1, -2, -2, -3, -1, -2, -3,  2,  0,  0, -1,  0,  3, -3, -2, -1,  3,  8, -1, -2, -2, -1, -5,
/* V */  0, -2, -3, -3, -1, -3, -3, -3, -3,  3,  1, -2,  1,  0, -3, -1,  0, -3, -1,  5, -3, -3, -1, -5,
/* B */ -1, -1,  4,  5, -2,  0,  1, -1,  0, -3, -3,  0, -2, -3, -2,  0,  0, -4, -2, -3,  4,  2, -1, -5,
/* Z */ -1,  0,  0,  1, -3,  4,  4, -2,  0, -3, -2,  1, -1, -3, -1,  0, -1, -2, -2, -3,  2,  4, -1, -5,
/* X */  0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -2, -1, -1, -1, -1, -1, -5,
/* * */ -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,  1
};

static const int blosum45[24][24] = {
/*       A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */{ 5, -2, -1, -2, -1, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -2, -2,  0, -1, -1,  0, -5},
/* R */{-2,  7,  0, -1, -3,  1,  0, -2,  0, -3, -2,  3, -1, -2, -2, -1, -1, -2, -1, -2, -1,  0, -1, -5},
/* N */{-1,  0,  6,  2, -2,  0,  0,  0,  1, -2, -3,  0, -2, -2, -2,  1,  0, -4, -2, -3,  4,  0, -1, -5},
/* D */{-2, -1,  2,  7, -3,  0,  2, -1,  0, -4, -3,  0, -3, -4, -1,  0, -1, -4, -2, -3,  5,  1, -1, -5},
/* C */{-1, -3, -2, -3, 12, -3, -3, -3, -3, -3, -2, -3, -2, -2, -4, -1, -1, -5, -3, -1, -2, -3, -2, -5},
/* Q */{-1,  1,  0,  0, -3,  6,  2, -2,  1, -2, -2,  1,  0, -4, -1,  0, -1, -2, -1, -3,  0,  4, -1, -5},
/* E */{-1,  0,  0,  2, -3,  2,  6, -2,  0, -3, -2,  1, -2, -3,  0,  0, -1, -3, -2, -3,  1,  4, -1, -5},
/* G */{ 0, -2,  0, -1, -3, -2, -2,  7, -2, -4, -3, -2, -2, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -5},
/* H */{-2,  0,  1,  0, -3,  1,  0, -2, 10, -3, -2, -1,  0, -2, -2, -1, -2, -3,  2, -3,  0,  0, -1, -5},
/* I */{-1, -3, -2, -4, -3, -2, -3, -4, -3,  5,  2, -3,  2,  0, -2, -2, -1, -2,  0,  3, -3, -3, -1, -5},
/* L */{-1, -2, -3, -3, -2, -2, -2, -3, -2,  2,  5, -3,  2,  1, -3, -3, -1, -2,  0,  1, -3, -2, -1, -5},
/* K */{-1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -3,  5, -1, -3, -1, -1, -1, -2, -1, -2,  0,  1, -1, -5},
/* M */{-1, -1, -2, -3, -2,  0, -2, -2,  0,  2,  2, -1,  6,  0, -2, -2, -1, -2,  0,  1, -2, -1, -1, -5},
/* F */{-2, -2, -2, -4, -2, -4, -3, -3, -2,  0,  1, -3,  0,  8, -3, -2, -1,  1,  3,  0, -3, -3, -1, -5},
/* P */{-1, -2, -2, -1, -4, -1,  0, -2, -2, -2, -3, -1, -2, -3,  9, -1, -1, -3, -3, -3, -2, -1, -1, -5},
/* S */{ 1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -3, -1, -2, -2, -1,  4,  2, -4, -2, -1,  0,  0,  0, -5},
/* T */{ 0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -1, -1,  2,  5, -3, -1,  0,  0, -1,  0, -5},
/* W */{-2, -2, -4, -4, -5, -2, -3, -2, -3, -2, -2, -2, -2,  1, -3, -4, -3, 15,  3, -3, -4, -2, -2, -5},
/* Y */{-2, -1, -2, -2, -3, -1, -2, -3,  2,  0,  0, -1,  0,  3, -3, -2, -1,  3,  8, -1, -2, -2, -1, -5},
/* V */{ 0, -2, -3, -3, -1, -3, -3, -3, -3,  3,  1, -2,  1,  0, -3, -1,  0, -3, -1,  5, -3, -3, -1, -5},
/* B */{-1, -1,  4,  5, -2,  0,  1, -1,  0, -3, -3,  0, -2, -3, -2,  0,  0, -4, -2, -3,  4,  2, -1, -5},
/* Z */{-1,  0,  0,  1, -3,  4,  4, -2,  0, -3, -2,  1, -1, -3, -1,  0, -1, -2, -2, -3,  2,  4, -1, -5},
/* X */{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -2, -1, -1, -1, -1, -1, -5},
/* * */{-5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,  1}
};

/*                                     A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
static const int b45_01[24] = /* A */{ 5, -2, -1, -2, -1, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -2, -2,  0, -1, -1,  0, -5};
static const int b45_02[24] = /* R */{-2,  7,  0, -1, -3,  1,  0, -2,  0, -3, -2,  3, -1, -2, -2, -1, -1, -2, -1, -2, -1,  0, -1, -5};
static const int b45_03[24] = /* N */{-1,  0,  6,  2, -2,  0,  0,  0,  1, -2, -3,  0, -2, -2, -2,  1,  0, -4, -2, -3,  4,  0, -1, -5};
static const int b45_04[24] = /* D */{-2, -1,  2,  7, -3,  0,  2, -1,  0, -4, -3,  0, -3, -4, -1,  0, -1, -4, -2, -3,  5,  1, -1, -5};
static const int b45_05[24] = /* C */{-1, -3, -2, -3, 12, -3, -3, -3, -3, -3, -2, -3, -2, -2, -4, -1, -1, -5, -3, -1, -2, -3, -2, -5};
static const int b45_06[24] = /* Q */{-1,  1,  0,  0, -3,  6,  2, -2,  1, -2, -2,  1,  0, -4, -1,  0, -1, -2, -1, -3,  0,  4, -1, -5};
static const int b45_07[24] = /* E */{-1,  0,  0,  2, -3,  2,  6, -2,  0, -3, -2,  1, -2, -3,  0,  0, -1, -3, -2, -3,  1,  4, -1, -5};
static const int b45_08[24] = /* G */{ 0, -2,  0, -1, -3, -2, -2,  7, -2, -4, -3, -2, -2, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -5};
static const int b45_09[24] = /* H */{-2,  0,  1,  0, -3,  1,  0, -2, 10, -3, -2, -1,  0, -2, -2, -1, -2, -3,  2, -3,  0,  0, -1, -5};
static const int b45_10[24] = /* I */{-1, -3, -2, -4, -3, -2, -3, -4, -3,  5,  2, -3,  2,  0, -2, -2, -1, -2,  0,  3, -3, -3, -1, -5};
static const int b45_11[24] = /* L */{-1, -2, -3, -3, -2, -2, -2, -3, -2,  2,  5, -3,  2,  1, -3, -3, -1, -2,  0,  1, -3, -2, -1, -5};
static const int b45_12[24] = /* K */{-1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -3,  5, -1, -3, -1, -1, -1, -2, -1, -2,  0,  1, -1, -5};
static const int b45_13[24] = /* M */{-1, -1, -2, -3, -2,  0, -2, -2,  0,  2,  2, -1,  6,  0, -2, -2, -1, -2,  0,  1, -2, -1, -1, -5};
static const int b45_14[24] = /* F */{-2, -2, -2, -4, -2, -4, -3, -3, -2,  0,  1, -3,  0,  8, -3, -2, -1,  1,  3,  0, -3, -3, -1, -5};
static const int b45_15[24] = /* P */{-1, -2, -2, -1, -4, -1,  0, -2, -2, -2, -3, -1, -2, -3,  9, -1, -1, -3, -3, -3, -2, -1, -1, -5};
static const int b45_16[24] = /* S */{ 1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -3, -1, -2, -2, -1,  4,  2, -4, -2, -1,  0,  0,  0, -5};
static const int b45_17[24] = /* T */{ 0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -1, -1,  2,  5, -3, -1,  0,  0, -1,  0, -5};
static const int b45_18[24] = /* W */{-2, -2, -4, -4, -5, -2, -3, -2, -3, -2, -2, -2, -2,  1, -3, -4, -3, 15,  3, -3, -4, -2, -2, -5};
static const int b45_19[24] = /* Y */{-2, -1, -2, -2, -3, -1, -2, -3,  2,  0,  0, -1,  0,  3, -3, -2, -1,  3,  8, -1, -2, -2, -1, -5};
static const int b45_20[24] = /* V */{ 0, -2, -3, -3, -1, -3, -3, -3, -3,  3,  1, -2,  1,  0, -3, -1,  0, -3, -1,  5, -3, -3, -1, -5};
static const int b45_21[24] = /* B */{-1, -1,  4,  5, -2,  0,  1, -1,  0, -3, -3,  0, -2, -3, -2,  0,  0, -4, -2, -3,  4,  2, -1, -5};
static const int b45_22[24] = /* Z */{-1,  0,  0,  1, -3,  4,  4, -2,  0, -3, -2,  1, -1, -3, -1,  0, -1, -2, -2, -3,  2,  4, -1, -5};
static const int b45_23[24] = /* X */{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -2, -1, -1, -1, -1, -1, -5};
static const int b45_24[24] = /* * */{-5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5,  1};
static const int * const restrict blosum45_[24] = {
b45_01,
b45_02,
b45_03,
b45_04,
b45_05,
b45_06,
b45_07,
b45_08,
b45_09,
b45_10,
b45_11,
b45_12,
b45_13,
b45_14,
b45_15,
b45_16,
b45_17,
b45_18,
b45_19,
b45_20,
b45_21,
b45_22,
b45_23,
b45_24
};

#endif /* _PARASAIL_BLOSUM45_H_ */
