#include <iostream>
#include <vector>
#include "matrix.hpp"
#include <algorithm>

using namespace std;

#define debug true

//NOTE: you can't create a 2-dimension array without defining either row or col
//solution: use vector class in STL to make length of row changeable 

//defination
int matrix::approximate(void){
    if(isempty)
        throw Error<2>();
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if(pmtx[i][j] < 1e-7 && pmtx[i][j] >-1e-7)
                pmtx[i][j] = 0.0;
    return 0;
}
void matrix::setHeight(Plus h) {
    if(!h)
        return ;
    if(height == h)
        return ;
    this->isempty = 1;
    this->height = h;
    if(pmtx) {
        Row* ptemp = this->pmtx;
        delete [] ptemp;
    }
    this->pmtx = new Row[h];
    return ;
}
void matrix::setWidth(Plus w) {
    if(!w)
        return ;
    if(width == w)
        return ;
    this->isempty = 1;
    this->width = w;
    for(int i = 0; i < height; i++)
        this->pmtx[i].clear();
    return ;
}
int matrix::Width(void)  const{  return width;  };
int matrix::Height(void) const{ return height;  };
bool matrix::Empty(void) const{ return isempty; };
void matrix::dump(void) {isempty = 1;}
void matrix::clear(void)
{
    Row* ptemp = pmtx;
    delete [] ptemp;
    pmtx = NULL;
    isempty = 1;
    width = 0;
    height = 0;
}


matrix::matrix():isempty(1), width(0), height(0), pmtx(NULL){};
//仅有缺省构造函数可能产生宽或高为0的矩阵

matrix::matrix(Plus m, Plus n){
    if(m != 0 && n != 0){
        height = m;
        width = n;
        isempty = 1;
        pmtx = new Row[m];
    }
    while(m == 0 || n == 0){
        cerr << errReport[5];
        cout << "Now the matrix's size is:\tHeight: " << m;
        cout << "\tWidth: " << n <<endl;
        cout << "Please set the size of matrix\n";
        cout << "Height: ";     cin >> m;
        cout << "Width: ";      cin >> n;
        if(m != 0 && n != 0){
            height = m;
            width = n;
            isempty = 1;
            pmtx = new Row[m];
        }
    }
};

matrix::matrix(const matrix& A){
    if(A.isempty)
        throw Error<2>();
    isempty = A.isempty;
    width = A.width;
    height = A.height;
    pmtx = new Row[height];
    for(int i = 0; i < height; i++)
        pmtx[i] = A.pmtx[i];
}

matrix::matrix(const matrix& A, int row_off, int col_off){
    if( A.isempty )
        throw Error<2>();
    else{
        if( !(row_off >= 0 && row_off < A.height)&& !(col_off >= 0 && col_off < A.width) )
        {   
            //no row or column off:
            isempty = A.isempty;
            height = A.height;
            width = A.width;
            pmtx = new Row[height];
            for(Plus i = 0; i < height; i++){
                *(pmtx + i) = *(A.pmtx + i);
            }
        }
        else if( !(row_off >= 0 && row_off < A.height)&&(col_off >= 0 && col_off < A.width) )
        {
            //no row off, one col off
            height = A.height;
            width = A.height - 1;
            if(height <= 0 || width <= 0)
                throw Error<5>();
            else{
                isempty = A.isempty;
                pmtx = new Row[height];
                for(Plus i = 0; i < height; i++){
                    *(pmtx + i) = *(A.pmtx + i);
                    (pmtx + i)->erase(pmtx[i].begin()+col_off);
                }
            }
        }
        else if( (row_off >= 0 && row_off < A.height)&&!(col_off >= 0 && col_off < A.width) )
        {
            //no column off, one row off
            height = A.height - 1;
            width = A.width;
            if(height <= 0 || width <= 0)
                throw Error<5>();
            else{
                isempty = A.isempty;
                pmtx = new Row[height];
                for(Plus i = 0; i < row_off; i++)
                    *(pmtx + i) = *(A.pmtx + i);
                for(Plus i = row_off; i < height; i++)
                    *(pmtx + i) = *(A.pmtx + i + 1);
            }
        }
        else if(  (row_off >= 0 && row_off < A.height)&&(col_off >= 0 && col_off < A.width) )
        {
            height = A.height -1;
            width = A.width -1;
            if(height <= 0 || width <= 0)
                throw Error<5>();
            else{
                isempty = A.isempty;
                pmtx = new Row[height];
                for(Plus i = 0; i < row_off; i++)
                {
                    *(pmtx + i) = *(A.pmtx + i);
                    (pmtx + i)->erase(pmtx[i].begin()+col_off);
                }
                for(Plus i = row_off; i < height; i++)
                {
                    *(pmtx + i) = *(A.pmtx + i + 1);
                    (pmtx + i)->erase(pmtx[i].begin()+col_off);
                }
            }
        }
    }   
}   

//overload operator "*" & "+" to make row mutiplication easier. 
Row operator *(double lambda, Row row){
    if(row.empty())
        throw Error<1>();
    int l = row.size();
    for(int i = 0; i < l; i++){
        row[i] *= lambda;
    }
    return row;
}
Row operator +(const Row& r1, const Row& r2){
    if( r1.empty() || r2.empty() )
        throw Error<1>();
    int l0 = r1.size(); int l1 = r2.size();
    if(l0 != l1) 
        throw Error<3>();
    else{
        Vec r(r1);
        for(int i = 0; i < l0; i++){
            r[i] += r2[i];
        }
        return r;
    }
}

//calculate dot product(standard inner producct):
double operator *(const Vec& v1, const Vec& v2)
{
    if (v1.empty() || v2.empty())
        throw Error<1>();
    int l1 = v1.size();
    int l2 = v2.size();
    if (l1 != l2)
        throw Error<3>();
    double dotProduct = 0;
    for (int i = 0; i < l1; i++)
        dotProduct += v1[i]*v2[i];
    return dotProduct;    
}   

matrix matrix::operator *(const matrix& B){
    if( isempty || B.isempty )
        throw Error<2>();
    if(width != B.height)
        throw Error<4>();
    int m = height, n = width, s = B.width;
    matrix Rlt(m, s);
    //do row mutiplication here:
    for(int i = 0; i < m; i++){
        Rlt.pmtx[i] = (*(pmtx+i))[0] * (*B.pmtx);
        for(int j = 1; j < n; j++){
            Rlt.pmtx[i] = Rlt.pmtx[i] + (*(pmtx+i))[j] * (*(B.pmtx+j));
        }
    }
    Rlt.isempty = 0;
    //mutiplication's done!
    return Rlt;
}

//!(*A.pmtx).empty() is to test whether A has been initialized by checking the empty of A's first row 
matrix& matrix::operator =(const matrix& A){
    if(A.isempty)
        throw Error<2>();
    if(height == 0 || width == 0){
        height = A.height;
        width = A.width;
        pmtx = new Row[height];
    } 
    else if(A.height != height){
        this->clear();
        this->height = A.height;
        this->width = A.width;
        pmtx = new Row[height];
    }

    //to prevent self-assigned
    if(this == &A)
        return *this;
    for(Plus i = 0; i < height; i++)    
        pmtx[i] = A.pmtx[i];
    this->isempty = 0;
    this->width = A.width;
    return *this;
}

matrix matrix::operator +(const matrix& A){
    if(isempty || A.isempty)
        throw Error<2>();
    if(height != A.height || width != A.width)
        throw Error<4>();
    matrix Rlt = *this;
    for(Plus i = 0; i < height; i++){
        Rlt.pmtx[i] = Rlt.pmtx[i] + A.pmtx[i];
    }
    Rlt.isempty = 0;
    return Rlt;
}

istream& operator >> (istream& a, matrix& A){
    double enter;
    while(A.height == 0 || A.width == 0){
        cerr << errReport[5];
        cout << "Now the matrix's size is:\tHeight: " << A.height;
        cout << "\tWidth: " << A.width <<endl;
        cout << "Please set the size of matrix\n";
        cout << "Height: ";     cin >> A.height;
        cout << "Width: ";      cin >> A.width;
        if(A.height != 0 && A.width != 0){
            A.pmtx = new Row[A.height];
            A.isempty = 1;
        }
    }
    
    if(A.pmtx[0].empty())
        for(Plus i = 0; i < A.height; i++)
            for(Plus j = 0; j < A.width; j++){
                a >> enter;
                A.pmtx[i].push_back(enter);
            }
    // if(debug) cout <<"data received\n";
    
    else 
        for(Plus i = 0; i < A.height; i++)
            for(Plus j = 0; j < A.width; j++){
                a >> enter;
                A.pmtx[i][j] = enter;
            }
    A.isempty = 0;
    return a;
}

ostream& operator << (ostream& b, const matrix& B){
    if(B.isempty)
        throw Error<2>();
    for(Plus i = 0; i < B.height; i++){ 
        for(double& number: B.pmtx[i])
        {
            if(number > 1e-7 || number < -1e-7)
                b << number <<" ";
            else 
                b << 0 << " ";
        }   
        b << endl; 
    }
    return b;
}

matrix matrix::operator *(double lambda) const{
    if(isempty)
        throw Error<2>();
    matrix Rlt(*this);
    for(Plus i = 0; i < height; i++) Rlt.pmtx[i] = lambda*pmtx[i];
    return Rlt;
}

matrix operator *(double lambda, const matrix& A){
    return A*lambda;
}

Row& matrix::operator [] (int rowi)
{
    if (isempty)
        throw Error<2>();
    return this->pmtx[rowi];
}

matrix matrix::transpose(void) const{
    if(isempty)
        throw Error<2>();
    int n = height, m = width;
    matrix Rlt(m,n);
    Rlt.isempty = 0;
    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            Rlt.pmtx[i].push_back(pmtx[j][i]);
        }
    }
    return Rlt;
}

int findptv(int col_in, int height, Row* p_matrix, int row_start = 0){
    for(int i = row_start; i < height; i++)
    {   
        if( p_matrix[i][col_in] > 1e-5 || p_matrix[i][col_in] < -1e-5)
            return i;
        else
            p_matrix[i][col_in] = 0;
    }       
    return -1;
}

int matrix::rowSwap(Plus s,Plus t){
    if(isempty)
        throw Error<2>();
    if(s < height && t < height && s != t){
        Row temp = pmtx[s];
        pmtx[s] = pmtx[t];
        pmtx[t] = temp;
        return 0;
    }
    return -1;
}

Vec matrix::rowMultiply(Plus rowi, double lambda){
    if (isempty)
        throw Error<2>();
    int M = this->height;
    Vec rlt;
    if (rowi < height)
    {
        for(double& real: pmtx[rowi]){
            if( real > -1e-5 && real < 1e-5)
                real = 0;
            else
                real *= lambda;
            rlt.push_back(real);
        }
    }
    return rlt;
}

Vec matrix::rowSubstract(Plus rowi, Plus row_substractor, double ratio){
    if (isempty)
        throw Error<2>();
    int M = this->height;
    Vec rlt;
    if (rowi < M && row_substractor < M)
    {
        for(int j = 0; j < width; j++){
            if(pmtx[rowi][j] > -1e-5 && pmtx[rowi][j] < 1e-5)
                pmtx[rowi][j] = 0;
            pmtx[rowi][j] -= ratio*pmtx[row_substractor][j];
            rlt.push_back(pmtx[rowi][j]);
        }
    }
    return rlt;
}

Vec matrix::row(Plus rowi) const
{
    if (isempty)
        throw Error<2>();
    if (rowi >= height)
        throw Error<6>();
    return pmtx[rowi];
}

int matrix::colSwap(Plus s, Plus t)
{
    if (isempty)
        throw Error<2>();
    int M = this->height;
    int N = this->width;
    if (s >= N || t >= N)
        throw Error<6>();
    else if (s != t)
    {
        for (int i = 0; i < M; i++)
            swap(pmtx[i][s], pmtx[i][t]);
        return 0;
    }
    return -1;
}

Vec matrix::colMultiply(Plus coli, double lambda)
{
    if (isempty) 
        throw Error<2>();
    int M = this->height;
    int N = this->width;
    Vec rlt;
    if (coli >= N)
        throw Error<6>();
    else
    {
        for (int i = 0; i < M; i++)
        {
            pmtx[i][coli] *= lambda;
            if(pmtx[i][coli] > -1e-5 && pmtx[i][coli] < 1e-5)
                pmtx[i][coli] = 0;
            rlt.push_back(pmtx[i][coli]);
        }
    }
    return rlt;
}

Vec matrix::colSubstract(Plus coli, Plus col_substractor, double ratio)
{
    if (isempty)
        throw Error<2>();
    int M = this->height;
    int N = this->width;
    Vec rlt;
    if (coli >= N || col_substractor >= N)
        throw Error<6>();
    else 
    {
        for (int i = 0; i < M; i++)
        {
            if(pmtx[i][coli] > -1e-5 && pmtx[i][coli] < 1e-5)
                pmtx[i][coli] = 0;
            pmtx[i][coli] -= ratio*pmtx[i][col_substractor];
            rlt.push_back(pmtx[i][coli]);
        }
    }
    return rlt;
}

Vec matrix::col(Plus coli) const
{
    if (isempty)
        throw Error<2>();
    if (coli >= width)
        throw Error<6>();
    Vec rlt;
    for (int i = 0; i < height; i++)
        rlt.push_back(pmtx[i][coli]);
    return rlt;    
}

int matrix::eliminate(matrix *augment){
    if(isempty)
        throw Error<2>();
    //start
    int pivots = 0;
    if (augment == NULL)
    {
        int m = height, n = width;
        int find_start = 0;
        for(int j = 0; j < n; j++){
            int pos = findptv(j, m, pmtx, find_start);  //0 included
            if(pos == -1) continue;
            this->rowSwap(find_start, pos);         //line 0 has been reset!
            this->rowMultiply(find_start, 1/pmtx[find_start][j]);       //lambda is non-zero
            for(int i = 0; i < m; i++){
                if( i == find_start )
                    continue;
                if( pmtx[i][j] > 1e-5 || pmtx[i][j] < -1e-5) {
                    double ratio = pmtx[i][j];
                    this->rowSubstract( i, find_start, ratio );
                } 
                else
                    pmtx[i][j] = 0;
            }
            pivots++;
            find_start++;
        }
        this->approximate();
    }

    else
    {
        int m = height, n = width;
        int find_start = 0;
        for(int j = 0; j < n; j++){
            int pos = findptv(j, m, pmtx, find_start);  //0 included
            if(pos == -1) continue;
            augment->rowSwap(find_start, pos);
            this->rowSwap(find_start, pos);         //line 0 has been reset!
            augment->rowMultiply(find_start, 1/pmtx[find_start][j]);
            this->rowMultiply(find_start, 1/pmtx[find_start][j]);       //lambda is non-zero
            for(int i = 0; i < m; i++){
                if( i == find_start )
                    continue;
                if( pmtx[i][j] > 1e-5 || pmtx[i][j] < -1e-5) {
                    double ratio = pmtx[i][j];
                    augment->rowSubstract(i, find_start, ratio);
                    this->rowSubstract(i, find_start, ratio);
                } 
                else
                    pmtx[i][j] = 0;
            }
            pivots++;
            find_start++;
        }
        augment->approximate();
        this->approximate();
    }

    return pivots;
}                       //time complexity:O(mn)

matrix matrix::inverse(void) const{
    if(isempty || width != height ){
        if(isempty)
            throw Error<2>();
        else
            throw Error<4>("The matrix is not square!, which has no inverse\n");
    }
    matrix A(*this);
    int n = width;
    matrix In(n, n);
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(i == j) 
                In.pmtx[i].push_back(1);
            else
                In.pmtx[i].push_back(0);
        }
    }
    In.isempty = 0;
    // old version:
    // int find_start = 0;
    // int pivots = 0;
    // for(int j = 0; j < n; j++){
    //     int pos = findptv(j, n, A.pmtx, find_start);    //0 included
    //     if(pos == -1) continue;
    //     A.rowSwap(find_start, pos);         //line 0 has been reset!
    //     In.rowSwap(find_start, pos);
    //     In.rowMultiply(find_start, 1/A.pmtx[find_start][j]);       //lambda is non-zero
    //     A.rowMultiply(find_start, 1/A.pmtx[find_start][j]);
    //     for(int i = 0; i < n; i++){
    //         if(i == find_start)
    //             continue;
    //         if( A.pmtx[i][j] > 1e-5 || A.pmtx[i][j] < -1e-5) {
    //             double ratio = A.pmtx[i][j];
    //             A.rowSubstract( i, find_start, ratio );
    //             In.rowSubstract( i, find_start, ratio );
    //         } 
    //     }
    //     find_start++;
    //     pivots++;
    // }
    int pivots = A.eliminate(&In);
    if(pivots < n){
        cout << endl;
        for(int i = 0; i < 50; i++)
            cout << '*';
        cout << endl;
        cout << *this;
        cout << "Sorry, this matrix(above) has no inverse.\n";
        cout << "You may want its pseudo-inverse, which has performed as an alternative\n";
        for(int i = 0; i < 50; i++)
            cout << '*';
        cout << endl; 
    } 
    return In;
} 

bool matrix::invertible(void) const{
    if( isempty )
        throw Error<2>();
    if( height != width )
        return false;
    matrix temp(*this);
    if( height != temp.eliminate() )
        return false;
    return true;
}

double matrix::determinant(void) const
{
    if(isempty)
        throw Error<2>();
    if(height != width)
        throw Error<4>("The matrix is not square, which has no determinant");
    // initialized square matrix:
    
    if(height == 2)
        return pmtx[0][0]*pmtx[1][1] - pmtx[0][1]*pmtx[1][0];
    else if(height > 2){
        double sum = 0.0;
        int sgn = 1;
        for(int i = 0; i < height; i++){
            sum += sgn*pmtx[0][i]*matrix(*this,0,i).determinant();
            sgn *= -1;
        }
        return sum;
    }
    if(height == 1)
        return pmtx[0][0];
}

double power(double base, int degree){
    if(degree == 0)
        return 1;
    else if(degree > 0) {
        double product = 1.0;
        while(degree--)
            product *= base;
        return product;
    }
    else {
        degree = -degree;
        double product = 1.0;
        while(degree--)
            product *= base;
        return 1.0 / product;
    }
}

matrix matrix::adjoint() const{
    if( isempty )
        throw Error<2>();  
    if( width != height )
        throw Error<4>("The matrix is not square, which has no adjoint matrix!\n");
    int n = width;
    matrix Rlt(n,n);
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            Rlt.pmtx[i].push_back( power(-1,i+1+j+1)*matrix(*this, i, j).determinant() );
    Rlt.isempty = 0;
    return Rlt.transpose();
}

matrix matrix::kernel(void) const
{
    matrix mtxTemp(*this);
    int dimKel = mtxTemp.width - mtxTemp.eliminate();
    matrix Rlt;
    if (dimKel == 0)
    {
        Rlt.setWidth(1);
        Rlt.setHeight(mtxTemp.width);
        for (int rowi = 0; rowi < Rlt.height; rowi++)
        {
            Rlt.pmtx[rowi].push_back(0);
        }
        Rlt.isempty = 0;
        return Rlt;
    }
    else
    {
        Rlt.setHeight(mtxTemp.width);
        Rlt.setWidth(dimKel);
        Rlt.pmtx = new Row[Rlt.height];
        Rlt.isempty = 0;
    }

    // set value based on column properties, pivot col: true; none pivot col: false
    bool * isPivotColumn = new bool [mtxTemp.width];
    for (int i = 0; i < mtxTemp.width; i++) 
    {
        *(isPivotColumn+i) = false;
    }
    int m = 0, n = 0;
    for (; m < mtxTemp.height; m++)
    {
        for (; n < mtxTemp.width ; n++)
        {
            if (mtxTemp.pmtx[m][n] > 1+1e-5 || mtxTemp.pmtx[m][n] < 1-1e-5)
            {
                continue;
            }
            *(isPivotColumn + n) = true;
            break;
        }
    }

    for (int coli = 0; coli < mtxTemp.width; coli++)
    {
        while (isPivotColumn[coli] && coli < mtxTemp.width)
        {
            coli++;
        }
        if (coli >= mtxTemp.width)
        {
            break;
        }
        //"coli" is the none pivot column in mtxTemp:
        int rowi = findptv(coli, mtxTemp.height, mtxTemp.pmtx);
        //none pivot's coordinate in mtxTemp is (rowi, coli):
        for (int k = 0; k < Rlt.height; k++)
        {
            Rlt.pmtx[k].push_back(0);
        }
        Rlt.pmtx[coli].back() = 1;
        // to find the pivot (rowi, colj):
        for (; rowi != -1; rowi = findptv(coli, mtxTemp.height, mtxTemp.pmtx, rowi+1))
        {
            for (int colj = 0; colj < coli; colj++)
            {   
                if (mtxTemp.pmtx[rowi][colj] < 1+1e-5 && mtxTemp.pmtx[rowi][colj] > 1-1e-5)
                {
                    Rlt.pmtx[colj].back() = -mtxTemp.pmtx[rowi][coli];               
                }
            }
        }
    }
    delete [] isPivotColumn;
    return Rlt;
}
Vec matrix::specialSolution(Vec b) const
{
    if (isempty)
        throw Error<2>();
    if (b.size() != this->height)
        throw Error<3>();
    matrix mtxb(this->height, 1);
    for (int rowi = 0; rowi < this->height; rowi++)
    {
        mtxb.pmtx[rowi].push_back(b[rowi]);
    }
    mtxb.isempty = 0;
    matrix A(*this);
    int pivots = A.eliminate(&mtxb);
    bool noSolution = false;
    for (int rowi = pivots; rowi < A.height; rowi++)
    {
        if (mtxb.pmtx[rowi][0] < -1e-5 || mtxb.pmtx[rowi][0] > 1e-5)
        {
            noSolution = true;
        }
    }
    if (noSolution)
    {
        return Vec();
    }

    //solvable:
    Vec Solution;
    int m = 0, n = 0;
    for (; m < A.height; m++)
    {
        for (; n < A.width ; n++)
        {
            if (A.pmtx[m][n] < 1-1e-5 || A.pmtx[m][n] > 1+1e-5)
            {
                Solution.push_back(0);
                continue;
            }
            Solution.push_back(mtxb.pmtx[m][0]);
            n++;
            break;
        }
        if (m == A.height - 1)
        {
            while (n++ < A.width)
                Solution.push_back(0);
            break;
        }
    }
    return Solution;
}

matrix matrix::orthogonalize(void) const
{
    // Gram-Schmidt transformation:
    if (isempty)
        throw Error<2>();
    else if (this->width != this->height)
        throw Error<3>("The matrix is not square!\n");
    int N = this->width;
    matrix rlt(*this);
    for (int i = 0; i < N; i++)
    {
        rlt.pmtx[i] = pmtx[i];
        for (int j = 0; j < i; j++)
        {
            double norm = rlt.pmtx[j]*rlt.pmtx[j];
            if (fabs(norm) < 1e-5)
                continue;
            rlt.pmtx[i] = rlt.pmtx[i] + -(rlt.pmtx[j]*pmtx[i])/norm*rlt.pmtx[j];
        }
    }
    return rlt;
}

bool matrix::operator == (const matrix& B) const
{
    if (this->isempty || B.isempty)
        throw Error<2>("Attempt to compare uninitialzed matrices\n");
    if (width != B.width || height != B.height)
        return false;
    else
    {
        int M = height;
        for (int i = 0; i < M; i++)
            if (pmtx[i] != B.pmtx[i])   return false;
    }
    return true;
}

bool matrix::symmetric(void) const
{
    if (isempty)
        throw Error<2>();
    if (width != height)
        return false;
    if (*this == this->transpose())
        return true;
    else
        return false;
}

double matrix::trace(void) const
{
    if (isempty)
        throw Error<2>();
    if (width != height)
        throw Error<4>("The matrix is not square!\n");
    double tr = 0.0;
    for (int i = 0; i < width; i++)
        tr += pmtx[i][i];
    return tr;
}

// matrix matrix::congruentTransform(matrix* transMat)
// {
//     if (isempty)
//         throw Error
// }