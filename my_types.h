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

#endif