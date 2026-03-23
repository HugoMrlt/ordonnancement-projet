//Paramètres

//bigM est une grande constante
int bigM = 10000;

//nombre de jobs
int n = ...;
range NBJOBS = 1..n;

//nombre de machines
range NBMACHINE = 1..3;

//P[j][i] : temps d'éxecution de la tache j sur la machine i
int P[NBJOBS][NBMACHINE]= ...;

//D[i] : date d'écheance du job i
int D[NBJOBS]= ...;

/* 	y[k][i]jk] : variable de décision indiquant si pour une machine k la tache i 
* 	se fait avant ou après la tache j
*/
dvar boolean Y[NBMACHINE][NBJOBS][NBJOBS];

/*	x[j][k][k_prime] : variable de décision indiquant si pour un job j la machine k 
*	l'éxecute avant ou après la machine k_prime
*/
dvar boolean X[NBJOBS][NBMACHINE][NBMACHINE];

//C[i][j] : date de fin du job i dans la machine j
dvar float+ C[NBJOBS][NBMACHINE];

//CMax[i] : date de fin du job i
dvar float+ CMax[NBJOBS];

//T[i] : retard du job i
dvar float+ T[NBJOBS];

minimize
	   sum(i in NBJOBS) T[i];
subject to {
  //contrainte sur les machines
  forall(k in NBMACHINE, i,j in NBJOBS: i!=j) {
    C[i][k]>=C[j][k]+P[i][k]-bigM*(1-Y[k][j][i]);
    Y[k][i][j] + Y[k][j][i] == 1;
  }
  //contrainte sur les jobs
  forall(j in NBJOBS, k,k_prime in NBMACHINE: k!=k_prime) {
    C[j][k_prime]>=C[j][k]+P[j][k_prime]-bigM*(1-X[j][k][k_prime]);
    X[j][k][k_prime] + X[j][k_prime][k] == 1;
  }
  forall(j in NBJOBS, k in NBMACHINE)
    C[j][k] >= P[j][k];
  //temps de completion
  forall(j in NBJOBS, k in NBMACHINE)
    CMax[j] >= C[j][k];
  //retard
  forall(j in NBJOBS){
    T[j] >= CMax[j] - D[j];  
  }    
}

execute Display{

  writeln("=========== RESULTATS ===========");

  writeln("Retard total : ",cplex.getObjValue());
  
}
