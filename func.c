#include "func.h"

void initPendingMult(Product * prod)
{
	for(int i=0; i < prod->size; prod->pendingMult[i]=1, i++);
}

int nbPendingMult(Product * prod)
{
	int nb=0;
	for(int i=0; i< prod->size; nb += prod->pendingMult[i], i++);
	return nb;
}

void wasteTime(unsigned long ms)
{
	unsigned long t,t0;
	struct timeval tv;
	gettimeofday(&tv,(struct timezone *)0);
	t0=tv.tv_sec*1000LU+tv.tv_usec/1000LU;
	do
	{
		gettimeofday(&tv,(struct timezone *)0);
		t=tv.tv_sec*1000LU+tv.tv_usec/1000LU;
	} while(t-t0<ms);
}

/*****************************************************************************/

void * mult(void * data)
{
	thread_matrice_params_t * params = (thread_matrice_params_t *) data;


	printf("Begin mult(%d %d)\n", params->i, params->j);

	for(int iter=0; iter<prod.nbIterations; iter++)
	{
		pthread_mutex_lock(&(prod.mutex));
		while(prod.state != STATE_MULT || prod.pendingMult[params->id] != 1)
		{
			pthread_cond_wait(&(prod.cond), &(prod.mutex));
		}
		pthread_mutex_unlock(&(prod.mutex));

		printf("--> mult(%d; %d)\n", params->i, params->j);

		// On éffectue le produit des matrices
		double sum=0;
		for(int i=0; i<params->array_sizes; i++) 
			sum += params->arr_1[i] * params->arr_2[i];
		
		prod.mat_result->vecteurs[params->i][params->j] = sum;

		wasteTime(200+(rand()%200)); 

		fprintf(stderr,"<-- mult(%d; %d) : %2f\n",       
				params->i, params->j, sum);
																	
		pthread_mutex_lock(&(prod.mutex));
			prod.pendingMult[params->id] = 0;
			if(nbPendingMult(&prod)==0)
			{
				prod.state = STATE_PRINT;
				pthread_cond_broadcast(&(prod.cond));
			}
		pthread_mutex_unlock(&(prod.mutex));
	}

	fprintf(stderr,"Quit mult(%d; %d)\n", params->i, params->j);
	return (data);
}
