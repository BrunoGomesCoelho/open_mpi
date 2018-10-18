/*
 * Bruno Gomes Coelho - 9791160
 * Josué Grace Kabongo Kalala - 9770382
 *
 * Grupo 19

 * Para compilar:
 *	mpicc ex_aula8.c -o run -fopenmp 
 *
 * Para rodar:
 *	mpirun -n [N*N] run 
 *
 * Onde "N" é a ordem da matriz, isto é, precisamos criar N*N processos para 
 * 	o programa funcionar.
 *
 * É necessário também ter o arquivo "entrada.txt" para ele ler a matriz.
 *
 */


#include <mpi.h>
#include <omp.h>

#include <stdio.h>
#include <stdlib.h>


int openmp(int L[], int C[], int N) {
	int i, resultado;

	#pragma omp parallel num_threads(N)
	{
		resultado = 0;

		#pragma omp parallel for reduction(+:resultado)
		for (i=0; i<N; i++)	{
			resultado += L[i]*C[i];
		}
	}

	return resultado;
}


int main(int argc, char** argv) {

	// inicializa variáveis
	int N;
	int my_rank;
	int npes;
	int resultado;
    int i, j, k;
	FILE *arquivo_entrada;

    // Initializa o ambiente MPI
    MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);

	// Se for o processo 0, leia o arquivo e comunique para os outros processos
	if (my_rank == 0) {

		if(!(arquivo_entrada=fopen("entrada.txt","r"))) 
		{
		  printf("Erro ao abrir entrada.txt como leitura! Saindo! \n");
		  return(-1);
		}
		
		// Leitura da primeira linha de entrada.txt contendo as dimensoes de M
		fscanf(arquivo_entrada, "%d", &N);
		
		// criando M[NxN]
		int M[N*N];
		int L[N], C[N];

		// carregando M do arquivo
		for(i=0; i<N; i++)
		  for(j=0; j<N; j++)
		  	fscanf(arquivo_entrada, "%d", &M[i*N+j]);
		
		// Cria os vetores de linha e coluna
		for (i=0; i<N; i++)	{
			L[i] = M[i];
			C[i] = M[i*N];
		}

		// Envia N
		MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// Mandando a linha e coluna necessária para os outros processos
		for (i=0; i < N; i++) {
			for (j=0; j < N; j++) {

				// Calcula para o processo 0 já
				if (i == 0 && j == 0) {
					resultado = openmp(L, C, N);
				}

				// Envia a linha i
				MPI_Send(&M[i*N], N, MPI_INT, i*N+j, 1, MPI_COMM_WORLD);
				
				// Envia a coluna j
				for(k=0; k < N; k++) {
					MPI_Send(&M[k*N+j], 1, MPI_INT, i*N+j, 2, MPI_COMM_WORLD);
				}
			}
		}

		// Envia o resultado para ele mesmo e lê dos outros processos
		int matriz_resultante[N*N];
		MPI_Gather(&resultado, 1, MPI_INT, &matriz_resultante, 1, MPI_INT, 0, MPI_COMM_WORLD);

		// Imprime a matriz resultante
		for (i=0; i<N; i++)
			for (j=0; j<N; j++)
				printf("C[%d][%d]= %d\n", i, j, matriz_resultante[i*N + j]);
	} 


	// Se não for o processo 0, receba os dados do processo 0
	else {
		// Recebe N
		MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

		int L[N], C[N];

		// Recebe Linha
		MPI_Recv(&L, N, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Recebe coluna
		for (i=0; i<N; i++) {
			MPI_Recv(&C[i], 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		resultado = openmp(L, C, N);

		// Envia o resultado para o processo 0
		MPI_Gather(&resultado, 1, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}

    MPI_Finalize();
}

