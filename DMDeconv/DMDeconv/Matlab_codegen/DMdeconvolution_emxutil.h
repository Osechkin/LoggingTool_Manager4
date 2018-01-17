/*
 * DMdeconvolution_emxutil.h
 *
 * Code generation for function 'DMdeconvolution_emxutil'
 *
 * C source code generated on: Tue Jan 31 00:56:15 2017
 *
 */

#ifndef __DMDECONVOLUTION_EMXUTIL_H__
#define __DMDECONVOLUTION_EMXUTIL_H__
/* Include files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "rtwtypes.h"
#include "DMdeconvolution_types.h"

/* Function Declarations */
extern void b_emxInit_real_T(emxArray_real_T **pEmxArray, int32_T numDimensions);
extern void emxEnsureCapacity(emxArray__common *emxArray, int32_T oldNumel, int32_T elementSize);
extern void emxFree_int32_T(emxArray_int32_T **pEmxArray);
extern void emxFree_real_T(emxArray_real_T **pEmxArray);
extern void emxInit_int32_T(emxArray_int32_T **pEmxArray, int32_T numDimensions);
extern void emxInit_real_T(emxArray_real_T **pEmxArray, int32_T numDimensions);
#endif
/* End of code generation (DMdeconvolution_emxutil.h) */
