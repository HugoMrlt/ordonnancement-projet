/*********************************************
 * OPL 22.1.1.0 Model
 * Author: katoe
 * Creation Date: 23 mars 2026 at 15:49:42
 *********************************************/

int nbJobs = ...;           // Nombre de jobs
int NbMachines = ...;  // Nombre de machines (K)
float BigM = 10000;    //

range Jobs = 1..nbJobs;
range Machines = 1..NbMachines;

float p[Jobs][Machines] = ...; // Temps de traitement 
float d[Jobs] = ...;           // Dates d'échéance


// Variables de décision
dvar boolean Y[Machines][Jobs][Jobs]; 
dvar boolean X[Jobs][Machines][Machines]; 

// Date de fin de l'opération du job j sur la machine k (C_{j,k})
dvar float+ C_op[Jobs][Machines]; 

// Date de fin globale du job j (C_j)
dvar float+ C[Jobs]; 

// Retard du job j (T_j)
dvar float+ T[Jobs]; 

// Fonction Objectif :  Minimiser la somme des retards
minimize sum(j in Jobs) T[j];


// Contraintes

subject to {
    
    // Exclusion mutuelle des jobs sur une même machine
    forall(k in Machines, i in Jobs, j in Jobs : i != j)
        Y[k][i][j] + Y[k][j][i] == 1;

    // Exclusion mutuelle des opérations d'un même job
    forall(j in Jobs, k in Machines, k_prime in Machines : k != k_prime)
        X[j][k][k_prime] + X[j][k_prime][k] == 1;

    // Contrainte de précédence/disjonction sur une machine
    forall(k in Machines, i in Jobs, j in Jobs : i != j)
        C_op[i][k] >= C_op[j][k] + p[i][k] + (Y[k][j][i] - 1) * BigM;

    // Contrainte de précédence/disjonction pour un même job
    forall(j in Jobs, k in Machines, k_prime in Machines : k != k_prime)
        C_op[j][k_prime] >= C_op[j][k] + p[j][k_prime] + (X[j][k][k_prime] - 1) * BigM;

    //La date de fin du job est supérieure ou égale à la fin de toutes ses opérations
    forall(j in Jobs, k in Machines)
        C[j] >= C_op[j][k];

    // Calcul du retard
    forall(j in Jobs)
        T[j] >= C[j] - d[j];
       
    forall(j in Jobs, k in Machines){
      C_op[j][k]>=p[j][k];
    }
    
 

}
   execute Display{
      writeln("=================RESULTATS=================");
      writeln("Retard total T : ", cplex.getObjValue());
    }