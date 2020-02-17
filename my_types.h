#ifndef ARCHI_PROJET_H
#define ARCHI_PROJET_H


typedef struct {
    int nb_iterations;
    int taille_vecteurs;
    double ** vecteurs;
} file_reader_result_t;


typedef struct {
    int nb_lignes;
    int nb_colonnes;
    double ** vecteurs;
} matrice_t;


typedef struct {
    int nb_mult;
    matrice_t ** matrices;
} product_informations_t;

/*
    Cette structure represente les données nécessaires pour le calcul
    de chaque coéfficient de matrices.
    les valeurs "i" et "j" representent l'indice ou le coéfficient resultant
    devra etre écrit dans la matrice resultat de la structure "Product"
*/
typedef struct {
    double * arr_1;
    double * arr_2;
    int array_sizes;
    int i;
    int j;
    int id;
} thread_matrice_params_t;

#endif