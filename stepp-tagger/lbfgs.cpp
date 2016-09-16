#include <vector>
#include <iostream>
#include <cmath>
#include <cstdio>
#include "mathvec.h"
#include "lbfgs.h"

using namespace std;

const static int    M = LBFGS_M;
const static double LINE_SEARCH_ALPHA = 0.1;
const static double LINE_SEARCH_BETA  = 0.5;

// stopping criteria
const static int    MAX_ITER      = 1000;
const static double MIN_GRAD_NORM = 0.0001;


template<class FuncGrad>
static double 
backtracking_line_search(FuncGrad func_grad,
			 const Vec & x0, const Vec & grad0, const double f0, 
			 const Vec & dx, Vec & x, Vec & grad1)
{
  double t = 1.0 / LINE_SEARCH_BETA;

  double f;
  do {
    t *= LINE_SEARCH_BETA;
    x = x0 + t * dx;
    f = func_grad(x.STLVec(), grad1.STLVec());
    //        cout << "*";
  } while (f > f0 + LINE_SEARCH_ALPHA * t * dot_product(dx, grad0));

  return f;
}

//
// Jorge Nocedal, "Updating Quasi-Newton Matrices With Limited Storage",
// Mathematics of Computation, Vol. 35, No. 151, pp. 773-782, 1980.
//
Vec 
approximate_Hg(const int iter, const Vec & grad,
	       const Vec s[],  const Vec y[], const double z[])
{
  int offset, bound;
  if (iter <= M) { offset = 0;        bound = iter; }
  else           { offset = iter - M; bound = M;    }

  Vec q = grad;
  double alpha[M], beta[M];
  for (int i = bound - 1; i >= 0; i--) {
    const int j = (i + offset) % M;
    alpha[i]    = z[j]   * dot_product(s[j], q);
    q          += -alpha[i] * y[j];
  }
  if (iter > 0) {
    const int j = (iter - 1) % M;
    const double gamma = ((1.0 / z[j]) / dot_product(y[j], y[j]));
    //    static double gamma;
    //    if (gamma == 0) gamma = ((1.0 / z[j]) / dot_product(y[j], y[j]));
    q *= gamma;
  }
  for (int i = 0; i <= bound - 1; i++) {
    const int j = (i + offset) % M;
    beta[i]     = z[j] * dot_product(y[j], q);
    q          += s[j] * (alpha[i] - beta[i]);
  }

  return q;
}

//template<class FuncGrad>
vector<double> 
perform_LBFGS(double (*func_grad)(const vector<double> &, vector<double> &), 
	      const vector<double> & x0)
{
  const size_t dim = x0.size();
  Vec x = x0;

  Vec grad(dim), dx(dim);
  double f = func_grad(x.STLVec(), grad.STLVec());

  Vec s[M], y[M];
  double z[M];  // rho

  for (int iter = 0; iter < MAX_ITER; iter++) {

      std::printf("%4d logl = %f\n", iter, -f);

    if (sqrt(dot_product(grad, grad)) < MIN_GRAD_NORM) break;

    dx = -1 * approximate_Hg(iter, grad, s, y, z);

    Vec x1(dim), grad1(dim);
    f = backtracking_line_search(func_grad, x, grad, f, dx, x1, grad1);

    s[iter % M] = x1 - x;
    y[iter % M] = grad1 - grad;
    z[iter % M] = 1.0 / dot_product(y[iter % M], s[iter % M]);
    x = x1;
    grad = grad1;
  }

  return x.STLVec();
}

