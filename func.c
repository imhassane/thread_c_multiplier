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
	size_t index;
	size_t iter;
	cpu_set_t ensemble;
	size_t coeur;

	// Récuperation de l'index à partir des parametres du thread.
	index = (size_t) data;

	CPU_ZERO(&ensemble);
	coeur = sysconf(_SC_NPROCESSORS_ONLN);
	CPU_SET(index%coeur, &ensemble);
	sched_setaffinity(0, sizeof(cpu_set_t), &ensemble);


	printf("Begin mult(%d)\n", (int) index);

	for(iter=0; iter<prod.nbIterations; iter++)
	{
		pthread_mutex_lock(&(prod.mutex));
		while(prod.state != STATE_MULT || prod.pendingMult[index] != 1)
		{
			pthread_cond_wait(&(prod.cond), &(prod.mutex));
		}
		pthread_mutex_unlock(&(prod.mutex));

		printf("--> mult(%ld)\n",index);
		printf("CPU : %d --> mult(%ld)\n", sched_getcpu(), index);

		/*=>Effectuer la multiplication a l'index du thread courant... */
		prod.v3[index]= (prod.v1[index]) * (prod.v2[index]);

		wasteTime(200+(rand()%200)); /* Perte du temps avec wasteTime() */

		fprintf(stderr,"<-- mult(%ld) : %.3g*%.3g=%.3g\n",           /* Affichage du */
				index,prod.v1[index],prod.v2[index],prod.v3[index]);/* calcul sur   */
																	/* l'index      */
		/*=>Marquer la fin de la multiplication en cours... */
		pthread_mutex_lock(&(prod.mutex));
			prod.pendingMult[index] = 0;
			if(nbPendingMult(&prod)==0)
			{
				prod.state = STATE_ADD;
				pthread_cond_broadcast(&(prod.cond));
			}
		pthread_mutex_unlock(&(prod.mutex));
	}

	fprintf(stderr,"Quit mult(%ld)\n",index);
	return(data);
}

/*****************************************************************************/
void * add(void * data)
{
	size_t iter;
	size_t modulo;
	cpu_set_t ensemble;
	size_t coeur;

	fprintf(stderr,"Begin add()\n");

	CPU_ZERO(&ensemble);
	coeur = sysconf(_SC_NPROCESSORS_ONLN);
	modulo = prod.size%coeur;
	for (int i = modulo; i < coeur; i++)
	{
		CPU_SET(i,&ensemble);
	}
	sched_setaffinity(0, sizeof(cpu_set_t), &ensemble);
		
	for(iter=0;iter<prod.nbIterations;iter++)
	{
		size_t index;

		/*=>Attendre l'autorisation d'addition... */
		pthread_mutex_lock(&(prod.mutex));
		while(prod.state != STATE_ADD)
		{
			pthread_cond_wait(&(prod.cond), &(prod.mutex));
		}
		pthread_mutex_unlock(&(prod.mutex));

		fprintf(stderr,"--> add\n"); /* L'addition peut commencer */
		printf("CPU : %d --> add\n", sched_getcpu());

		/* Effectuer l'addition... */
		prod.result=0.0;
		for(int i=0; i<prod.size; prod.result += prod.v3[i], i++);

		wasteTime(100+(rand()%100)); /* Perdre du temps avec wasteTime() */

		fprintf(stderr,"<-- add\n");

		/*=>Autoriser le demarrage de l'affichage... */
		pthread_mutex_lock(&(prod.mutex));

			prod.state = STATE_PRINT;
			pthread_cond_broadcast(&(prod.cond));

		pthread_mutex_unlock(&(prod.mutex));

	}
	fprintf(stderr,"Quit add()\n");
	return(data);
}