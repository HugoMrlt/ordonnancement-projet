#include <stdio.h>
#include <string.h>
#include "openshop.h"

/* Recupere l'ordre des jobs sur chaque machine depuis une solution */
static void extraire_ordre(Sol *s, int ordre[NM][NJ]) {
    for (int k = 0; k < NM; k++) {
        for (int j = 0; j < NJ; j++)
            ordre[k][j] = j;

        for (int i = 0; i < NJ - 1; i++)
            for (int j = i + 1; j < NJ; j++)
                if (s->start[ordre[k][i]][k] > s->start[ordre[k][j]][k]) {
                    int t = ordre[k][i];
                    ordre[k][i] = ordre[k][j];
                    ordre[k][j] = t;
                }
    }
}

/* Reconstruit une solution a partir de l'ordre des jobs sur chaque machine */
static void reconstruire(int ordre[NM][NJ], Sol *s) {
    int fin_machine[NM] = {0};
    int fin_job[NJ] = {0};
    int pos[NM] = {0};
    int places = 0;

    while (places < NJ * NM) {
        int progres = 0;

        for (int k = 0; k < NM; k++) {
            if (pos[k] >= NJ) continue;

            int j = ordre[k][pos[k]];
            int debut = max2(fin_machine[k], fin_job[j]);

            s->start[j][k] = debut;
            fin_machine[k] = debut + p[j][k];
            fin_job[j] = debut + p[j][k];
            pos[k]++;
            places++;
            progres = 1;
        }

        if (!progres) break;
    }
}

/* N1 : essayer tous les swaps de 2 jobs sur chaque machine */
static int essayer_swaps(Sol *courante, Sol *meilleure) {
    int ordre[NM][NJ];
    extraire_ordre(courante, ordre);
    int best = sum_tj(courante);
    int trouve = 0;

    for (int k = 0; k < NM; k++) {
        for (int i = 0; i < NJ; i++) {
            for (int j = i + 1; j < NJ; j++) {
                // Swap
                int t = ordre[k][i];
                ordre[k][i] = ordre[k][j];
                ordre[k][j] = t;

                Sol test;
                reconstruire(ordre, &test);

                if (sum_tj(&test) < best) {
                    best = sum_tj(&test);
                    memcpy(meilleure, &test, sizeof(Sol));
                    trouve = 1;
                }

                // Annuler
                t = ordre[k][i];
                ordre[k][i] = ordre[k][j];
                ordre[k][j] = t;
            }
        }
    }

    return trouve;
}

/* N2 : essayer toutes les reinsertions d'un job a une autre position */
static int essayer_reinsertions(Sol *courante, Sol *meilleure) {
    int ordre[NM][NJ];
    extraire_ordre(courante, ordre);
    int best = sum_tj(courante);
    int trouve = 0;

    for (int k = 0; k < NM; k++) {
        for (int i = 0; i < NJ; i++) {
            int job = ordre[k][i];

            // Retirer le job de la sequence
            int reste[NJ - 1];
            int pos = 0;
            for (int x = 0; x < NJ; x++)
                if (x != i) reste[pos++] = ordre[k][x];

            // Essayer de le reinserer a chaque autre position
            for (int j = 0; j < NJ; j++) {
                if (j == i) continue;

                int new_ordre[NM][NJ];
                memcpy(new_ordre, ordre, sizeof(new_ordre));

                pos = 0;
                for (int x = 0; x < NJ; x++) {
                    if (x == j) new_ordre[k][x] = job;
                    else new_ordre[k][x] = reste[pos++];
                }

                Sol test;
                reconstruire(new_ordre, &test);

                if (sum_tj(&test) < best) {
                    best = sum_tj(&test);
                    memcpy(meilleure, &test, sizeof(Sol));
                    trouve = 1;
                }
            }
        }
    }

    return trouve;
}

/* VNS : alterne entre N1 et N2 jusqu'a plus d'amelioration */
void vns(Sol *init, Sol *best) {
    Sol courante;
    memcpy(&courante, init, sizeof(Sol));
    memcpy(best, init, sizeof(Sol));

    int k = 1;
    printf("VNS: depart sum_Tj = %d\n", sum_tj(&courante));

    while (k <= 2) {
        Sol voisin;
        int ameliore = 0;

        if (k == 1) ameliore = essayer_swaps(&courante, &voisin);
        if (k == 2) ameliore = essayer_reinsertions(&courante, &voisin);

        if (ameliore) {
            memcpy(&courante, &voisin, sizeof(Sol));
            if (sum_tj(&courante) < sum_tj(best))
                memcpy(best, &courante, sizeof(Sol));
            printf("  N%d: amelioration -> sum_Tj = %d\n", k, sum_tj(&courante));
            k = 1;  // retour au premier voisinage
        } else {
            printf("  N%d: pas d'amelioration\n", k);
            k++;
        }
    }

    printf("VNS: resultat = %d\n\n", sum_tj(best));
}
