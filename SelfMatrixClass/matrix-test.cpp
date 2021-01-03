#include <iostream>
#include "matrix.hpp"

using namespace std;

int main()
{
    // TEST:
    matrix m1(3,2); 
    matrix m2(3,1);
    cin >> m1 >> m2;
    m1.widen(m2);
    printf("height: %d width: %d\n", m1.Height(), m1.Width());
    cout << m1;

    // matrix m1(2, 3);
    // cin >> m1;
    // Vec b1(2);
    // cin >> b1[0] >> b1[1];
    // Vec x = m1.specialSolution(b1);
    // if (x.empty())
    //     cout << "no solution\n";
    // else
    //     for (const double& num: x)
    //         cout << num << ' ';

    // cout << "ker(m1) is:\n" << m1.kernel();
    // cout <<"m1 is invertible? :" << m1.invertible()<<endl;
    // cout << "det(m2) is "<<m2.determinant()<<endl;
    // m2.eliminate();
    // cout << m2 << endl;
    // system("pause");
    // matrix m3(3,2);
    // cin >> m3;
    // m3.eliminate();
    // cout << "after eliminated, m3 is" << m3;
    // // cout << "t_m3 is";
    // // cout << m3.transpose();
    system("pause"); 
    return 0; 
}