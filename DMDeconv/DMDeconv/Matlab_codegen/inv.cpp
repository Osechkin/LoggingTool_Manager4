/*
 * inv.c
 *
 * Code generation for function 'inv'
 *
 * C source code generated on: Tue Jan 31 00:56:15 2017
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "DMdeconvolution.h"
#include "inv.h"
#include "DMdeconvolution_emxutil.h"

/* Function Definitions */
void invNxN(const emxArray_real_T *x, emxArray_real_T *y)
{
  int32_T i2;
  int32_T n;
  emxArray_real_T *A;
  int32_T yk;
  emxArray_int32_T *ipiv;
  int32_T k;
  int32_T j;
  int32_T mmj;
  int32_T c;
  int32_T ix;
  real_T smax;
  real_T s;
  int32_T i3;
  int32_T b_c;
  int32_T ijA;
  emxArray_int32_T *p;
  i2 = y->size[0] * y->size[1];
  y->size[0] = x->size[0];
  y->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)y, i2, (int32_T)sizeof(real_T));
  n = x->size[0] * x->size[1];
  for (i2 = 0; i2 < n; i2++) {
    y->data[i2] = 0.0;
  }

  emxInit_real_T(&A, 2);
  i2 = A->size[0] * A->size[1];
  A->size[0] = x->size[0];
  A->size[1] = x->size[1];
  emxEnsureCapacity((emxArray__common *)A, i2, (int32_T)sizeof(real_T));
  n = x->size[0] * x->size[1];
  for (i2 = 0; i2 < n; i2++) {
    A->data[i2] = x->data[i2];
  }

  n = x->size[0];
  yk = x->size[0];
  if (n <= yk) {
  } else {
    n = yk;
  }

  if (n < 1) {
    n = 0;
  }

  emxInit_int32_T(&ipiv, 2);
  i2 = ipiv->size[0] * ipiv->size[1];
  ipiv->size[0] = 1;
  ipiv->size[1] = n;
  emxEnsureCapacity((emxArray__common *)ipiv, i2, (int32_T)sizeof(int32_T));
  if (n > 0) {
    ipiv->data[0] = 1;
    yk = 1;
    for (k = 2; k <= n; k++) {
      yk++;
      ipiv->data[k - 1] = yk;
    }
  }

  if ((x->size[0] < 1) || (x->size[0] < 1)) {
  } else {
    n = x->size[0] - 1;
    yk = x->size[0];
    if (n <= yk) {
      i2 = n;
    } else {
      i2 = yk;
    }

    for (j = 0; j + 1 <= i2; j++) {
      mmj = x->size[0] - j;
      c = j * (x->size[0] + 1);
      if (mmj < 1) {
        yk = -1;
      } else {
        yk = 0;
        if (mmj > 1) {
          ix = c;
          smax = fabs(A->data[c]);
          for (k = 2; k <= mmj; k++) {
            ix++;
            s = fabs(A->data[ix]);
            if (s > smax) {
              yk = k - 1;
              smax = s;
            }
          }
        }
      }

      if (A->data[c + yk] != 0.0) {
        if (yk != 0) {
          ipiv->data[j] = (j + yk) + 1;
          ix = j;
          yk += j;
          for (k = 1; k <= x->size[0]; k++) {
            smax = A->data[ix];
            A->data[ix] = A->data[yk];
            A->data[yk] = smax;
            ix += x->size[0];
            yk += x->size[0];
          }
        }

        i3 = c + mmj;
        for (yk = c + 1; yk + 1 <= i3; yk++) {
          A->data[yk] /= A->data[c];
        }
      }

      b_c = x->size[0] - j;
      yk = (c + x->size[0]) + 1;
      n = c + x->size[0];
      for (k = 1; k < b_c; k++) {
        smax = A->data[n];
        if (A->data[n] != 0.0) {
          ix = c + 1;
          i3 = mmj + yk;
          for (ijA = yk; ijA + 1 < i3; ijA++) {
            A->data[ijA] += A->data[ix] * -smax;
            ix++;
          }
        }

        n += x->size[0];
        yk += x->size[0];
      }
    }
  }

  if (x->size[0] < 1) {
    n = 0;
  } else {
    n = x->size[0];
  }

  emxInit_int32_T(&p, 2);
  i2 = p->size[0] * p->size[1];
  p->size[0] = 1;
  p->size[1] = n;
  emxEnsureCapacity((emxArray__common *)p, i2, (int32_T)sizeof(int32_T));
  if (n > 0) {
    p->data[0] = 1;
    yk = 1;
    for (k = 2; k <= n; k++) {
      yk++;
      p->data[k - 1] = yk;
    }
  }

  for (k = 0; k < ipiv->size[1]; k++) {
    if (ipiv->data[k] > 1 + k) {
      n = p->data[ipiv->data[k] - 1];
      p->data[ipiv->data[k] - 1] = p->data[k];
      p->data[k] = n;
    }
  }

  emxFree_int32_T(&ipiv);
  for (k = 0; k + 1 <= x->size[0]; k++) {
    y->data[k + y->size[0] * (p->data[k] - 1)] = 1.0;
    for (j = k; j + 1 <= x->size[0]; j++) {
      if (y->data[j + y->size[0] * (p->data[k] - 1)] != 0.0) {
        for (yk = j + 1; yk + 1 <= x->size[0]; yk++) {
          y->data[yk + y->size[0] * (p->data[k] - 1)] -= y->data[j + y->size[0] *
            (p->data[k] - 1)] * A->data[yk + A->size[0] * j];
        }
      }
    }
  }

  emxFree_int32_T(&p);
  if ((x->size[0] == 0) || ((y->size[0] == 0) || (y->size[1] == 0))) {
  } else {
    for (j = 1; j <= x->size[0]; j++) {
      c = x->size[0] * (j - 1);
      for (k = x->size[0] - 1; k + 1 > 0; k--) {
        b_c = x->size[0] * k;
        if (y->data[k + c] != 0.0) {
          y->data[k + c] /= A->data[k + b_c];
          for (yk = 1; yk <= k; yk++) {
            y->data[(yk + c) - 1] -= y->data[k + c] * A->data[(yk + b_c) - 1];
          }
        }
      }
    }
  }

  emxFree_real_T(&A);
}

/* End of code generation (inv.c) */
