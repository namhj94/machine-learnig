// Pthread_KNN 쓰레드 4개사용(dist 4개로 분할)+SIMD
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>
#include <pthread.h>

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

void *calcul(void *num);

Info *test = NULL;
Info *ref = NULL;
Dist *dist1 = NULL, *dist2 = NULL, *dist3 = NULL, *dist4 = NULL;
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
	int i, j;
	pthread_t thread[4];
	int thr_id;
	test = (Info*)_aligned_malloc(sizeof(Info)*TEST_SIZE, 16);
	ref = (Info*)_aligned_malloc(sizeof(Info)*REF_SIZE, 16);
	for (i = 0; i<TEST_SIZE; i++)
		test[i].pos = (float*)_aligned_malloc(sizeof(float)*VECTOR_LENGTH, 16);
	for (i = 0; i<REF_SIZE; i++)
		ref[i].pos = (float*)_aligned_malloc(sizeof(float)*VECTOR_LENGTH, 16);
	dist1 = (Dist*)malloc(sizeof(Dist)*REF_SIZE);
	dist2 = (Dist*)malloc(sizeof(Dist)*REF_SIZE);
	dist3 = (Dist*)malloc(sizeof(Dist)*REF_SIZE);
	dist4 = (Dist*)malloc(sizeof(Dist)*REF_SIZE);

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

	int n1 = 0, n2 = 1024, n3 = 2048, n4 = 3072;

	start = clock();
	thr_id = pthread_create(&thread[0], NULL, calcul, (void *)&n1);
	if (thr_id < 0)
		printf("thread create error 1 !!");
	thr_id = pthread_create(&thread[1], NULL, calcul, (void *)&n2);
	if (thr_id < 0)
		printf("thread create error 2 !!");
	thr_id = pthread_create(&thread[2], NULL, calcul, (void *)&n3);
	if (thr_id < 0)
		printf("thread create error 3 !!");
	thr_id = pthread_create(&thread[3], NULL, calcul, (void *)&n4);
	if (thr_id < 0)
		printf("thread create error 4 !!");
	thr_id = pthread_join(thread[0], NULL);
	thr_id = pthread_join(thread[1], NULL);
	thr_id = pthread_join(thread[2], NULL);
	thr_id = pthread_join(thread[3], NULL);
	end = clock();
	printf("걸린시간 %f s", (double)(end - start) / CLOCKS_PER_SEC);
	_aligned_free(test);
	_aligned_free(ref);
	free(dist1);
	free(dist2);
	free(dist3);
	free(dist4);

	getchar();
	return 0;

}
void *calcul(void *num)
{
	__m128 unit1, unit2, result_unit, sum_sse;
	sum_sse.m128_f32[0] = 0;
	sum_sse.m128_f32[1] = 0;
	sum_sse.m128_f32[2] = 0;
	sum_sse.m128_f32[3] = 0;
	int n = *((int *)num);
	int i, j, h;
	Dist *index;
	switch (n)
	{
	case 0:	index = dist1;
		break;
	case 1024: index = dist2;
		break;
	case 2048:	index = dist3;
		break;
	case 3072:	index = dist4;
		break;
	default:
		printf("shit");
		break;
	}
	printf(" n : %d \n", n);
	for (i = n; i<n + 1024; i++)
	{

		for (j = 0; j < REF_SIZE; j++)
		{
			for (h = 0; h < VECTOR_LENGTH/4; h++)
			{
				unit1 = _mm_load_ps(&test[i].pos[h*4]);
				unit2 = _mm_load_ps(&ref[j].pos[h*4]);
				result_unit = _mm_sub_ps(unit2, unit1);
				result_unit = _mm_mul_ps(result_unit, result_unit);
				sum_sse = _mm_add_ps(sum_sse, result_unit);
			}
			index[j].distVal = sum_sse.m128_f32[0] + sum_sse.m128_f32[1] + sum_sse.m128_f32[2] + sum_sse.m128_f32[3];
			index[j].lableInfo = ref[j].lable;
			sum_sse.m128_f32[0] = 0;
			sum_sse.m128_f32[1] = 0;
			sum_sse.m128_f32[2] = 0;
			sum_sse.m128_f32[3] = 0;
		}
		qsort(index, REF_SIZE, sizeof(Dist), comp);
		printf("dist : %f , lable : %d \n", index[0].distVal, index[0].lableInfo);
	}
	return NULL;
}
