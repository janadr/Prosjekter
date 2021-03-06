#include "rossby_bounded.h"


int main(int argc, char *argv[]){
  ofstream outpsi, outzeta;
  string zetaname = "../data/zeta_bounded";
  string psiname = "../data/psi_bounded";

  // funksjonen tar tre cmd argument, dt, dx og slutt tid
  double deltapos = atof(argv[1]);
  double deltatime = atof(argv[2]);
  double endtime = atof(argv[3]);
  double endpos = 1.0;

  bool initialSine;
  if(atof(argv[4])==0){
    initialSine = true;
    zetaname += "_sine";
    psiname += "_sine";
  }
  else{
    initialSine = false;
    zetaname += "_gaussian";
    psiname += "_gaussian";
  }

  bool advanceForward;
  if(atof(argv[5])==0){
    advanceForward = true;
    zetaname += "_forward";
    psiname += "_forward";
  }
  else{
    advanceForward = false;
    zetaname += "_centered";
    psiname += "_centered";
  }
  zetaname += ".dat";
  psiname += ".dat";

  outpsi.open(psiname);
  outzeta.open(zetaname);

  int posdim = (int) endpos/deltapos-1;      //-1 siden endepunktene er kjente, og ikke beregnes
  int timedim = (int) endtime/deltatime;


  // initialising a vector of vectors (matrix) to hold a wave for every timestep
  vector<double> psi(posdim);
  vector<double> zeta(posdim);
  vector<double> zeta_previous;
  vector<double> zeta_2previous;

  // grensebetingelser til bølgen
  double psiClosed = 0;

  // matriseelementer
  vector<double> a(posdim);
  vector<double> b(posdim);
  vector<double> c(posdim);
  vector<double> d(posdim);
  vector<double> x(posdim);

  initWave(posdim, deltapos, psi, zeta, initialSine);
  zeta_2previous = zeta;
  zeta_previous = zeta;
  for(int n = 0; n < timedim; ++n){
    outpsi<< setw(15) << psiClosed;      // skrivet ut venstre BC
    writePsi(outpsi, psi[0]);            // skriver ut første verdi til fil
    // finner den første x-verdien til zeta
    if(advanceForward){
      advance_vorticity_forward(zeta[0], psi[1], psiClosed, deltatime, deltapos);
    }
    else{
      advance_vorticity_centered(zeta[0], zeta_2previous[0], psi[1], psiClosed, deltatime, deltapos);
    }
    for(int j = 1; j < posdim - 1; ++j){
      if(advanceForward){
        advance_vorticity_forward(zeta[j], psi[j+1], psi[j-1], deltatime, deltapos);
      }
      else{
        advance_vorticity_centered(zeta[j], zeta_2previous[j], psi[j+1], psi[j-1], deltatime, deltapos);
      }
      writeZeta(outzeta, zeta[j]);
      writePsi(outpsi, psi[j]);
    }
    if(advanceForward){
      advance_vorticity_forward(zeta[posdim-1], psiClosed, psi[posdim-2], deltatime, deltapos);
    }
    else{
      advance_vorticity_centered(zeta[posdim-1], zeta_2previous[posdim-1], psiClosed, psi[posdim-2], deltatime, deltapos);
    }
    zeta_2previous = zeta_previous;
    zeta_previous = zeta;
    writePsi(outpsi, psi[posdim-1]);    // skriver ut siste verdi til fil
    outpsi << setw(15) << psiClosed;    // skriver ut høyre BC
    outzeta << endl;
    outpsi << endl;
    initMatrixElements(posdim, deltapos, zeta, x, a, b, c, d);
    forward_sub(posdim, a, b, c, d);
    backward_sub(posdim, psi, b, c, d);
  }
  outpsi.close();
  outzeta.close();
  return 0;
}


//initsaliserer strømfunksjonen
void initWave(int posdim, double deltapos, vector<double> &psi, vector<double> &zeta, bool initialSine){
  double x;
  //double h = 1.0/(posdim + 1.0;
  double sigma = 0.1; double x0 = 0.5;
  for(int j = 0; j < posdim; ++j){
    x = (j + 1)*deltapos;
    if(initialSine){
      zeta[j] = sinewaveDerivative(x);
      psi[j] = sinewave(x);
    }
    else{
      zeta[j] = gaussianDerivative(x, x0, sigma);
      psi[j] = gaussian(x, x0, sigma);
    }

  }
  return;
}


// initialiserer matriseelementene og funksjonsverdiene
void initMatrixElements(int posdim, double deltapos, vector<double> zeta, vector<double> &x, vector<double> &a,
                        vector<double> &b, vector<double> &c, vector<double> &d) {
  const double h = deltapos;
  const double hh = h*h;
  for(int j = 0; j < posdim; ++j) {
    x[j] = (j + 1)*h;
    a[j] = 1.0;
    b[j] = -2.0;
    c[j] = 1.0;
    d[j] = hh*zeta[j];
  }
  return;
}

// utfører gaussisk eliminasjon for å redusere likningssettet
void forward_sub(int posdim, vector<double> &a, vector<double> &b,
                 vector<double> &c, vector<double> &d) {
  for(int j = 1; j < posdim; ++j) {
    const double k = a[j-1]/b[j-1];
    b[j] -= k*c[j-1];
    d[j] -= k*d[j-1];
  }
  return;
}


// løser likningssettet for v-vektoren
void backward_sub(int posdim, vector<double> &psi, vector<double> &b,
                  vector<double> &c, vector<double> &d) {
  psi[posdim-1] = d[posdim-1]/b[posdim-1];
  for(int j = posdim - 2; j >= 0; --j) {
    psi[j] = (d[j] - c[j]*psi[j+1])/b[j];
  }
  return;
}



void advance_vorticity_forward(double &zeta_forward, double psi_forward,
                              double psi_backward, double deltatime,
                              double deltapos){
  zeta_forward -= (deltatime/(2.0*deltapos))*(psi_forward - psi_backward);
  return;
}

void advance_vorticity_centered(double &zeta_forward, double zeta_backward,
                                double psi_forward, double psi_backward,
                                double deltatime, double deltapos){
  zeta_forward = (deltatime/deltapos)*(psi_backward-psi_forward) + zeta_backward;
  return;
}

void writePsi(ofstream &outpsi, double &psivalue){
  outpsi << setiosflags(ios::showpoint | ios::uppercase);
  outpsi << setw(15) << setprecision(8) << psivalue;
  return;
}

void writeZeta(ofstream &outzeta, double &zetavalue){
  outzeta << setiosflags(ios::showpoint | ios::uppercase);
  outzeta << setw(15) << setprecision(8) << zetavalue;
  return;
}
