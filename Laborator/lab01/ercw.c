/**
 * Algoritmul ERCW-PRAM cu n^3 procesoare pentru inmultire de matrici
 * C <- A * B
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * Se considera un numar de procesoare egal cu n^3, unde n este dimensiunea matricei.
 * 
 * Algoritmul consta in 2 etape
 * 1. Fiecare procesor P(i,j,k) calculeaza in paralel produsul A[i][j] * B[j][k]
 *    A[i][j], respectiv B[j][k] se citesc la un moment de timp diferit, fiind extrase
 *    in variabilele a si b.
 * 2. Se face scrierea concurenta a procesoarelor in locatia C[i][j] 
 *    a termenilor A[i][k] * B[k][j], k = [0...n-1], care se insumeaza.
 */

typedef struct Index {
    int i;
    int j;
    int k;
} Index;

#define n 2

int A[n][n];
int B[n][n];
int C[n][n];

pthread_mutex_t mutex[n][n];
pthread_mutex_t mutexA[n][n];
pthread_mutex_t mutexB[n][n];


void read() {
    int i;
    for (i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &A[i][j]);
        }
    }
    for (i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &B[i][j]);
        }
    }
}

void* threadFunction(void* var) {

    Index thread_id = *(Index*) var;

    pthread_mutex_lock(&mutexA[thread_id.i][thread_id.j]);
    int a = A[thread_id.i][thread_id.k];
    pthread_mutex_unlock(&mutexA[thread_id.i][thread_id.j]);

    pthread_mutex_lock(&mutexB[thread_id.i][thread_id.j]);
    int b = B[thread_id.k][thread_id.j];
    pthread_mutex_unlock(&mutexB[thread_id.i][thread_id.j]);

    // 1. partial result
    int sum = a * b;
    printf("Thread [%d %d %d]: %d\n", thread_id.i, thread_id.j, thread_id.k, sum);

    // 2. add to final result
    pthread_mutex_lock(&mutex[thread_id.i][thread_id.j]);
    C[thread_id.i][thread_id.j] += sum;
    pthread_mutex_unlock(&mutex[thread_id.i][thread_id.j]);

    return NULL;
}

void print() {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    int N = n * n * n; // n^3 procesoare

    read();

    // creare thread_id vector
    // each thread is characterised by three indices (i, j, k) specific to the processor
    struct Index thread_id[N];
    int idx = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                thread_id[idx].i = i;
                thread_id[idx].j = j;
                thread_id[idx].k = k;
                idx++;
            }
        }
    }

    // init mutex
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
           pthread_mutex_init(&mutex[i][j], NULL);
           pthread_mutex_init(&mutexA[i][j], NULL);
           pthread_mutex_init(&mutexB[i][j], NULL);
        }
    }

    // create N threads
	pthread_t tid[N];
	for(int i = 0; i < N; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

    for(int i = 0; i < N; i++) {
		pthread_join(tid[i], NULL);
	}

    // distroy mutex
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            pthread_mutex_destroy(&mutex[i][j]);
            pthread_mutex_destroy(&mutexA[i][j]);
            pthread_mutex_destroy(&mutexB[i][j]);

        }
    }

    print();

    return 0;
}
