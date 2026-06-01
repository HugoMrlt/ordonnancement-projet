#include <string.h>
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

/* Gueret et Prins : a chaque etape on place l'operation la plus critique
 * (max de charge_machine_restante + travail_job_restant) au plus tot */
void heuristique_gueret_prins(Sol *s) {
    int planifie[NJ][NM];
    int fin_machine[NM] = {0};
    int fin_job[NJ] = {0};

    memset(planifie, 0, sizeof(planifie));
    memset(s, 0, sizeof(Sol));

    int reste = NJ * NM;
    while (reste > 0) {
        /* Travail restant par job et charge restante par machine */
        int rest_job[NJ] = {0};
        int rest_machine[NM] = {0};
        for (int j = 0; j < NJ; j++)
            for (int k = 0; k < NM; k++)
                if (!planifie[j][k]) {
                    rest_job[j] += p[j][k];
                    rest_machine[k] += p[j][k];
                }

        /* Choisir l'operation la plus critique : max(L_k + P_j) */
        int bj = -1, bk = -1, meilleur_score = -1;
        for (int j = 0; j < NJ; j++)
            for (int k = 0; k < NM; k++)
                if (!planifie[j][k]) {
                    int score = rest_machine[k] + rest_job[j];
                    if (score > meilleur_score) {
                        meilleur_score = score;
                        bj = j;
                        bk = k;
                    }
                }

        /* Placer l'operation au plus tot */
        int debut = max2(fin_machine[bk], fin_job[bj]);
        s->start[bj][bk] = debut;
        fin_machine[bk] = debut + p[bj][bk];
        fin_job[bj] = debut + p[bj][bk];
        planifie[bj][bk] = 1;
        reste--;
    }
}
