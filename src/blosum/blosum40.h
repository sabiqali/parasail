/**
 * @file
 *
 * @author jeff.daily@pnnl.gov
 *
 * Copyright (c) 2014 Battelle Memorial Institute.
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#ifndef _PARASAIL_BLOSUM40_H_
#define _PARASAIL_BLOSUM40_H_

static const int8_t blosum40__[] = {
/*       A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */  5, -2, -1, -1, -2,  0, -1,  1, -2, -1, -2, -1, -1, -3, -2,  1,  0, -3, -2,  0, -1, -1,  0, -6, 
/* R */ -2,  9,  0, -1, -3,  2, -1, -3,  0, -3, -2,  3, -1, -2, -3, -1, -2, -2, -1, -2, -1,  0, -1, -6, 
/* N */ -1,  0,  8,  2, -2,  1, -1,  0,  1, -2, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  4,  0, -1, -6, 
/* D */ -1, -1,  2,  9, -2, -1,  2, -2,  0, -4, -3,  0, -3, -4, -2,  0, -1, -5, -3, -3,  6,  1, -1, -6, 
/* C */ -2, -3, -2, -2, 16, -4, -2, -3, -4, -4, -2, -3, -3, -2, -5, -1, -1, -6, -4, -2, -2, -3, -2, -6, 
/* Q */  0,  2,  1, -1, -4,  8,  2, -2,  0, -3, -2,  1, -1, -4, -2,  1, -1, -1, -1, -3,  0,  4, -1, -6, 
/* E */ -1, -1, -1,  2, -2,  2,  7, -3,  0, -4, -2,  1, -2, -3,  0,  0, -1, -2, -2, -3,  1,  5, -1, -6, 
/* G */  1, -3,  0, -2, -3, -2, -3,  8, -2, -4, -4, -2, -2, -3, -1,  0, -2, -2, -3, -4, -1, -2, -1, -6, 
/* H */ -2,  0,  1,  0, -4,  0,  0, -2, 13, -3, -2, -1,  1, -2, -2, -1, -2, -5,  2, -4,  0,  0, -1, -6, 
/* I */ -1, -3, -2, -4, -4, -3, -4, -4, -3,  6,  2, -3,  1,  1, -2, -2, -1, -3,  0,  4, -3, -4, -1, -6, 
/* L */ -2, -2, -3, -3, -2, -2, -2, -4, -2,  2,  6, -2,  3,  2, -4, -3, -1, -1,  0,  2, -3, -2, -1, -6, 
/* K */ -1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -2,  6, -1, -3, -1,  0,  0, -2, -1, -2,  0,  1, -1, -6, 
/* M */ -1, -1, -2, -3, -3, -1, -2, -2,  1,  1,  3, -1,  7,  0, -2, -2, -1, -2,  1,  1, -3, -2,  0, -6, 
/* F */ -3, -2, -3, -4, -2, -4, -3, -3, -2,  1,  2, -3,  0,  9, -4, -2, -1,  1,  4,  0, -3, -4, -1, -6, 
/* P */ -2, -3, -2, -2, -5, -2,  0, -1, -2, -2, -4, -1, -2, -4, 11, -1,  0, -4, -3, -3, -2, -1, -2, -6, 
/* S */  1, -1,  1,  0, -1,  1,  0,  0, -1, -2, -3,  0, -2, -2, -1,  5,  2, -5, -2, -1,  0,  0,  0, -6, 
/* T */  0, -2,  0, -1, -1, -1, -1, -2, -2, -1, -1,  0, -1, -1,  0,  2,  6, -4, -1,  1,  0, -1,  0, -6, 
/* W */ -3, -2, -4, -5, -6, -1, -2, -2, -5, -3, -1, -2, -2,  1, -4, -5, -4, 19,  3, -3, -4, -2, -2, -6, 
/* Y */ -2, -1, -2, -3, -4, -1, -2, -3,  2,  0,  0, -1,  1,  4, -3, -2, -1,  3,  9, -1, -3, -2, -1, -6, 
/* V */  0, -2, -3, -3, -2, -3, -3, -4, -4,  4,  2, -2,  1,  0, -3, -1,  1, -3, -1,  5, -3, -3, -1, -6, 
/* B */ -1, -1,  4,  6, -2,  0,  1, -1,  0, -3, -3,  0, -3, -3, -2,  0,  0, -4, -3, -3,  5,  2, -1, -6, 
/* Z */ -1,  0,  0,  1, -3,  4,  5, -2,  0, -4, -2,  1, -2, -4, -1,  0, -1, -2, -2, -3,  2,  5, -1, -6, 
/* X */  0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1,  0, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -6, 
/* * */ -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6,  1
};

static const int blosum40[24][24] = {
/*       A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
/* A */{ 5, -2, -1, -1, -2,  0, -1,  1, -2, -1, -2, -1, -1, -3, -2,  1,  0, -3, -2,  0, -1, -1,  0, -6}, 
/* R */{-2,  9,  0, -1, -3,  2, -1, -3,  0, -3, -2,  3, -1, -2, -3, -1, -2, -2, -1, -2, -1,  0, -1, -6}, 
/* N */{-1,  0,  8,  2, -2,  1, -1,  0,  1, -2, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  4,  0, -1, -6}, 
/* D */{-1, -1,  2,  9, -2, -1,  2, -2,  0, -4, -3,  0, -3, -4, -2,  0, -1, -5, -3, -3,  6,  1, -1, -6}, 
/* C */{-2, -3, -2, -2, 16, -4, -2, -3, -4, -4, -2, -3, -3, -2, -5, -1, -1, -6, -4, -2, -2, -3, -2, -6}, 
/* Q */{ 0,  2,  1, -1, -4,  8,  2, -2,  0, -3, -2,  1, -1, -4, -2,  1, -1, -1, -1, -3,  0,  4, -1, -6}, 
/* E */{-1, -1, -1,  2, -2,  2,  7, -3,  0, -4, -2,  1, -2, -3,  0,  0, -1, -2, -2, -3,  1,  5, -1, -6}, 
/* G */{ 1, -3,  0, -2, -3, -2, -3,  8, -2, -4, -4, -2, -2, -3, -1,  0, -2, -2, -3, -4, -1, -2, -1, -6}, 
/* H */{-2,  0,  1,  0, -4,  0,  0, -2, 13, -3, -2, -1,  1, -2, -2, -1, -2, -5,  2, -4,  0,  0, -1, -6}, 
/* I */{-1, -3, -2, -4, -4, -3, -4, -4, -3,  6,  2, -3,  1,  1, -2, -2, -1, -3,  0,  4, -3, -4, -1, -6}, 
/* L */{-2, -2, -3, -3, -2, -2, -2, -4, -2,  2,  6, -2,  3,  2, -4, -3, -1, -1,  0,  2, -3, -2, -1, -6}, 
/* K */{-1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -2,  6, -1, -3, -1,  0,  0, -2, -1, -2,  0,  1, -1, -6}, 
/* M */{-1, -1, -2, -3, -3, -1, -2, -2,  1,  1,  3, -1,  7,  0, -2, -2, -1, -2,  1,  1, -3, -2,  0, -6}, 
/* F */{-3, -2, -3, -4, -2, -4, -3, -3, -2,  1,  2, -3,  0,  9, -4, -2, -1,  1,  4,  0, -3, -4, -1, -6}, 
/* P */{-2, -3, -2, -2, -5, -2,  0, -1, -2, -2, -4, -1, -2, -4, 11, -1,  0, -4, -3, -3, -2, -1, -2, -6}, 
/* S */{ 1, -1,  1,  0, -1,  1,  0,  0, -1, -2, -3,  0, -2, -2, -1,  5,  2, -5, -2, -1,  0,  0,  0, -6}, 
/* T */{ 0, -2,  0, -1, -1, -1, -1, -2, -2, -1, -1,  0, -1, -1,  0,  2,  6, -4, -1,  1,  0, -1,  0, -6}, 
/* W */{-3, -2, -4, -5, -6, -1, -2, -2, -5, -3, -1, -2, -2,  1, -4, -5, -4, 19,  3, -3, -4, -2, -2, -6}, 
/* Y */{-2, -1, -2, -3, -4, -1, -2, -3,  2,  0,  0, -1,  1,  4, -3, -2, -1,  3,  9, -1, -3, -2, -1, -6}, 
/* V */{ 0, -2, -3, -3, -2, -3, -3, -4, -4,  4,  2, -2,  1,  0, -3, -1,  1, -3, -1,  5, -3, -3, -1, -6}, 
/* B */{-1, -1,  4,  6, -2,  0,  1, -1,  0, -3, -3,  0, -3, -3, -2,  0,  0, -4, -3, -3,  5,  2, -1, -6}, 
/* Z */{-1,  0,  0,  1, -3,  4,  5, -2,  0, -4, -2,  1, -2, -4, -1,  0, -1, -2, -2, -3,  2,  5, -1, -6}, 
/* X */{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1,  0, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -6}, 
/* * */{-6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6,  1}
};

/*                                     A   R   N   D   C   Q   E   G   H   I   L   K   M   F   P   S   T   W   Y   V   B   Z   X   * */
static const int b40_01[24] = /* A */{ 5, -2, -1, -1, -2,  0, -1,  1, -2, -1, -2, -1, -1, -3, -2,  1,  0, -3, -2,  0, -1, -1,  0, -6}; 
static const int b40_02[24] = /* R */{-2,  9,  0, -1, -3,  2, -1, -3,  0, -3, -2,  3, -1, -2, -3, -1, -2, -2, -1, -2, -1,  0, -1, -6}; 
static const int b40_03[24] = /* N */{-1,  0,  8,  2, -2,  1, -1,  0,  1, -2, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  4,  0, -1, -6}; 
static const int b40_04[24] = /* D */{-1, -1,  2,  9, -2, -1,  2, -2,  0, -4, -3,  0, -3, -4, -2,  0, -1, -5, -3, -3,  6,  1, -1, -6}; 
static const int b40_05[24] = /* C */{-2, -3, -2, -2, 16, -4, -2, -3, -4, -4, -2, -3, -3, -2, -5, -1, -1, -6, -4, -2, -2, -3, -2, -6}; 
static const int b40_06[24] = /* Q */{ 0,  2,  1, -1, -4,  8,  2, -2,  0, -3, -2,  1, -1, -4, -2,  1, -1, -1, -1, -3,  0,  4, -1, -6}; 
static const int b40_07[24] = /* E */{-1, -1, -1,  2, -2,  2,  7, -3,  0, -4, -2,  1, -2, -3,  0,  0, -1, -2, -2, -3,  1,  5, -1, -6}; 
static const int b40_08[24] = /* G */{ 1, -3,  0, -2, -3, -2, -3,  8, -2, -4, -4, -2, -2, -3, -1,  0, -2, -2, -3, -4, -1, -2, -1, -6}; 
static const int b40_09[24] = /* H */{-2,  0,  1,  0, -4,  0,  0, -2, 13, -3, -2, -1,  1, -2, -2, -1, -2, -5,  2, -4,  0,  0, -1, -6}; 
static const int b40_10[24] = /* I */{-1, -3, -2, -4, -4, -3, -4, -4, -3,  6,  2, -3,  1,  1, -2, -2, -1, -3,  0,  4, -3, -4, -1, -6}; 
static const int b40_11[24] = /* L */{-2, -2, -3, -3, -2, -2, -2, -4, -2,  2,  6, -2,  3,  2, -4, -3, -1, -1,  0,  2, -3, -2, -1, -6}; 
static const int b40_12[24] = /* K */{-1,  3,  0,  0, -3,  1,  1, -2, -1, -3, -2,  6, -1, -3, -1,  0,  0, -2, -1, -2,  0,  1, -1, -6}; 
static const int b40_13[24] = /* M */{-1, -1, -2, -3, -3, -1, -2, -2,  1,  1,  3, -1,  7,  0, -2, -2, -1, -2,  1,  1, -3, -2,  0, -6}; 
static const int b40_14[24] = /* F */{-3, -2, -3, -4, -2, -4, -3, -3, -2,  1,  2, -3,  0,  9, -4, -2, -1,  1,  4,  0, -3, -4, -1, -6}; 
static const int b40_15[24] = /* P */{-2, -3, -2, -2, -5, -2,  0, -1, -2, -2, -4, -1, -2, -4, 11, -1,  0, -4, -3, -3, -2, -1, -2, -6}; 
static const int b40_16[24] = /* S */{ 1, -1,  1,  0, -1,  1,  0,  0, -1, -2, -3,  0, -2, -2, -1,  5,  2, -5, -2, -1,  0,  0,  0, -6}; 
static const int b40_17[24] = /* T */{ 0, -2,  0, -1, -1, -1, -1, -2, -2, -1, -1,  0, -1, -1,  0,  2,  6, -4, -1,  1,  0, -1,  0, -6}; 
static const int b40_18[24] = /* W */{-3, -2, -4, -5, -6, -1, -2, -2, -5, -3, -1, -2, -2,  1, -4, -5, -4, 19,  3, -3, -4, -2, -2, -6}; 
static const int b40_19[24] = /* Y */{-2, -1, -2, -3, -4, -1, -2, -3,  2,  0,  0, -1,  1,  4, -3, -2, -1,  3,  9, -1, -3, -2, -1, -6}; 
static const int b40_20[24] = /* V */{ 0, -2, -3, -3, -2, -3, -3, -4, -4,  4,  2, -2,  1,  0, -3, -1,  1, -3, -1,  5, -3, -3, -1, -6}; 
static const int b40_21[24] = /* B */{-1, -1,  4,  6, -2,  0,  1, -1,  0, -3, -3,  0, -3, -3, -2,  0,  0, -4, -3, -3,  5,  2, -1, -6}; 
static const int b40_22[24] = /* Z */{-1,  0,  0,  1, -3,  4,  5, -2,  0, -4, -2,  1, -2, -4, -1,  0, -1, -2, -2, -3,  2,  5, -1, -6}; 
static const int b40_23[24] = /* X */{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1,  0, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -6}; 
static const int b40_24[24] = /* * */{-6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6,  1};
static const int * const restrict blosum40_[24] = {
b40_01,
b40_02,
b40_03,
b40_04,
b40_05,
b40_06,
b40_07,
b40_08,
b40_09,
b40_10,
b40_11,
b40_12,
b40_13,
b40_14,
b40_15,
b40_16,
b40_17,
b40_18,
b40_19,
b40_20,
b40_21,
b40_22,
b40_23,
b40_24
};

#endif /* _PARASAIL_BLOSUM40_H_ */
