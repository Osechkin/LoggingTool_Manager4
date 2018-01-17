/*
 * DMdeconvolution.c
 *
 * Code generation for function 'DMdeconvolution'
 *
 * C source code generated on: Tue Jan 31 00:56:15 2017
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "DMdeconvolution.h"
#include "DMdeconvolution_emxutil.h"
#include "inv.h"

/* Function Definitions */
void DMdeconvolution(const emxArray_real_T *Amplitudes, const emxArray_real_T *Ticks, const emxArray_real_T *T2, int32_T Iterations, int32_T Alpha, emxArray_real_T *Spectrum)
{
  int32_T i0;
  int32_T ar;  
  int32_T k;  
  emxArray_real_T *L;
  int32_T i;
  emxArray_real_T *b;
  int32_T br;
  int32_T i1;
  emxArray_real_T *W;
  int32_T cr;
  uint32_T unnamed_idx_0;
  uint32_T unnamed_idx_1;
  int32_T ic;
  int32_T ib;
  int32_T ia;
  emxArray_real_T *B;
  emxArray_real_T *AmplitudesL;
  real_T u0;
  real_T u1;

  
  emxInit_real_T(&L, 2);

  /*  convolution matrix */
  
  
  L->size[0] = T2->size[0];  
  L->size[1] = Ticks->size[0];  
  i0 = L->size[0] * L->size[1]; 
  emxEnsureCapacity((emxArray__common *)L, i0, (int32_T)sizeof(real_T));
  
  for (i = 0; i < T2->size[0]; i++) 
  {
	  for (k = 0; k < Ticks->size[0]; k++) 
	  {
		  L->data[i + L->size[0] * k] = exp(-Ticks->data[k] / T2->data[i]);
	  }
  }

  emxInit_real_T(&b, 2);

  /*  Gauss symmetrization and regularization kernel  */  
  b->size[0] = L->size[1];
  b->size[1] = L->size[0];
  i0 = b->size[0] * b->size[1];
  emxEnsureCapacity((emxArray__common *)b, i0, (int32_T)sizeof(real_T));
  ar = L->size[0];
  for (i0 = 0; i0 < ar; i0++) 
  {
	  br = L->size[1];
	  for (i1 = 0; i1 < br; i1++) 
	  {
		  b->data[i1 + b->size[0] * i0] = L->data[i0 + L->size[0] * i1];
	  }
  }

  emxInit_real_T(&W, 2);
  if ((L->size[1] == 1) || (b->size[0] == 1)) 
  {
	  i0 = W->size[0] * W->size[1];
	  W->size[0] = L->size[0];
	  W->size[1] = b->size[1];
	  emxEnsureCapacity((emxArray__common *)W, i0, (int32_T)sizeof(real_T));
	  ar = L->size[0];
	  for (i0 = 0; i0 < ar; i0++) {
		  br = b->size[1];
		  for (i1 = 0; i1 < br; i1++) {
			  W->data[i0 + W->size[0] * i1] = 0.0;
			  k = L->size[1];
			  for (cr = 0; cr < k; cr++) {
				  W->data[i0 + W->size[0] * i1] += L->data[i0 + L->size[0] * cr] *
					  b->data[cr + b->size[0] * i1];
			  }
		  }
	  }
  } 
  else 
  {
	  unnamed_idx_0 = (uint32_T)L->size[0];
	  unnamed_idx_1 = (uint32_T)b->size[1];
	  
	  W->size[0] = (int32_T)unnamed_idx_0;	  
	  W->size[1] = (int32_T)unnamed_idx_1;
	  i0 = W->size[0] * W->size[1];	  
	  emxEnsureCapacity((emxArray__common *)W, i0, (int32_T)sizeof(real_T));
	  
	  if ((L->size[0] == 0) || (b->size[1] == 0)) 
	  {
	  } 
	  else 
	  {
		  k = L->size[0] * (b->size[1] - 1);
		  cr = 0;
		  while ((L->size[0] > 0) && (cr <= k)) 
		  {
			  i0 = cr + L->size[0];
			  for (ic = cr; ic + 1 <= i0; ic++) 
			  {
				  W->data[ic] = 0.0;
			  }

			  cr += L->size[0];
		  }

		  br = 0;
		  cr = 0;
		  while ((L->size[0] > 0) && (cr <= k)) 
		  {
			  ar = 0;
			  i0 = br + L->size[1];
			  for (ib = br; ib + 1 <= i0; ib++) 
			  {
				  if (b->data[ib] != 0.0) 
				  {
					  ia = ar;
					  i1 = cr + L->size[0];
					  for (ic = cr; ic + 1 <= i1; ic++) 
					  {
						  ia++;
						  W->data[ic] += b->data[ib] * L->data[ia - 1];
					  }
				  }

				  ar += L->size[0];
			  }

			  br += L->size[1];
			  cr += L->size[0];
		  }
	  }
  }

  emxFree_real_T(&b);

  b_emxInit_real_T(&B, 1);
  B->size[0] = Amplitudes->size[0];
  i0 = B->size[0];  
  emxEnsureCapacity((emxArray__common *)B, i0, (int32_T)sizeof(real_T));
  ar = Amplitudes->size[0];
  for (i0 = 0; i0 < ar; i0++) 
  {
	  B->data[i0] = Amplitudes->data[i0];
  }

  b_emxInit_real_T(&AmplitudesL, 1);
  if ((L->size[1] == 1) || (B->size[0] == 1)) 
  {	  
	  AmplitudesL->size[0] = L->size[0];
	  i0 = AmplitudesL->size[0];
	  emxEnsureCapacity((emxArray__common *)AmplitudesL, i0, (int32_T)sizeof(real_T));
	  ar = L->size[0];
	  for (i0 = 0; i0 < ar; i0++) 
	  {
		  AmplitudesL->data[i0] = 0.0;
		  br = L->size[1];
		  for (i1 = 0; i1 < br; i1++) 
		  {
			  AmplitudesL->data[i0] += L->data[i0 + L->size[0] * i1] * B->data[i1];
		  }
	  }
  } 
  else 
  {
	  unnamed_idx_0 = (uint32_T)L->size[0];	  
	  AmplitudesL->size[0] = (int32_T)unnamed_idx_0;
	  i0 = AmplitudesL->size[0];
	  emxEnsureCapacity((emxArray__common *)AmplitudesL, i0, (int32_T)sizeof(real_T));
	  
	  if (L->size[0] == 0) 
	  {
	  } 
	  else 
	  {
		  cr = 0;
		  while ((L->size[0] > 0) && (cr <= 0)) 
		  {
			  i0 = L->size[0];
			  for (ic = 1; ic <= i0; ic++) 
			  {
				  AmplitudesL->data[ic - 1] = 0.0;
			  }

			  cr = L->size[0];
		  }

		  br = 0;
		  cr = 0;
		  while ((L->size[0] > 0) && (cr <= 0)) 
		  {
			  ar = 0;
			  i0 = br + L->size[1];
			  for (ib = br; ib + 1 <= i0; ib++) 
			  {
				  if (B->data[ib] != 0.0) 
				  {
					  ia = ar;
					  i1 = L->size[0];
					  for (ic = 0; ic + 1 <= i1; ic++) 
					  {
						  ia++;
						  AmplitudesL->data[ic] += B->data[ib] * L->data[ia - 1];
					  }
				  }

				  ar += L->size[0];
			  }

			  br += L->size[1];
			  cr = L->size[0];
		  }
	  }
  }

  /*  regularization parameter */
  k = T2->size[0];
  i0 = L->size[0] * L->size[1];
  L->size[0] = k;
  emxEnsureCapacity((emxArray__common *)L, i0, (int32_T)sizeof(real_T));
  k = T2->size[0];
  i0 = L->size[0] * L->size[1];
  L->size[1] = k;
  emxEnsureCapacity((emxArray__common *)L, i0, (int32_T)sizeof(real_T));
  ar = T2->size[0] * T2->size[0];
  for (i0 = 0; i0 < ar; i0++) 
  {
	  L->data[i0] = 0.0;
  }

  u0 = T2->size[0];
  u1 = T2->size[0];
  if (u0 <= u1) 
  {
  } else 
  {
	  u0 = u1;
  }

  k = (int32_T)u0;
  if (k > 0) 
  {
	  for (i = 0; i + 1 <= k; i++) 
	  {
		  L->data[i + L->size[0] * i] = 1.0;
	  }
  }

  i0 = W->size[0] * W->size[1];
  emxEnsureCapacity((emxArray__common *)W, i0, (int32_T)sizeof(real_T));
  k = W->size[0];
  cr = W->size[1];
  br = T2->size[0];
  ar = k * cr;
  for (i0 = 0; i0 < ar; i0++) 
  {
	  W->data[i0] += (real_T)br * L->data[i0];
  }

  invNxN(W, L);
  emxFree_real_T(&W);

  /* Omega = eye(P, P)\(W + Alpha*eye(P, P)); */
  /*  solution */
  for (i = 1; i <= Iterations; i++) 
  {
	  //br = T2->size[0];
	  br = Alpha;
	  i0 = B->size[0];
	  B->size[0] = Spectrum->size[0];
	  emxEnsureCapacity((emxArray__common *)B, i0, (int32_T)sizeof(real_T));
	  ar = Spectrum->size[0];
	  for (i0 = 0; i0 < ar; i0++) 
	  {
		  B->data[i0] = (real_T)br * Spectrum->data[i0] + AmplitudesL->data[i0];
	  }

	  /*  A = Omega * B; */
	  /*  Spectrum = (abs(A) + A)/2.0;     */
	  if ((L->size[1] == 1) || (B->size[0] == 1)) 
	  {
		  i0 = Spectrum->size[0];
		  Spectrum->size[0] = L->size[0];
		  emxEnsureCapacity((emxArray__common *)Spectrum, i0, (int32_T)sizeof(real_T));
		  ar = L->size[0];
		  for (i0 = 0; i0 < ar; i0++) 
		  {
			  Spectrum->data[i0] = 0.0;
			  br = L->size[1];
			  for (i1 = 0; i1 < br; i1++) 
			  {
				  Spectrum->data[i0] += L->data[i0 + L->size[0] * i1] * B->data[i1];
			  }
		  }
	  } 
	  else 
	  {
		  unnamed_idx_0 = (uint32_T)L->size[0];
		  i0 = Spectrum->size[0];
		  Spectrum->size[0] = (int32_T)unnamed_idx_0;
		  emxEnsureCapacity((emxArray__common *)Spectrum, i0, (int32_T)sizeof(real_T));
		  ar = (int32_T)unnamed_idx_0;
		  for (i0 = 0; i0 < ar; i0++) 
		  {
			  Spectrum->data[i0] = 0.0;
		  }

		  if (L->size[0] == 0) 
		  {
		  } 
		  else 
		  {
			  cr = 0;
			  while ((L->size[0] > 0) && (cr <= 0)) 
			  {
				  i0 = L->size[0];
				  for (ic = 1; ic <= i0; ic++) 
				  {
					  Spectrum->data[ic - 1] = 0.0;
				  }

				  cr = L->size[0];
			  }

			  br = 0;
			  cr = 0;
			  while ((L->size[0] > 0) && (cr <= 0)) 
			  {
				  ar = 0;
				  i0 = br + L->size[1];
				  for (ib = br; ib + 1 <= i0; ib++) 
				  {
					  if (B->data[ib] != 0.0) 
					  {
						  ia = ar;
						  i1 = L->size[0];
						  for (ic = 0; ic + 1 <= i1; ic++) 
						  {
							  ia++;
							  Spectrum->data[ic] += B->data[ib] * L->data[ia - 1];
						  }
					  }

					  ar += L->size[0];
				  }

				  br += L->size[1];
				  cr = L->size[0];
			  }
		  }
	  }

	  for (k = 0; k < T2->size[0]; k++) 
	  {
		  if (Spectrum->data[k] < 0.0) 
		  {
			  Spectrum->data[k] = -Spectrum->data[k];
		  }
	  }
  }

  emxFree_real_T(&B);
  emxFree_real_T(&AmplitudesL);
  emxFree_real_T(&L);
  
}

/* End of code generation (DMdeconvolution.c) */
