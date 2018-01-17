#ifndef GF_DATA_GENERAL_H
#define GF_DATA_GENERAL_H


#include <iostream> 

#include "rscoding.h"


using namespace std;


typedef struct 
{
	GF*			gf;			// Поле Галуа
	GFPoly**	gf_polys;	// массив указателей на порождающие полиномы степени от 1 до 127	
	int			index;		// номер текущего порождающего полинома в структуре gf_polys
	int			index_hdr;	// номер текущего порождающего полинома в структуре gf_polys для кодирования заголовка передаваемых сообщений
	int			index_body;	// номер текущего порождающего полинома в структуре gf_polys для кодирования данных в теле передаваемых сообщений
	int			index_ftr;	// номер текущего порождающего полинома в структуре gf_polys для кодирования концовки передаваемых сообщений
	int			size;		// количество элементов массива gf_polys
} GF_Data;


typedef struct  
{
	int		pack_block_len;	// общая длина блоков в пакетах многопакетных сообщений (включая проверочные байты Рида-Соломона)
	int		pack_rec_errs;	// количество восстанавливаемых ошибок в каждом блоке в пакетах многопакетных сообщений
} RS_Data;


void gfdata_init(GF_Data *gf_data, int sz);
void gfdata_destroy(GF_Data *gf_data);


#endif // GF_DATA_GENERAL_H