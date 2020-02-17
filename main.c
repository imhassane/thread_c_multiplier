#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sched.h>
#include <pthread.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "my_types.h"
#include "func.h"
#include "helper.h"

char FILE_ADRESS[] = "./matrices_test_save.txt";

void get_product_informations_data(product_informations_t *);
void get_product_informations_from_file(product_informations_t *, char *, int);
void save_product_informations_data(product_informations_t *, int);
void free_product_informations(product_informations_t *);

file_reader_result_t * get_values_from_array(char *, int);

int main(int argc,char ** argv)
{

    /*
        On recupere les informations du fichier.
        Tant que nb_mult > 0
            . on parcourt les matrices deux par deux.
            . mat_result sera initalisée en fonction des dimensions
            . on initialise un tableau de thread de multiplications
              des deux matrices.
                . chaque thread de multiplication doit recevoir
                   les vecteurs avec lesquels il doit travaille et
                   des indices pour pouvoir assigner son resultat
            . On initialise chaque case de mat_result
            . pour chaque case
                . On initialise un thread qui va calculer le coefficient.
            
    */
    
    product_informations_t p_informations;
    int fd = open(FILE_ADRESS, O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;

    if(fstat(fd, &sb) == -1) {
        perror("couldn't get file size");
        exit(1);
    }

    char *file_in_memory = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Initialisation de prod.mutex
    pthread_mutex_init(&(prod.mutex), NULL);
    pthread_cond_init(&(prod.cond), NULL);

    // Récuperation des informations du fichier.
    get_product_informations_from_file(&p_informations, file_in_memory, sb.st_size);

    pthread_t   *multh_th;
    void        *thread_return_value;
    thread_matrice_params_t * thread_params = NULL, thread_param;

    int nb_repetitions = 0;
    int n = 0, nb_thread = 0;
    while(nb_repetitions < p_informations.nb_mult * 2) {

        
        // On récupère les matrices.
        prod.nbIterations = 1;
        prod.mat_1 = p_informations.matrices[n];
        prod.mat_2 = p_informations.matrices[n+1];

        // On vérifie que l'opération est bien possible.
        if(prod.mat_1->nb_colonnes == prod.mat_2->nb_lignes) {
            // On initialise la matrice "resultat"
            prod.mat_result = malloc(sizeof * prod.mat_result);
            prod.mat_result->nb_lignes = prod.mat_1->nb_lignes;
            prod.mat_result->nb_colonnes = prod.mat_2->nb_colonnes;
            prod.mat_result->vecteurs = malloc(sizeof * prod.mat_result->vecteurs * prod.mat_1->nb_lignes);

            for(int i=0; i<prod.mat_result->nb_lignes; i++)
                prod.mat_result->vecteurs[i] = malloc(sizeof * prod.mat_result->vecteurs[i] * prod.mat_result->nb_colonnes);

            // On initialise le tableau de thread et les paramètres pour les threads..
            nb_thread = prod.mat_result->nb_lignes * prod.mat_result->nb_colonnes;
            multh_th = malloc(sizeof * multh_th * nb_thread);
            thread_params = malloc(sizeof * thread_params * nb_thread);

            // On initialise pendingMult
            prod.size = (size_t) nb_thread;
            prod.pendingMult = malloc(sizeof * prod.pendingMult * nb_thread);

            // On créé les params pour les threads.
            int counter = 0;
            for(int i=0; i<prod.mat_result->nb_lignes; i++) {
                for(int j=0; j<prod.mat_result->nb_colonnes; j++) {
                    // La taille des tableaux passés au thread_param correspond au nombre de lignes
                    // de la matrice resulat
                    thread_param.array_sizes = prod.mat_result->nb_lignes;
                    thread_param.arr_1 = malloc(sizeof * thread_param.arr_1 * thread_param.array_sizes);
                    thread_param.arr_2 = malloc(sizeof * thread_param.arr_2 * thread_param.array_sizes);
                    thread_param.i = i;
                    thread_param.j = j;
                    thread_param.id = counter;

                    // On multiplie la ligne du premier tableau par la colonne du second
                    // On passe la ligne du premier tableau en parametres.
                    thread_param.arr_1 = prod.mat_1->vecteurs[i];
                    // On passe la colone du second tableau en parametres.
                    for(int k=0; k<thread_param.array_sizes; k++) {
                        thread_param.arr_2[k] = prod.mat_2->vecteurs[k][j];
                    }
                    // On ajoute les données.
                    thread_params[counter] = thread_param;
                    counter++;
                }
            }

            // On lance les threads.
            for(int i=0; i<prod.size; i++) {
                if(pthread_create(&multh_th[i], NULL, mult, (void * restrict) &thread_params[i]) != 0) {
                    perror("creation de threads");
                    exit(1);
                }
            }

            for(int iter=0; iter<prod.nbIterations; iter++) {
                // On démarre les multiplications.
                pthread_mutex_lock(&(prod.mutex));
                for(int i=0; i<nb_thread; prod.pendingMult[i]=1, i++);

                prod.state = STATE_MULT;

                pthread_cond_broadcast(&(prod.cond));
                pthread_mutex_unlock(&(prod.mutex));

                // Gestion de l'affichage
                pthread_mutex_lock(&(prod.mutex));
                    while(prod.state != STATE_PRINT)
                    {
                        pthread_cond_wait(&(prod.cond), &(prod.mutex));
                    }
                pthread_mutex_unlock(&(prod.mutex));
                puts("-- Resultat final --");
                print_matrice(prod.mat_result->vecteurs, prod.mat_result->nb_lignes, prod.mat_result->nb_colonnes);

            }

            // On attends les threads.
            for(int i=0; i<prod.size; i++) {
                if(pthread_join(multh_th[i], NULL) != 0) {
                    perror("attente de threads");
                    exit(1);
                }
            }

            pthread_cond_destroy(&(prod.cond));
            pthread_mutex_destroy(&(prod.mutex));

            // On libères les paramètres de threads.
            for(int i=0; i<nb_thread; i++){
                free(thread_param.arr_1);
                free(thread_param.arr_2);
            }
            free(thread_params);

            // On libère la matrice resultat.
            for(int i=0; i<prod.mat_result->nb_lignes; i++) {
                free(prod.mat_result->vecteurs[i]);
            }

            free(multh_th);
            free(prod.pendingMult);
            free(prod.mat_result->vecteurs);
            free(prod.mat_result);
        }
        
        nb_repetitions+=2;
    }

    munmap(file_in_memory, sb.st_size);
    free_product_informations(&p_informations);
    return(EXIT_SUCCESS);
}

void get_product_informations_data(product_informations_t * p_infos) {
    /*  RESULTAT DU TEST DE LA FONCTION POUR L'EXEMPLE DONNE
        SUR LA FICHE DE TP
    
        Entrez le nombre de multiplications a effectuer
        1
        ---- Informations de la matrice 1 -------
                Nombre de ligne :)
        2
                Nombre de colonnes :)
        2
                Vecteur n°0 de la matrice:
        Entrez mat[0][0]: 1
        Entrez mat[0][1]: 2
                Vecteur n°1 de la matrice:
        Entrez mat[1][0]: 3
        Entrez mat[1][1]: 4
        ---- Informations de la matrice 2 -------
                Nombre de ligne :)
        2
                Nombre de colonnes :)
        3
                Vecteur n°0 de la matrice:
        Entrez mat[0][0]: 5
        Entrez mat[0][1]: 6
        Entrez mat[0][2]: 7
                Vecteur n°1 de la matrice:
        Entrez mat[1][0]: 8
        Entrez mat[1][1]: 9
        Entrez mat[1][2]: 10
    */
    int nb_mult = 1,
        i = 0, j = 0, k = 0,
        n_lin = 0, n_col = 0,
        value = 0;
    puts("Entrez le nombre de multiplications a effectuer");
    scanf("%d", &nb_mult);
    // On entre le nombre de multiplications à éffectuer
    p_infos->nb_mult = nb_mult;
    p_infos->matrices = malloc(sizeof * p_infos->matrices * nb_mult);

    // On entre les matrices
    matrice_t * mat = NULL;

    while(nb_mult > 0) {
        puts("---- Informations de la matrice 1 -------");
        puts("\tNombre de ligne :)");
        scanf("%d", &n_lin);

        puts("\tNombre de colonnes :)");
        scanf("%d", &n_col);
        // On créé une nouvelle matrice
        mat = malloc(sizeof * mat);
        mat->nb_lignes = n_lin;
        mat->nb_colonnes = n_col;
        mat->vecteurs = malloc(sizeof * mat->vecteurs * n_lin);

        // On entre les valeurs des vecteurs de la matrice.
        for(j=0; j<n_lin; j++){
            printf("\tVecteur n°%d de la matrice:\n", j);
            mat->vecteurs[j] = malloc(sizeof * mat->vecteurs[j] * mat->nb_colonnes);

            for(k=0; k<n_col; k++){
                printf("Entrez mat[%d][%d]: ", j, k);
                scanf("%d", &value);
                mat->vecteurs[j][k] = value;
            }
        }

        // On l'ajoute dans la liste des matrices.
        p_infos->matrices[i] = mat;
        i++;

        puts("---- Informations de la matrice 2 -------");
        puts("\tNombre de ligne :)");
        scanf("%d", &n_lin);

        puts("\tNombre de colonnes :)");
        scanf("%d", &n_col);
        // On créé une nouvelle matrice
        mat = malloc(sizeof * mat);
        mat->nb_lignes = n_lin;
        mat->nb_colonnes = n_col;
        mat->vecteurs = malloc(sizeof * mat->vecteurs * n_lin);

        // On entre les valeurs des vecteurs de la matrice.
        for(j=0; j<n_lin; j++){
            printf("\tVecteur n°%d de la matrice:\n", j);
            mat->vecteurs[j] = malloc(sizeof * mat->vecteurs[j] * mat->nb_colonnes);

            for(k=0; k<n_col; k++){
                printf("Entrez mat[%d][%d]: ", j, k);
                scanf("%d", &value);
                mat->vecteurs[j][k] = value;
            }
        }
        
        // On l'ajoute dans la liste des matrices.
        p_infos->matrices[i] = mat;
        i++;
        nb_mult--;
    }
}

void get_product_informations_from_file(product_informations_t * p_infos, char * content, int size) {
    int start = 0, end = 0, value = 0;
    int index_i = 0, index_j = 0;
    int n = 0;

    while(content[end] != '\n') end++;
    p_infos->nb_mult = get_int_from_string(content, start, end);
    start = ++end;
    // On initialise le tableau de matrices.
    p_infos->matrices = malloc(sizeof * p_infos->matrices * p_infos->nb_mult * 2);

    // Déclaration des matrices.
    matrice_t * mat_1 = NULL, * mat_2 = NULL;

    while(n < p_infos->nb_mult*2) {
        // On suppose que chaque multipication requiert deux matrices
        // On recupere les valeurs des lignes et colonnes de ces matrices
        mat_1 = malloc(sizeof * mat_1);
        mat_2 = malloc(sizeof * mat_2);

        // On récupere les lignes des matrices.
        // le premier nombre represente la valeur du nombre de lignes
        // le second represente la valeur du nombre de colonnes.
        while(content[end] != ' ') end++;
        mat_1->nb_lignes = get_int_from_string(content, start, end);
        start = ++end;
        while(content[end] != '\n') end++;
        mat_1->nb_colonnes = get_int_from_string(content, start, end);
        start = ++end;

        while(content[end] != ' ') end++;
        mat_2->nb_lignes = get_int_from_string(content, start, end);
        start = ++end;
        while(content[end] != '\n') end++;
        mat_2->nb_colonnes = get_int_from_string(content, start, end);
        start = ++end;

        // On récupère les vecteurs de chaque matrice.
        // On initialise les vecteurs.
        mat_1->vecteurs = malloc(sizeof * mat_1->vecteurs * mat_1->nb_lignes);
        mat_2->vecteurs = malloc(sizeof * mat_2->vecteurs * mat_2->nb_lignes);

        // Pour chaque matrice, on initialise les vecteurs
        // en fonction du nombre de ligne et colonne
        // Puis on ajoute les valeurs lues depuis la chaine de caractere.
        for(int i=0; i<mat_1->nb_lignes; i++) {
            mat_1->vecteurs[i] = malloc(sizeof * mat_1->vecteurs[i] * mat_1->nb_colonnes);
            for(int j=0; j<mat_1->nb_colonnes; j++) {
                while(content[end] != ' ' && content[end] != '\n') end++;
                mat_1->vecteurs[i][j] = get_int_from_string(content, start, end);
                start = ++end;
            }
        }

        for(int i=0; i<mat_2->nb_lignes; i++) {
            mat_2->vecteurs[i] = malloc(sizeof * mat_2->vecteurs[i] * mat_2->nb_colonnes);
            for(int j=0; j<mat_2->nb_colonnes; j++) {
                while(content[end] != ' ' && content[end] != '\n') end++;
                mat_2->vecteurs[i][j] = get_int_from_string(content, start, end);
                start = ++end;
            }
        }

        // On ajoute les matrices.
        p_infos->matrices[n] = mat_1;
        p_infos->matrices[n+1] = mat_2;
        n+=2;
    }
}

void save_product_informations_data(product_informations_t * p_infos, int fd) {
    /* RESULTATS DU TEST DE LA FONCTION */
    /* Ces valeurs sont enregistrées dans un fichier sous cette forme */
    /*
        1
        2 2
        2 3
        1 2
        3 4
        5 6 7
        8 9 10
    */
    if(p_infos == NULL) {
        puts("Product informations is null");
        exit(1);
    }
    char map[268];
    // On créé une chaine de caractere representant notre objet.
    // Fonction disponble dans "helper.c"
    product_informations_to_string(p_infos, map);

    if(write(fd, map, strlen(map)) == -1) {
        perror("write file");
        free_product_informations(p_infos);
        exit(1);
    }

}

void free_product_informations(product_informations_t * p_infos) {
    int i, j;
    for(i=0; i<p_infos->nb_mult; i++) {
        for(j=0; j<p_infos->nb_mult*2; j++){
            free(p_infos->matrices[j]->vecteurs);
            free(p_infos->matrices[j]);
        }
        free(p_infos->matrices[i]);
    }
}

file_reader_result_t * get_values_from_array(char * content, int size) {
    int start = 0, end = 0;
    int value = 0;

    // On déclare des variables qui nous permettent d'enter
    // les données dans des tableaux deux dimensions.
    int index_i = 0, index_j = 0;

    file_reader_result_t * result = malloc(sizeof * result);

    // On récupère le nombre d'itérations.
    while(content[end] != ' ') end++;
    result->nb_iterations = get_int_from_string(content, start, end);

    // On récupère la taille des vecteurs.
    start = ++end;
    while(content[end] != '\n') end++;
    result->taille_vecteurs = get_int_from_string(content, start, end);

    // On initialise les vecteurs.
    result->vecteurs = malloc(result->nb_iterations * 2 * sizeof * result->vecteurs);
    for(
        int i = 0, l = result->nb_iterations * 2;
        i < l;
        i++
    ) {
        // On met toutes les cases à 0 initialement.
        result->vecteurs[i] = malloc(result->taille_vecteurs * sizeof * result->vecteurs[i]);
        for(int j = 0, c = result->taille_vecteurs; j < c; result->vecteurs[i][j] = 0, j++);
    }

    start = ++end;
    while(end < size) {
        // Tant qu'on a pas un espace ou un retour chariot, on lit la valeur.
        while(content[end] != ' ' && content[end] != '\n') end++;
        value = get_int_from_string(content, start, end);
        if(value != 0) {
            result->vecteurs[index_i][index_j] = (double) value;
            // Si un vecteur est plein, on passe au suivant.
            if(index_j == result->taille_vecteurs - 1) {
                index_j = 0;
                index_i++;
            } else {
                index_j++;
            }
        }
        end++;
        start=end;
    }

    /*
    -- Resultats du test de la fonction.

        int nb_iterations: 3
        int taille_vecteurs: 4
        int ** vecteurs: [
                [10  15  20  22 ]
                [5  2  5  11] 
                [14  17  52  1 ]
                [3  26  1  2] 
                [3  12  13  15 ]
                [78  45  10  9 ]
            ]
    */

    return result;
}

/*
file_reader_result_t * file_reader_result = NULL;

    int fd = open("./test_file.txt", O_RDONLY, S_IRUSR | S_IWUSR);
    struct stat sb;

    if(fstat(fd, &sb) == -1) {
        perror("couldn't get file size");
        exit(1);
    }
    // On créé un bloc ayant pour taille la taille du fichier.
    char *file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    // On matche les valeurs du tableau au file_reader_result.
    file_reader_result = get_values_from_array(file_in_memory, sb.st_size);


    munmap(file_in_memory, sb.st_size);
    for(int i = 0, l = file_reader_result->nb_iterations * 2; i < l; i++) {
        free(file_reader_result->vecteurs[i]);
    }
    free(file_reader_result->vecteurs);
    free(file_reader_result);
    */