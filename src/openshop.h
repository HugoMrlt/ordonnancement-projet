#ifndef OPENSHOP_H
#define OPENSHOP_H

#define NJ 3  // nombre de jobs
#define NM 3  // nombre de machines

/* Donnees du probleme (definies dans main.c) */
extern int p[NJ][NM];  // durees : p[job][machine]
extern int d[NJ];       // echeances

/* Structure solution : date de debut de chaque operation */
typedef struct {
    int start[NJ][NM];
} Sol;

/* Utilitaires */
int max2(int a, int b);
int sum_tj(Sol *s);
void afficher(Sol *s, const char *nom);
void construire(int *ordre_jobs, Sol *s);

/* Heuristiques */
void heuristique_spt(Sol *s);
void heuristique_edd(Sol *s);
void heuristique_ltr(Sol *s);

/* VNS */
void vns(Sol *init, Sol *best);

#endif
