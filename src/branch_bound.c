#include <stdio.h>
#include <string.h>
#include "openshop.h"

/* Etat d'un noeud dans l'arbre B&B */
typedef struct {
    int scheduled[NJ][NM];  /* 1 si operation (j,k) planifiee */
    int machine_avail[NM];  /* date de disponibilite machine */
    int job_avail[NJ];      /* date de disponibilite job */
    int start[NJ][NM];      /* dates de debut des operations */
    int depth;               /* nombre d'operations planifiees */
} BBNode;

/* Variables globales B&B */
static int best_ub;
static Sol best_sol;
static int nodes_explored;
static int nodes_pruned;

/* Borne inferieure : estime le minimum de sum Tj atteignable depuis ce noeud */
static int compute_lb(BBNode *node) {
    int lb = 0;

    for (int j = 0; j < NJ; j++) {
        int remaining = 0;
        int all_done = 1;

        for (int k = 0; k < NM; k++) {
            if (!node->scheduled[j][k]) {
                remaining += p[j][k];
                all_done = 0;
            }
        }

        int cj_lb;
        if (all_done) {
            /* Job termine : completion reelle */
            cj_lb = 0;
            for (int k = 0; k < NM; k++)
                cj_lb = max2(cj_lb, node->start[j][k] + p[j][k]);
        } else {
            /* Job incomplet : borne = disponibilite + temps restant */
            cj_lb = node->job_avail[j] + remaining;

            /* Contrainte machine : au moins machine_avail[k] + p[j][k] */
            for (int k = 0; k < NM; k++) {
                if (!node->scheduled[j][k])
                    cj_lb = max2(cj_lb, node->machine_avail[k] + p[j][k]);
            }
        }

        lb += max2(0, cj_lb - d[j]);
    }

    return lb;
}

/* DFS recursif */
static void bb_dfs(BBNode *node) {
    nodes_explored++;

    /* Cas de base : toutes les operations planifiees */
    if (node->depth == NJ * NM) {
        Sol s;
        memcpy(s.start, node->start, sizeof(s.start));
        int cost = sum_tj(&s);
        if (cost < best_ub) {
            best_ub = cost;
            memcpy(&best_sol, &s, sizeof(Sol));
        }
        return;
    }

    /* Enumerer les operations candidates */
    for (int j = 0; j < NJ; j++) {
        for (int k = 0; k < NM; k++) {
            if (node->scheduled[j][k])
                continue;

            /* Planifier temporairement l'operation (j, k) */
            int debut = max2(node->machine_avail[k], node->job_avail[j]);

            int old_ma = node->machine_avail[k];
            int old_ja = node->job_avail[j];

            node->scheduled[j][k] = 1;
            node->start[j][k] = debut;
            node->machine_avail[k] = debut + p[j][k];
            node->job_avail[j] = debut + p[j][k];
            node->depth++;

            /* Evaluer la borne inferieure */
            int lb = compute_lb(node);

            if (lb < best_ub) {
                bb_dfs(node);
            } else {
                nodes_pruned++;
            }

            /* Backtrack */
            node->scheduled[j][k] = 0;
            node->machine_avail[k] = old_ma;
            node->job_avail[j] = old_ja;
            node->depth--;
        }
    }
}

/* Point d'entree B&B */
int branch_bound(Sol *vns_sol, Sol *bb_best) {
    /* Initialiser UB depuis la solution VNS */
    best_ub = sum_tj(vns_sol);
    memcpy(&best_sol, vns_sol, sizeof(Sol));
    nodes_explored = 0;
    nodes_pruned = 0;

    /* Noeud racine : ordonnancement vide */
    BBNode root;
    memset(&root, 0, sizeof(BBNode));

    printf("B&B: UB initial = %d (depuis VNS)\n", best_ub);

    bb_dfs(&root);

    printf("B&B: optimal = %d\n", best_ub);
    printf("B&B: noeuds explores = %d, noeuds elagues = %d\n\n",
           nodes_explored, nodes_pruned);

    memcpy(bb_best, &best_sol, sizeof(Sol));
    return best_ub;
}
