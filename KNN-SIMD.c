// KNN 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <time.h>

#define TEST_SIZE 4096
#define REF_SIZE 2048
#define VECTOR_LENGTH 2048

typedef struct
{
	float *pos;
	int lable;
} Info;

typedef struct
{
	float distVal;
	int lableInfo;
} Dist;

int comp(const void*a, const void*b)
{
	const Dist *x = (const Dist *)a;
	const Dist *y = (const Dist *)b;
	if (x->distVal < y->distVal)	return -1;
	else if (x->distVal > y->distVal) return 1;
	return 0;
}
int main(int argc, char *argv[])
{
	clock_t start, end;
	__m128 unit1, unit2, runit, sum_sse;
	sum_sse.m128_f32[0] = 0;
	sum_sse.m128_f32[1] = 0;
	sum_sse.m128_f32[2] = 0;
	sum_sse.m128_f32[3] = 0;
	int k = atoi(argv[1]);
	int i, j, h;
	Info *test = (Info*)_aligned_malloc(sizeof(Info)*TEST_SIZE, 16);
	Info *ref = (Info*)_aligned_malloc(sizeof(Info)*REF_SIZE, 16);
	for (i = 0; i<TEST_SIZE; i++)
		test[i].pos = (float*)_aligned_malloc(sizeof(float)*VECTOR_LENGTH, 16);
	for (i = 0; i<REF_SIZE; i++)
		ref[i].pos = (float*)_aligned_malloc(sizeof(float)*VECTOR_LENGTH, 16);
	Dist *dist = (Dist*)malloc(sizeof(Dist)*REF_SIZE);

	for (i = 0; i<TEST_SIZE; i++)
	{
		for (j = 0; j<VECTOR_LENGTH; j++)
			test[i].pos[j] = (rand() % 100) / 100.0;
		test[i].lable = (rand() % 5) + 1;
	}

	for (i = 0; i<REF_SIZE; i++)
	{
		for (j = 0; j<VECTOR_LENGTH; j++)
			ref[i].pos[j] = (rand() % 100) / 100.0;
		ref[i].lable = (rand() % 5) + 1;
	}
	start = clock();
	for (i = 0; i<TEST_SIZE; i++)
	{
		for (j = 0; j<REF_SIZE; j++)
		{
			for (h = 0; h<VECTOR_LENGTH; h += 4)
			{
				unit1 = _mm_load_ps(&test[i].pos[h]); //_mm_store_ps(&c[i*4], c_SSE);
				unit2 = _mm_load_ps(&ref[j].pos[h]);
				runit = _mm_sub_ps(unit2, unit1);
				runit = _mm_mul_ps(runit, runit);
				sum_sse = _mm_add_ps(sum_sse, runit);
			}
			dist[j].distVal = sum_sse.m128_f32[0] + sum_sse.m128_f32[1] + sum_sse.m128_f32[2] + sum_sse.m128_f32[3];
			dist[j].lableInfo = ref[j].lable;
			sum_sse.m128_f32[0] = 0;
			sum_sse.m128_f32[1] = 0;
			sum_sse.m128_f32[2] = 0;
			sum_sse.m128_f32[3] = 0;
		}
		qsort(dist, REF_SIZE, sizeof(Dist), comp);
		int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0, cnt5 = 0;
		for (h = 0; h<k; h++) //정렬된거리에서 가장가까운 k개만큼 뽑은후 클래스정보비교 
		{
			if (dist[h].lableInfo == 1)
				cnt1++;
			else if (dist[h].lableInfo == 2)
				cnt2++;
			else if (dist[h].lableInfo == 3)
				cnt3++;
			else if (dist[h].lableInfo == 4)
				cnt4++;
			else if (dist[h].lableInfo == 5)
				cnt5++;
		}
	}
	end = clock();
	printf("걸린시간 %f s", (double)(end - start)/CLOCKS_PER_SEC);;
	_aligned_free(test);
	_aligned_free(ref);
	free(dist);

	return 0;
}
