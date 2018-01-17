/*
 * DMdeconvolution.h
 *
 * Code generation for function 'DMdeconvolution'
 *
 * C source code generated on: Tue Jan 31 00:56:15 2017
 *
 */

#ifndef __DMDECONVOLUTION_H__
#define __DMDECONVOLUTION_H__
/* Include files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "rtwtypes.h"
#include "DMdeconvolution_types.h"

/* Function Declarations */

/*  Deconvolution multiexponentioal decayes to spectrum of relaxation times */
/*  The implicit iterative scheme for Tikhonov's method */
/*  with an importation of the prior information on a nonnegativity */
/*  and zero smoothnesses of spectrum */
/* 	varargin: */
/*       Ampitudes, Ticks - a one-dimensional massifs (vectors)of the same size */
/*       T2 - a vector of relaxation times */
/*       Iterations - a number of iterations in solution (regularization parameter)  */
/*  varargout: */
/*       Spectrum - vectors of distribution of relaxation times */  
/*  16/12/2016 m_dorogin@bk.ru */
extern void DMdeconvolution(const emxArray_real_T *Amplitudes, const emxArray_real_T *Ticks, const emxArray_real_T *T2, int32_T Iterations, int32_T Alpha, emxArray_real_T *Spectrum);

#endif
/* End of code generation (DMdeconvolution.h) */
