#include "helper.h"

int get_int_from_string(char * c, int start, int end){
    int i = start, s = 0;
    for(; i < end; s = s*10+(c[i]-'0'), i++);
    return s;
}

void get_char_from_int(char * c, int value, int * start) {
    char nb[5];
    int i = 0;
    sprintf(nb, "%d", value);
    while(nb[i] != '\0'){
        c[*start] = nb[i];
        i++; *start = *start+1;
    }
}

void product_informations_to_string(product_informations_t * p_infos, char * map) {
    int i, j, n, w = 0;

    // On écrit le nombre de multiplications à éffectuer
    get_char_from_int(map, p_infos->nb_mult, &w);
    map[w++] = '\n';

    n = p_infos->nb_mult;
    matrice_t * mat = NULL;

    while(n > 0) {
        // On écrit le nombre de lignes et colonnes de chaque matrice.
        /*
            1 2
            1 3
        */
        for(i=0; i<p_infos->nb_mult*2; i++) {
            mat = p_infos->matrices[i];
            map[w++] = '0' + mat->nb_lignes;
            map[w++] = ' ';
            map[w++] = '0' + mat->nb_colonnes;
            map[w++] = '\n';
        }

        // On écrit les vecteurs de chaque matrice.
        /*
            2 10
            3 5 10
        */
        for(i=0; i<p_infos->nb_mult*2; i++) {
            mat = p_infos->matrices[i];
            for(j=0; j<mat->nb_lignes; j++) {
                for(int k=0; k<mat->nb_colonnes; k++) {
                    get_char_from_int(map, (int) mat->vecteurs[j][k], &w);
                    if(k != mat->nb_colonnes-1)
                        map[w++] = ' ';
                }
                map[w++] = '\n';
            }
        }

        n--;
    }
    map[w++] = '\0';
}

void print_product_informations(product_informations_t * p) {
    char map[268];
    product_informations_to_string(p, map);
    puts(map);
}

void print_matrice(double ** mat, int n_lin, int n_col) {
    for(int i=0; i<n_lin; i++)
        for(int j=0; j<n_col; j++) {
            if(j==0)
                printf("[ ");
            printf(" %d ", (int) mat[i][j]);
            if(j == n_col-1)
                printf(" ]\n");
        }

}