#ifndef DMCONV_H
#define DMCONV_H


#include <QtCore/QVector>


namespace dm_deconv
{
	typedef		QVector<QVector<double>* >		mat2D;
	typedef		QVector<double>					mat1D;

	enum errCode {
		NoErrors = 0,		// No errors
		DataSizeError = 1	// Sizes of Ampitudes and Ticks must be equal and more than zero		
	};
	

	__declspec(dllexport) void do_dm_deconv(const mat1D *Amps_vec, const mat1D *Ticks_vec, const mat1D *T2_vec, int Iters, int Alpha, mat1D *Spec_vec, errCode &ErrCode);
}

#endif // DMCONV_H