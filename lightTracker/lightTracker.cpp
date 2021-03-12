/*******************************************************************
 * @file            : lightTracker.cpp
 * @brief           : to track the parallel incident light
 * @author          : Yang Ming
 * @email           : 1308592371@qq.com
 * @version         : 1.0.0
 * @license         : GNU General Public License (GPL)
 *******************************************************************/
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

double h = 10;
double u, n, n_, l, r_left, r_right;
double l_, u_;

void output(double l_, double u_)
{
    printf("l\': %.5f\tu\': %.5f\n", l_, u_);
    return ;
}

void swap(double& a, double& b)
{
    double temp = n_;
    n_ = n;
    n = temp;
}

void calculate(double r)
{
    double coeff = n_ * r / ((n_ - n) * l + n * r);
    l_ = coeff * l;
    u_ = u / coeff;
}

void transmit(double d)
{
    l = l_ - d;
    u = u_;
}

int main()
{
    ifstream fin;

    /* To change this relative path to absolute path can also work */
    fin.open("./params.txt");

    if (!fin)
    {
        cerr << "error: unable to load params.txt\nPlease check if it exists.\n";
        system("pause");
        exit(1);
    }
    
    printf("how many lens: \n");
    unsigned int T;
    cin >> T;
    for (int i = 0; i < T; i++)
    {
        n = 1;
        
        if (i == 0)
        {
            double d;
            fin >> r_left >> r_right >> d >> n_;
            
            l_ = n_ * r_left / (n_ - n);
            u_ = h / l_;
            
            output(l_, u_);

            transmit(d);

            swap(n, n_);

            calculate(r_right);

            output(l_, u_);
        }

        else 
        {
            double d1, d2;
            fin >> r_left >> d1;
            fin >> r_right >> d2 >> n_;

            transmit(d1);

            calculate(r_left);

            output(l_, u_);

            transmit(d2);

            swap(n, n_);

            calculate(r_right);

            output(l_, u_);
        }
        
    }
    
    fin.close();

    printf("----------program ends----------\n");

    system("pause");
    return 0;
}