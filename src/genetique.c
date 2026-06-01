#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "openshop.h"

/* ================================================================
 * PARAMETRES ET TYPE INTERNE
 * ================================================================ */

#define POP_SIZE     20
#define NB_GEN       500
#define TAUX_CROIS   0.85
#define TAUX_MUT     0.20
#define TAILLE_TOURN 3

typedef struct {
    int ordre[NM][NJ];
    int fitness;
} Individu;

/* ================================================================
 * UTILITAIRES
 * ================================================================ */

static double rand_double(void) {
    return (double)rand() / (double)RAND_MAX;
}

static int rand_int(int n) {
    return rand() % n;
}

/* Evalue la fitness d'un individu.
 * On schedule une operation a la fois : celle qui peut demarrer le plus tot
 * parmi toutes les machines, ce qui garantit l'absence de chevauchement
 * sur un meme job (contrainte open shop). */
static void evaluer(Individu *ind) {
    int fin_machine[NM] = {0};
    int fin_job[NJ]     = {0};
    int pos[NM]         = {0};
    Sol s;
    memset(&s, 0, sizeof(Sol));

    int places = 0;
    int total  = NJ * NM;

    while (places < total) {
        /* Trouver l'operation planifiable au plus tot */
        int best_k = -1, best_j = -1, best_t = 1 << 30;

        for (int k = 0; k < NM; k++) {
            if (pos[k] >= NJ) continue;
            int j     = ind->ordre[k][pos[k]];
            int debut = fin_machine[k] > fin_job[j] ? fin_machine[k] : fin_job[j];
            if (debut < best_t) {
                best_t = debut;
                best_k = k;
                best_j = j;
            }
        }

        if (best_k == -1) break;

        s.start[best_j][best_k] = best_t;
        fin_machine[best_k]     = best_t + p[best_j][best_k];
        fin_job[best_j]         = best_t + p[best_j][best_k];
        pos[best_k]++;
        places++;
    }

    ind->fitness = sum_tj(&s);
}

static void copier_individu(Individu *dst, const Individu *src) {
    memcpy(dst, src, sizeof(Individu));
}

/* ================================================================
 * INITIALISATION
 * ================================================================ */

static void permutation_aleatoire(int *tab) {
    for (int i = 0; i < NJ; i++) tab[i] = i;
    for (int i = NJ - 1; i > 0; i--) {
        int j   = rand_int(i + 1);
        int tmp = tab[i];
        tab[i]  = tab[j];
        tab[j]  = tmp;
    }
}

static void individu_aleatoire(Individu *ind) {
    for (int k = 0; k < NM; k++)
        permutation_aleatoire(ind->ordre[k]);
    evaluer(ind);
}

static void sol_vers_individu(Sol *s, Individu *ind) {
    for (int k = 0; k < NM; k++) {
        for (int i = 0; i < NJ; i++) ind->ordre[k][i] = i;
        for (int i = 0; i < NJ - 1; i++)
            for (int j = i + 1; j < NJ; j++)
                if (s->start[ind->ordre[k][i]][k] > s->start[ind->ordre[k][j]][k]) {
                    int tmp          = ind->ordre[k][i];
                    ind->ordre[k][i] = ind->ordre[k][j];
                    ind->ordre[k][j] = tmp;
                }
    }
    evaluer(ind);
}

static void init_population(Individu *pop, Sol *spt, Sol *edd, Sol *ltr) {
    sol_vers_individu(spt, &pop[0]);
    sol_vers_individu(edd, &pop[1]);
    sol_vers_individu(ltr, &pop[2]);
    for (int i = 3; i < POP_SIZE; i++)
        individu_aleatoire(&pop[i]);
}

/* ================================================================
 * SELECTION PAR TOURNOI
 * ================================================================ */

static int selection_tournoi(Individu *pop) {
    int meilleur = rand_int(POP_SIZE);
    for (int i = 1; i < TAILLE_TOURN; i++) {
        int concurrent = rand_int(POP_SIZE);
        if (pop[concurrent].fitness < pop[meilleur].fitness)
            meilleur = concurrent;
    }
    return meilleur;
}

/* ================================================================
 * CROISEMENT OX (Order Crossover)
 * ================================================================ */

/* OX sur une machine : segment de p1, complement de p2 -> enfant */
static void ox_une_machine(int *p1, int *p2, int *enfant) {
    int i = rand_int(NJ);
    int j = rand_int(NJ);
    if (i > j) { int tmp = i; i = j; j = tmp; }

    int present[NJ];
    memset(present, 0, sizeof(present));

    /* Copier segment [i,j] de p1 */
    for (int x = i; x <= j; x++) {
        enfant[x]      = p1[x];
        present[p1[x]] = 1;
    }

    /* Completer avec p2 dans l'ordre */
    int pos_p2 = 0;
    for (int x = 0; x < NJ; x++) {
        if (x >= i && x <= j) continue;
        while (present[p2[pos_p2]]) pos_p2++;
        enfant[x]       = p2[pos_p2];
        present[p2[pos_p2]] = 1;
        pos_p2++;
    }
}

/* Croisement OX : produit 2 enfants depuis 2 parents
 * E1 : segment de Ch1 + complement Ch2
 * E2 : segment de Ch2 + complement Ch1  */
static void croisement(Individu *ch1, Individu *ch2,
                        Individu *e1,  Individu *e2) {
    if (rand_double() < TAUX_CROIS) {
        for (int k = 0; k < NM; k++) {
            ox_une_machine(ch1->ordre[k], ch2->ordre[k], e1->ordre[k]);
            ox_une_machine(ch2->ordre[k], ch1->ordre[k], e2->ordre[k]);
        }
        evaluer(e1);
        evaluer(e2);
    } else {
        /* Pas de croisement : enfants = copies des parents */
        copier_individu(e1, ch1);
        copier_individu(e2, ch2);
    }
}

/* ================================================================
 * MUTATION : swap aleatoire de 2 jobs sur une machine
 * ================================================================ */

static void mutation(Individu *ind) {
    if (rand_double() >= TAUX_MUT) return;

    int k = rand_int(NM);
    int i = rand_int(NJ);
    int j = rand_int(NJ);
    while (j == i) j = rand_int(NJ);

    int tmp          = ind->ordre[k][i];
    ind->ordre[k][i] = ind->ordre[k][j];
    ind->ordre[k][j] = tmp;

    evaluer(ind);
}

/* ================================================================
 * REMPLACEMENT : l'enfant remplace le moins bon si meilleur
 * ================================================================ */

static void remplacer_moins_bon(Individu *pop, Individu *enfant) {
    int idx_pire = 0;
    for (int i = 1; i < POP_SIZE; i++)
        if (pop[i].fitness > pop[idx_pire].fitness)
            idx_pire = i;
    if (enfant->fitness < pop[idx_pire].fitness)
        copier_individu(&pop[idx_pire], enfant);
}

/* ================================================================
 * ALGO GENETIQUE PRINCIPAL
 * ================================================================ */

void genetique(Sol *spt, Sol *edd, Sol *ltr, Sol *best) {
    srand((unsigned int)time(NULL));

    Individu pop[POP_SIZE];
    Individu e1, e2;

    /* 1. Initialisation */
    init_population(pop, spt, edd, ltr);

    /* Meilleur initial */
    int idx_best = 0;
    for (int i = 1; i < POP_SIZE; i++)
        if (pop[i].fitness < pop[idx_best].fitness)
            idx_best = i;

    Individu meilleur;
    copier_individu(&meilleur, &pop[idx_best]);
    printf("Genetique: depart fitness = %d\n", meilleur.fitness);

    /* 2. Boucle des generations */
    for (int gen = 0; gen < NB_GEN; gen++) {

        /* Selection de deux parents distincts */
        int idx_p1 = selection_tournoi(pop);
        int idx_p2 = selection_tournoi(pop);
        while (idx_p2 == idx_p1)
            idx_p2 = selection_tournoi(pop);

        /* Croisement -> 2 enfants */
        croisement(&pop[idx_p1], &pop[idx_p2], &e1, &e2);

        /* Mutation des deux enfants */
        mutation(&e1);
        mutation(&e2);

        /* Remplacement : chaque enfant remplace le moins bon si meilleur */
        remplacer_moins_bon(pop, &e1);
        remplacer_moins_bon(pop, &e2);

        /* Mise a jour du meilleur global */
        if (e1.fitness < meilleur.fitness) {
            copier_individu(&meilleur, &e1);
            printf("  Gen %d: amelioration -> fitness = %d\n", gen + 1, meilleur.fitness);
        }
        if (e2.fitness < meilleur.fitness) {
            copier_individu(&meilleur, &e2);
            printf("  Gen %d: amelioration (E2) -> fitness = %d\n", gen + 1, meilleur.fitness);
        }
    }

    printf("Genetique: resultat = %d\n\n", meilleur.fitness);

    /* 3. Reconstruire la Sol finale */
    int fin_machine[NM] = {0};
    int fin_job[NJ]     = {0};
    int pos[NM]         = {0};
    memset(best, 0, sizeof(Sol));

    int places = 0;
    while (places < NJ * NM) {
        int best_k = -1, best_j = -1, best_t = 1 << 30;
        for (int k = 0; k < NM; k++) {
            if (pos[k] >= NJ) continue;
            int j     = meilleur.ordre[k][pos[k]];
            int debut = fin_machine[k] > fin_job[j] ? fin_machine[k] : fin_job[j];
            if (debut < best_t) {
                best_t = debut;
                best_k = k;
                best_j = j;
            }
        }
        if (best_k == -1) break;
        best->start[best_j][best_k] = best_t;
        fin_machine[best_k]         = best_t + p[best_j][best_k];
        fin_job[best_j]             = best_t + p[best_j][best_k];
        pos[best_k]++;
        places++;
    }
}
