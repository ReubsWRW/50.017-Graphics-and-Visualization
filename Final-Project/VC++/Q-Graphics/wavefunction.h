#ifndef WAVEFUNCTION_H
#define WAVEFUNCTION_H

#include "Header.h"

class wavefunction {

public:

	wavefunction() {} // constructor

	~wavefunction() {} // destructor

	// method declarations
	cdV ISW_eigenstate(int n, double L, cdV x, double t);
	cdV ISW_uniform_state(int n, double L, cdV x, double t, int num_states);
	cdV ISW_exp_state(int n, double L, cdV x, double t, int num_states);
	cplex coeff(int n1, double L1, int n, double L, double dx, cdV x, double t);
	cdV wavefun(wavefunction* wf, cdV coeffs, cdM wavefunctions);


	void set_width(double ISW_width) { L = ISW_width; } // sets width of ISW to argument value
	double get_width() { return L; } // returns width of the ISW

	void set_level(int level) { n = level; } // sets quantization index to argument value
	int get_level() { return n; } // returns quantization index
	

private:

	// width variables
	int n = n0; // quantization index
	double L = L0;

};





#endif // WAVEFUNCTION_H