#include <stdio.h>
#include <string.h>
#include "openshop.h"

/* ---- Donnees du probleme ---- */

int p[NJ][NM] = {
    {3, 2, 1},  // Job 1
    {1, 4, 2},  // Job 2
    {2, 1, 5},  // Job 3
};
int d[NJ] = {6, 8, 5};

/* ---- Utilitaires ---- */

int max2(int a, int b) {
    return a > b ? a : b;
}

// Calcule sum Tj = somme des retards
int sum_tj(Sol *s) {
    int total = 0;

    for (int j = 0; j < NJ; j++) {
        int cj = 0;
        for (int k = 0; k < NM; k++)
            cj = max2(cj, s->start[j][k] + p[j][k]);
        total += max2(0, cj - d[j]);
    }

    return total;
}

// Affiche le Gantt + retards
void afficher(Sol *s, const char *nom) {
    printf("=== %s === (sum Tj = %d)\n", nom, sum_tj(s));

    for (int k = 0; k < NM; k++) {
        printf("  M%d: ", k + 1);

        // Trier les jobs par date de debut sur cette machine
        int ordre[NJ] = {0, 1, 2};
        for (int i = 0; i < NJ - 1; i++)
            for (int j = i + 1; j < NJ; j++)
                if (s->start[ordre[i]][k] > s->start[ordre[j]][k]) {
                    int t = ordre[i];
                    ordre[i] = ordre[j];
                    ordre[j] = t;
                }

        for (int i = 0; i < NJ; i++) {
            int j = ordre[i];
            printf("J%d[%d-%d] ", j + 1, s->start[j][k], s->start[j][k] + p[j][k]);
        }
        printf("\n");
    }

    for (int j = 0; j < NJ; j++) {
        int cj = 0;
        for (int k = 0; k < NM; k++)
            cj = max2(cj, s->start[j][k] + p[j][k]);
        printf("  Job %d: C=%d, D=%d, T=%d\n", j + 1, cj, d[j], max2(0, cj - d[j]));
    }
    printf("\n");
}

// Construit un ordonnancement a partir d'un ordre de jobs
// Pour chaque job, place les operations de la plus courte a la plus longue
void construire(int *ordre_jobs, Sol *s) {
    int fin_machine[NM] = {0};
    int fin_job[NJ] = {0};

    for (int i = 0; i < NJ; i++) {
        int j = ordre_jobs[i];

        // Trier les machines par duree croissante pour ce job
        int mk[NM] = {0, 1, 2};
        for (int a = 0; a < NM - 1; a++)
            for (int b = a + 1; b < NM; b++)
                if (p[j][mk[a]] > p[j][mk[b]]) {
                    int t = mk[a];
                    mk[a] = mk[b];
                    mk[b] = t;
                }

        for (int mi = 0; mi < NM; mi++) {
            int k = mk[mi];
            int debut = max2(fin_machine[k], fin_job[j]);
            s->start[j][k] = debut;
            fin_machine[k] = debut + p[j][k];
            fin_job[j] = debut + p[j][k];
        }
    }
}

/* ---- Main ---- */

int main(void) {
    printf("=== Probleme Open Shop O3 || sum Tj ===\n\n");

    printf("         M1  M2  M3  Dj\n");
    for (int j = 0; j < NJ; j++)
        printf("  Job %d:  %d   %d   %d   %d\n", j + 1, p[j][0], p[j][1], p[j][2], d[j]);
    printf("\n");

    /* Heuristiques */
    Sol spt, edd, ltr;

    printf("--- HEURISTIQUES ---\n\n");

    heuristique_spt(&spt);
    afficher(&spt, "SPT");

    heuristique_edd(&edd);
    afficher(&edd, "EDD");

    heuristique_ltr(&ltr);
    afficher(&ltr, "LTR");

    /* VNS */
    Sol vns_res;

    printf("--- VNS (depuis LTR) ---\n\n");

    vns(&ltr, &vns_res);
    afficher(&vns_res, "VNS resultat");

    /* Recap */
    printf("--- RECAP ---\n");
    printf("  SPT:  sum Tj = %d\n", sum_tj(&spt));
    printf("  EDD:  sum Tj = %d\n", sum_tj(&edd));
    printf("  LTR:  sum Tj = %d\n", sum_tj(&ltr));
    printf("  VNS:  sum Tj = %d\n", sum_tj(&vns_res));
    printf("  Borne inf:     3\n");

    return 0;
}
