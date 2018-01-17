// DMConv.cpp: определяет экспортированные функции для приложения DLL.
//

#include "DMDeconv.h"

#include "Matlab_codegen/DMdeconvolution_types.h"
#include "Matlab_codegen/DMdeconvolution_emxutil.h"
#include "Matlab_codegen/DMdeconvolution.h"


void dm_deconv::do_dm_deconv(const mat1D *Amps_vec, const mat1D *Ticks_vec, const mat1D *T2_vec, int Iters, int Alpha, mat1D *Spec_vec, errCode &ErrCode)
{
	int32_T A_size = Amps_vec->size();
	int32_T T_size = Ticks_vec->size();
	int32_T T2_size = T2_vec->size();

	ErrCode = NoErrors;
	if (A_size != T_size || T2_size < 2) 
	{
		ErrCode = DataSizeError;
		return;
	}
	
	// Convert vector of amplitudes to Matlab data array
	emxArray_real_T *Amplitudes;
	emxInit_real_T(&Amplitudes, 1);		
	Amplitudes->size[0] = A_size;
	emxEnsureCapacity((emxArray__common *)Amplitudes, A_size, (int32_T)sizeof(real_T));		
	memcpy(Amplitudes->data, Amps_vec->data(), A_size*sizeof(double));

	// Convert vector of data times to Matlab data array
	emxArray_real_T *Ticks;
	emxInit_real_T(&Ticks, 1);		
	Ticks->size[0] = T_size;
	emxEnsureCapacity((emxArray__common *)Ticks, T_size, (int32_T)sizeof(real_T));		
	memcpy(Ticks->data, Ticks_vec->data(), T_size*sizeof(double));

	// Convert vector of T2 data to Matlab data array
	emxArray_real_T *T2;
	emxInit_real_T(&T2, 1);		
	T2->size[0] = T2_size;
	emxEnsureCapacity((emxArray__common *)T2, T2_size, (int32_T)sizeof(real_T));		
	memcpy(T2->data, T2_vec->data(), T2_size*sizeof(double));


	emxArray_real_T *Spectrum;
	emxInit_real_T(&Spectrum, 1);	
	Spectrum->size[0] = T2_size;
	emxEnsureCapacity((emxArray__common *)Spectrum, T2_size, (int32_T)sizeof(real_T));	
		
	real_T err_code = 0;
	DMdeconvolution(Amplitudes, Ticks, T2, Iters, Alpha, Spectrum);

	Spec_vec->resize(T2_size);
	for (int i = 0; i < T2_size; i++)
	{
		*(Spec_vec->data() + i) = Spectrum->data[i];
	}
}
