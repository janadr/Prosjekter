#include "catch.hpp"
#include "jacobis_method.hpp"

TEST_CASE("Sjekker om vi finner største verdi i matrisen"){
    int n = 3;
    double pmin=0, pmax=1;
    double h = (pmax-pmin)/(double(n));
    double hh = h*h;
    int k, l;
    double a = -1/hh;
    double d = 2/hh;

    mat A = zeros<mat>(n,n);
    mat S = zeros<mat>(n,n);
    double *analytical_eigval = new double [n];
    //initialize matrices and vector

    initialize(A, analytical_eigval, a, d, n);
    //find maximum matrix element
    double largest = find_largest(A, k, l, n);

    REQUIRE(k==0);
    REQUIRE(l==1);
    REQUIRE(largest==Approx(9));
}


TEST_CASE("Sjekker om eigenverdiene stemmer med analytiske verdier"){
  int n = 3
}