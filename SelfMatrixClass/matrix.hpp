// Error-Process Institution Upgraded :) 
// error-process is deprived of main code with the method of "template",
// whereby it can be edited easily and shown clearly.
// orientation: last barrier. it's the best that it can never be used.

#ifndef MATRIX_HPP
#define MATRIX_HPP
//content start ->

#include <iostream>
#include <vector>

using namespace std;

typedef vector<double> Vec;
typedef Vec Row;
typedef Vec Col;
typedef unsigned int Plus;

const char* const errReport[] = 
{
    "",
    "Error(type 1): a vector may be used without initialization.\n",
    "Error(type 2): a matrix may be used without initialization.\n",
    "Error(type 3): manipulation of vector with unmatched size.\n",
    "Error(type 4): manipulation of matrix with unmatched size.\n",
    "Error(type 5): misuse of a matrix with perverse size.\n",
    "Error(type 6): index is out of range.\n"
};

const char* const warnReport[]= 
{
    "Warning: "
};

class matrix {
// PRINCIPLE: Explicit conversion between vector and matrix.
//            vector can never be considered as matrix.
private:
    Plus height;
    Plus width;
    bool isempty;    
    Row* pmtx; 
public:
    matrix();
    matrix(Plus m,Plus n);
    matrix(const matrix& A);
    matrix(const matrix& A, int row_off, int col_off);
    ~matrix(){delete [] pmtx;};
    // Query function:    
    bool Empty(void) const;
    int Width(void) const;
    int Height(void) const;
    const Row& operator [] (int rowi); // random read-only access of a matrix (provided for users)
    bool operator == (const matrix& B) const;

    matrix operator *(const matrix& B);
    matrix operator +(const matrix& A);   //return a matrix that can be manipulated
    matrix operator *(double lambda) const;
    matrix& operator =(const matrix& A); 
    friend istream& operator >> (istream& a, matrix& A);
    friend ostream& operator << (ostream& b, const matrix& B);
    
    void appendRows(const matrix& augment);
    void appendCols(const matrix& augment);
    void setHeight(Plus h);
    void setWidth(Plus w);
    void dump(void);    //content devalidified
    void clear(void);   //all settings cancelled
    int approximate(void);
    
    int rowSwap(Plus s,Plus t);
    Vec rowMultiply(Plus rowi, double lambda);
    Vec rowSubstract(Plus rowi, Plus row_substractor, double ratio = 1);
    Vec row(Plus rowi) const; 
    int colSwap(Plus s, Plus t);
    Vec colMultiply(Plus coli, double lambda);
    Vec colSubstract(Plus coli, Plus col_substractor, double ratio = 1);
    Vec col(Plus coli) const;

    //useful tools 
    int eliminate(matrix* augment = NULL);  //return pivots number
    double determinant(void) const;
    bool invertible(void) const;
    matrix inverse(void) const; // by using augmented matrix 
    matrix transpose(void) const;
    matrix adjoint(void) const;
    matrix kernel(void) const;
    Vec specialSolution(Vec) const; // if no solution, an empty Vec will be returned 
    matrix orthogonalize(void) const;   //Gram-Schmdit method
    double trace(void) const;
    bool symmetric(void) const;
    matrix congruentTransform(const matrix& transMat) const; // the object will be rewritten, and it will return the transform matrix.
    
};

matrix operator *(double lambda, const matrix& A);
Row operator *(double lambda, Vec vec);
Row operator +(const Vec& r1, const Vec& r2);

// standard inner product:
double operator *(const Vec& v1, const Vec& v2);
// double power(double, int);

// inplement fo "template class" must be in .hpp 
// or the compiler don't know to use what template to generate the code. 
template<int T>
class Error
{
private:
    string message;
public:
    Error()
    {   
        cerr << errReport[T];
        system("pause");
    };
    Error(string errdata)
    {
        message = errdata;
        cerr << errReport[T];
        cerr << message;
        system("pause");
    };
};

template<int T>
class Warning
{
private:
    string message;
public:
    Warning()
    {
        cerr << warnReport[T];
    }
    Warning(const string& warndata)
    {
        cerr << warnReport[T];
        cerr << warndata;
    }

};

// -> end
#endif
