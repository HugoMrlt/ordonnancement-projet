#include <stdio.h>
#include <string.h>
#include "openshop.h"

/* Etat d'un noeud dans l'arbre B&B */
typedef struct {
    int planifie[NJ][NM];   /* 1 si operation (j,k) planifiee */
    int fin_machine[NM];     /* date de disponibilite machine */
    int fin_job[NJ];         /* date de disponibilite job */
    int start[NJ][NM];       /* dates de debut des operations */
    int profondeur;           /* nombre d'operations planifiees */
} Noeud;

/* Variables globales B&B */
static int meilleur_ub;
static Sol meilleure_sol;
static int noeuds_explores;
static int noeuds_elagues;

/* Borne inferieure : estime le minimum de sum Tj atteignable depuis ce noeud */
static int calculer_lb(Noeud *noeud) {
    int lb = 0;

    for (int j = 0; j < NJ; j++) {
        int restant = 0;
        int tout_fait = 1;

        for (int k = 0; k < NM; k++) {
            if (!noeud->planifie[j][k]) {
                restant += p[j][k];
                tout_fait = 0;
            }
        }

        int cj_lb;
        if (tout_fait) {
            /* Job termine : date de fin reelle */
            cj_lb = 0;
            for (int k = 0; k < NM; k++)
                cj_lb = max2(cj_lb, noeud->start[j][k] + p[j][k]);
        } else {
            /* Job incomplet : borne = disponibilite + temps restant */
            cj_lb = noeud->fin_job[j] + restant;

            /* Contrainte machine : au moins fin_machine[k] + p[j][k] */
            for (int k = 0; k < NM; k++) {
                if (!noeud->planifie[j][k])
                    cj_lb = max2(cj_lb, noeud->fin_machine[k] + p[j][k]);
            }
        }

        lb += max2(0, cj_lb - d[j]);
    }

    return lb;
}

/* Exploration recursive en profondeur */
static void explorer(Noeud *noeud) {
    noeuds_explores++;

    /* Cas de base : toutes les operations planifiees */
    if (noeud->profondeur == NJ * NM) {
        Sol s;
        memcpy(s.start, noeud->start, sizeof(s.start));
        int cout = sum_tj(&s);
        if (cout < meilleur_ub) {
            meilleur_ub = cout;
            memcpy(&meilleure_sol, &s, sizeof(Sol));
        }
        return;
    }

    /* Enumerer les operations candidates */
    for (int j = 0; j < NJ; j++) {
        for (int k = 0; k < NM; k++) {
            if (noeud->planifie[j][k])
                continue;

            /* Planifier temporairement l'operation (j, k) */
            int debut = max2(noeud->fin_machine[k], noeud->fin_job[j]);

            int ancien_fm = noeud->fin_machine[k];
            int ancien_fj = noeud->fin_job[j];

            noeud->planifie[j][k] = 1;
            noeud->start[j][k] = debut;
            noeud->fin_machine[k] = debut + p[j][k];
            noeud->fin_job[j] = debut + p[j][k];
            noeud->profondeur++;

            /* Evaluer la borne inferieure */
            int lb = calculer_lb(noeud);

            if (lb < meilleur_ub) {
                explorer(noeud);
            } else {
                noeuds_elagues++;
            }

            /* Retour arriere */
            noeud->planifie[j][k] = 0;
            noeud->fin_machine[k] = ancien_fm;
            noeud->fin_job[j] = ancien_fj;
            noeud->profondeur--;
        }
    }
}

/* Point d'entree B&B */
int branch_bound(Sol *vns_sol, Sol *bb_best) {
    /* Initialiser UB depuis la solution VNS */
    meilleur_ub = sum_tj(vns_sol);
    memcpy(&meilleure_sol, vns_sol, sizeof(Sol));
    noeuds_explores = 0;
    noeuds_elagues = 0;

    /* Noeud racine : ordonnancement vide */
    Noeud racine;
    memset(&racine, 0, sizeof(Noeud));

    printf("B&B: UB initial = %d (depuis VNS)\n", meilleur_ub);

    explorer(&racine);

    printf("B&B: optimal = %d\n", meilleur_ub);
    printf("B&B: noeuds explores = %d, noeuds elagues = %d\n\n",
           noeuds_explores, noeuds_elagues);

    memcpy(bb_best, &meilleure_sol, sizeof(Sol));
    return meilleur_ub;
}
