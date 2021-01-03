#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

#define debug 1

inline void swap(double& a, double& b){
	double temp = a;
	a = b;
	b = temp;
}

void rowswap(int rowi, int rowj, int n, double matrix[][100]){
	if(rowi != rowj){
		for(int i = 0; i < n; i++){
			swap(matrix[rowi][i], matrix[rowj][i]);
		}
	}
}

void rowsubstract(int rowi, int rowj, int n, double matrix[][100]){
	for(int i = 0; i < n; i++){
		matrix[rowi][i] -= matrix[rowj][i];
	}
}

void rowmultiply(int rowi, double lambda, int n, double matrix[][100]){
	for(int i = 0; i < n; i++){
		matrix[rowi][i] *= lambda;
	}
}

int findptv(int coli, int m, double matrix[][100], int rstart){
	for(int i = rstart; i < m; i++){
		if(abs(matrix[i][coli]) > 1e-5) {
			return i;
		}
	}
	return -1;
}

int main(){
	printf("Welcome to Gauss elimination!\nPlease enter the size of the matrix, and enter the entries\n");
	int m, n;
	printf("row:");
	cin>>m;
	printf("colomn:");
	cin>>n;
	double matrix[100][100];
	for(int i = 0; i < m; i++){
		for(int j = 0; j < n; j++){
			cin>>matrix[i][j];
		}
	}
    int pivots = 0;
	for(int j = 0; j < n; j++){
		int pos = findptv(j, m, matrix, pivots);	//0 included
		if(pos == -1) continue;
		rowswap(pivots, pos, n, matrix);         //line 0 has been reset!
		double lambda = 1/matrix[pivots][j];       //lambda is non-zero
		rowmultiply(pivots, lambda, n, matrix);
		for(int i = pivots + 1; i < m; i++){
			if(abs(matrix[i][j]) > 1e-5 ) {
				double ratio = 1/matrix[i][j];
				rowmultiply(i, ratio, n, matrix);
				rowsubstract(i, pivots, n, matrix);
			} 
		}
		pivots++;
	}
	printf("%s", "The eliminated matrix is as below:\n" );
	for(int i = 0; i < m; i++){
			for(int j = 0; j < n; j++){
				if(abs(matrix[i][j]) < 1e-5 ) matrix[i][j] = 0;
				cout<<matrix[i][j]<<" ";
			}
			cout<<endl;
		}
	system("pause"); 
//	printf("%s\n", "When you take it down, enter an arbitary character to end the program!");
//    char ch;
//    cin >> ch;
	return 0;
}
// system("pause") 可以让窗口停顿，防止一闪而过 
