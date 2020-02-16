#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

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

void get_data(product_informations_t *);
void free_product_informations(product_informations_t *);

file_reader_result_t * get_values_from_array(char *, int);

int main(int argc,char ** argv)
{
    
    product_informations_t p_informations;

    get_data(&p_informations);

    free_product_informations(&p_informations);

    
    return(EXIT_SUCCESS);
}

void get_data(product_informations_t * p_infos) {
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