#ifndef LINEAR_SOLVER
#define LINEAR_SOLVER

#include "../std.h"

struct LinearSolver {

	uint32_t n;
	vector<double> line;
	vector<vector<double>> A;
	// ids from 1 to n
	LinearSolver(uint32_t n): n(n), line(n+1,0), A(n,line) {}

 	void print() {
		for (int i=0; i<n; i++) {
		    for (int j=0; j<n+1; j++) {
		        cout << A[i][j] << "\t";
		        if (j == n-1) {
		            cout << "| ";
		        }
		    }
		    cout << "\n";
		}
		cout << endl;
	}


	void addCoefficient(uint32_t i, uint32_t j, double co) {
	  	A[i][j] += co;
	}
  
	void addConstant(uint32_t i, double co) {
  		A[i][n] += co;
  	}

  	// solve retunrs a vector ans where ans[i] is the solution for i-th variable
	vector<double> solve() {
	    for (int i=0; i<n; i++) {
	        // Search for maximum in this column
	        double maxEl = abs(A[i][i]);
	        int maxRow = i;
	        for (int k=i+1; k<n; k++) {
	            if (abs(A[k][i]) > maxEl) {
	                maxEl = abs(A[k][i]);
	                maxRow = k;
	            }
	        }

	        // Swap maximum row with current row (column by column)
	        for (int k=i; k<n+1;k++) {
	            double tmp = A[maxRow][k];
	            A[maxRow][k] = A[i][k];
	            A[i][k] = tmp;
	        }

	        if (abs(A[i][i]) < EPSILON)
	        	continue;
	        // Make all rows below this one 0 in current column
	        for (int k=i+1; k<n; k++) {
	            double c = -A[k][i]/A[i][i];
	            for (int j=i; j<n+1; j++) {
	                if (i==j) {
	                    A[k][j] = 0;
	                } else {
	                    A[k][j] += c * A[i][j];
	                }
	            }
	        }
	    }

	    // Solve equation Ax=b for an upper triangular matrix A
	    vector<double> x(n);
	    for (int i=n-1; i>=0; i--) {
	    		if (abs(A[i][i]) >= EPSILON) 
	        	x[i] = A[i][n]/A[i][i];
	        else
	        	x[i] = 0;
	        for (int k=i-1;k>=0; k--) {
	            A[k][n] -= A[k][i] * x[i];
	        }
	    }
	    return x;
	}
};

#endif // LINEAR_SOLVER



