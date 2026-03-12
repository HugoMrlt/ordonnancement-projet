#include "openshop.h"

/* SPT : trier les jobs par duree totale croissante */
void heuristique_spt(Sol *s) {
    int ordre[NJ] = {0, 1, 2};

    for (int i = 0; i < NJ - 1; i++)
        for (int j = i + 1; j < NJ; j++) {
            int pi = p[ordre[i]][0] + p[ordre[i]][1] + p[ordre[i]][2];
            int pj = p[ordre[j]][0] + p[ordre[j]][1] + p[ordre[j]][2];
            if (pi > pj) {
                int t = ordre[i];
                ordre[i] = ordre[j];
                ordre[j] = t;
            }
        }

    construire(ordre, s);
}

/* EDD : trier les jobs par echeance croissante */
void heuristique_edd(Sol *s) {
    int ordre[NJ] = {0, 1, 2};

    for (int i = 0; i < NJ - 1; i++)
        for (int j = i + 1; j < NJ; j++)
            if (d[ordre[i]] > d[ordre[j]]) {
                int t = ordre[i];
                ordre[i] = ordre[j];
                ordre[j] = t;
            }

    construire(ordre, s);
}

/* LTR : trier les jobs par duree totale decroissante */
void heuristique_ltr(Sol *s) {
    int ordre[NJ] = {0, 1, 2};

    for (int i = 0; i < NJ - 1; i++)
        for (int j = i + 1; j < NJ; j++) {
            int pi = p[ordre[i]][0] + p[ordre[i]][1] + p[ordre[i]][2];
            int pj = p[ordre[j]][0] + p[ordre[j]][1] + p[ordre[j]][2];
            if (pi < pj) {
                int t = ordre[i];
                ordre[i] = ordre[j];
                ordre[j] = t;
            }
        }

    construire(ordre, s);
}
