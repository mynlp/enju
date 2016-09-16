//---------------------------------------------------------------
// lbfgs.h
//
// Copyright (c) 2007, Yoshimasa Tsuruoka
//---------------------------------------------------------------

#ifndef _LBFGS_H_
#define _LBFGS_H_

//template<class FuncGrad>
//std::vector<double> 
//perform_LBFGS(FuncGrad func_grad, const std::vector<double> & x0);

std::vector<double> 
perform_LBFGS(double (*func_grad)(const std::vector<double> &, std::vector<double> &), 
	      const std::vector<double> & x0);


std::vector<double> 
perform_OWLQN(double (*func_grad)(const std::vector<double> &, std::vector<double> &), 
	      const std::vector<double> & x0,
	      const double C);

const int    LBFGS_M = 7;

#endif
