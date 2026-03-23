#include "openshop.h"

/* Calcule la duree totale d'un job */
static int duree_totale(int j) {
    int total = 0;
    for (int k = 0; k < NM; k++)
        total += p[j][k];
    return total;
}

/* SPT : trier les jobs par duree totale croissante */
void heuristique_spt(Sol *s) {
    int ordre[NJ];
    for (int i = 0; i < NJ; i++) ordre[i] = i;

    for (int i = 0; i < NJ - 1; i++)
        for (int j = i + 1; j < NJ; j++)
            if (duree_totale(ordre[i]) > duree_totale(ordre[j])) {
                int t = ordre[i];
                ordre[i] = ordre[j];
                ordre[j] = t;
            }

    construire(ordre, s);
}

/* EDD : trier les jobs par echeance croissante */
void heuristique_edd(Sol *s) {
    int ordre[NJ];
    for (int i = 0; i < NJ; i++) ordre[i] = i;

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
    int ordre[NJ];
    for (int i = 0; i < NJ; i++) ordre[i] = i;

    for (int i = 0; i < NJ - 1; i++)
        for (int j = i + 1; j < NJ; j++)
            if (duree_totale(ordre[i]) < duree_totale(ordre[j])) {
                int t = ordre[i];
                ordre[i] = ordre[j];
                ordre[j] = t;
            }

    construire(ordre, s);
}
