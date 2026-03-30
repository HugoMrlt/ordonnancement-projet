#include "openshop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Utilitaires VNS ---- */

/* Extrait l'ordre des jobs sur chaque machine depuis une solution */
static void extraire_ordre(Sol *s, int ordre[NM][NJ]) {
    for (int k = 0; k < NM; k++) {
        for (int j = 0; j < NJ; j++) ordre[k][j] = j;
        for (int i = 0; i < NJ - 1; i++)
            for (int j = i + 1; j < NJ; j++)
                if (s->start[ordre[k][i]][k] > s->start[ordre[k][j]][k]) {
                    int t = ordre[k][i];
                    ordre[k][i] = ordre[k][j];
                    ordre[k][j] = t;
                }
    }
}

/* Reconstruit une solution a partir de l'ordre des jobs par machine */
static void reconstruire(int ordre[NM][NJ], Sol *s) {
    int fin_machine[NM] = {0};
    int fin_job[NJ] = {0};

    for (int k = 0; k < NM; k++) {
        for (int i = 0; i < NJ; i++) {
            int j = ordre[k][i];
            int debut = max2(fin_machine[k], fin_job[j]);
            s->start[j][k] = debut;
            fin_machine[k] = debut + p[j][k];
            fin_job[j] = debut + p[j][k];
        }
    }
}

/* ---- Les 3 voisinages ---- */

/* N1 : swap de 2 jobs consecutifs sur une machine */
static int appliquer_n1(int ordre[NM][NJ], int k, int i) {
    if (i + 1 >= NJ) return 0;
    int t = ordre[k][i];
    ordre[k][i] = ordre[k][i + 1];
    ordre[k][i + 1] = t;
    return 1;
}

/* N2 : pour un job j, echanger l'ordre sur 2 machines (swap inter-machine) */
static void appliquer_n2(int ordre[NM][NJ], int j, int k1, int k2) {
    /* Trouver la position de j sur k1 et k2, puis swapper */
    int pos1 = -1, pos2 = -1;
    for (int i = 0; i < NJ; i++) {
        if (ordre[k1][i] == j) pos1 = i;
        if (ordre[k2][i] == j) pos2 = i;
    }
    if (pos1 > 0) {
        int t = ordre[k1][pos1];
        ordre[k1][pos1] = ordre[k1][pos1 - 1];
        ordre[k1][pos1 - 1] = t;
    }
    if (pos2 < NJ - 1) {
        int t = ordre[k2][pos2];
        ordre[k2][pos2] = ordre[k2][pos2 + 1];
        ordre[k2][pos2 + 1] = t;
    }
}

/* N3 : reinsertion d'un job a une autre position sur une machine */
static void appliquer_n3(int ordre[NM][NJ], int k, int de, int vers) {
    int job = ordre[k][de];
    if (de < vers) {
        for (int x = de; x < vers; x++) ordre[k][x] = ordre[k][x + 1];
    } else {
        for (int x = de; x > vers; x--) ordre[k][x] = ordre[k][x - 1];
    }
    ordre[k][vers] = job;
}

/* ---- VNS Deterministe (best improvement) ---- */

/* Teste tous les voisins d'un voisinage, retourne 1 si amelioration */
static int meilleur_voisin(Sol *courante, Sol *meilleure, int voisinage) {
    int ordre[NM][NJ];
    extraire_ordre(courante, ordre);
    int best = sum_tj(courante);
    int trouve = 0;

    for (int k = 0; k < NM; k++) {
        if (voisinage == 1) {
            /* N1 : swaps consecutifs */
            for (int i = 0; i < NJ - 1; i++) {
                int sauv[NJ];
                memcpy(sauv, ordre[k], sizeof(sauv));
                appliquer_n1(ordre, k, i);
                Sol test;
                reconstruire(ordre, &test);
                int cout = sum_tj(&test);
                if (cout < best) { best = cout; memcpy(meilleure, &test, sizeof(Sol)); trouve = 1; }
                memcpy(ordre[k], sauv, sizeof(sauv));
            }
        } else if (voisinage == 2) {
            /* N2 : swap inter-machine */
            for (int j = 0; j < NJ; j++) {
                for (int k2 = k + 1; k2 < NM; k2++) {
                    int sauv1[NJ], sauv2[NJ];
                    memcpy(sauv1, ordre[k], sizeof(sauv1));
                    memcpy(sauv2, ordre[k2], sizeof(sauv2));
                    appliquer_n2(ordre, j, k, k2);
                    Sol test;
                    reconstruire(ordre, &test);
                    int cout = sum_tj(&test);
                    if (cout < best) { best = cout; memcpy(meilleure, &test, sizeof(Sol)); trouve = 1; }
                    memcpy(ordre[k], sauv1, sizeof(sauv1));
                    memcpy(ordre[k2], sauv2, sizeof(sauv2));
                }
            }
        } else {
            /* N3 : reinsertions */
            for (int i = 0; i < NJ; i++) {
                for (int j = 0; j < NJ; j++) {
                    if (j == i) continue;
                    int sauv[NJ];
                    memcpy(sauv, ordre[k], sizeof(sauv));
                    appliquer_n3(ordre, k, i, j);
                    Sol test;
                    reconstruire(ordre, &test);
                    int cout = sum_tj(&test);
                    if (cout < best) { best = cout; memcpy(meilleure, &test, sizeof(Sol)); trouve = 1; }
                    memcpy(ordre[k], sauv, sizeof(sauv));
                }
            }
        }
    }
    return trouve;
}

void vns_deterministe(Sol *init, Sol *best) {
    Sol courante;
    memcpy(&courante, init, sizeof(Sol));
    memcpy(best, init, sizeof(Sol));

    int k = 1;
    printf("VNS-det: depart sum_Tj = %d\n", sum_tj(&courante));

    while (k <= 3) {
        Sol voisin;
        if (meilleur_voisin(&courante, &voisin, k)) {
            memcpy(&courante, &voisin, sizeof(Sol));
            if (sum_tj(&courante) < sum_tj(best))
                memcpy(best, &courante, sizeof(Sol));
            printf("  N%d: amelioration -> sum_Tj = %d\n", k, sum_tj(&courante));
            k = 1;
        } else {
            printf("  N%d: pas d'amelioration\n", k);
            k++;
        }
    }
    printf("VNS-det: resultat = %d\n\n", sum_tj(best));
}

/* ---- VNS Stochastique (perturbation aleatoire + recherche locale) ---- */

/* Genere un voisin aleatoire dans le voisinage k */
static void perturbation(Sol *courante, int ordre[NM][NJ], int voisinage) {
    extraire_ordre(courante, ordre);
    int k = rand() % NM;

    if (voisinage == 1) {
        int i = rand() % (NJ - 1);
        appliquer_n1(ordre, k, i);
    } else if (voisinage == 2) {
        int j = rand() % NJ;
        int k2 = (k + 1 + rand() % (NM - 1)) % NM;
        appliquer_n2(ordre, j, k, k2);
    } else {
        int i = rand() % NJ;
        int j;
        do { j = rand() % NJ; } while (j == i);
        appliquer_n3(ordre, k, i, j);
    }
}

/* Recherche locale : descente dans N1 */
static void recherche_locale(Sol *s) {
    int ameliore = 1;
    while (ameliore) {
        ameliore = 0;
        int ordre[NM][NJ];
        extraire_ordre(s, ordre);
        int best = sum_tj(s);

        for (int k = 0; k < NM; k++) {
            for (int i = 0; i < NJ - 1; i++) {
                int sauv[NJ];
                memcpy(sauv, ordre[k], sizeof(sauv));
                appliquer_n1(ordre, k, i);
                Sol test;
                reconstruire(ordre, &test);
                int cout = sum_tj(&test);
                if (cout < best) {
                    best = cout;
                    memcpy(s, &test, sizeof(Sol));
                    ameliore = 1;
                }
                memcpy(ordre[k], sauv, sizeof(sauv));
            }
        }
    }
}

void vns_stochastique(Sol *init, Sol *best) {
    srand(42);
    Sol courante;
    memcpy(&courante, init, sizeof(Sol));
    memcpy(best, init, sizeof(Sol));

    int k = 1;
    int max_iter = 100;
    printf("VNS-sto: depart sum_Tj = %d\n", sum_tj(&courante));

    for (int iter = 0; iter < max_iter && k <= 3; iter++) {
        /* Perturbation dans Nk */
        int ordre[NM][NJ];
        perturbation(&courante, ordre, k);
        Sol voisin;
        reconstruire(ordre, &voisin);

        /* Recherche locale (descente N1) */
        recherche_locale(&voisin);

        if (sum_tj(&voisin) < sum_tj(&courante)) {
            memcpy(&courante, &voisin, sizeof(Sol));
            if (sum_tj(&courante) < sum_tj(best))
                memcpy(best, &courante, sizeof(Sol));
            printf("  N%d (iter %d): amelioration -> sum_Tj = %d\n", k, iter, sum_tj(&courante));
            k = 1;
        } else {
            k++;
        }
    }
    printf("VNS-sto: resultat = %d\n\n", sum_tj(best));
}
