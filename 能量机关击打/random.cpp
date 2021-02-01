#include "random.hpp"

/* sample from normal distribution */
const double random_normal(const double mean, const double variance)
{
    ftime(&time_buffer);
    generator.seed(time_buffer.millitm);
    std::normal_distribution<double> dist(mean, variance);
    return dist(generator);
}

/* sample from uniform distribution */
const double random_uniform(const double mean, const double range)
{
    ftime(&time_buffer);
    srand(time_buffer.millitm);
    return (mean - range/2) + (rand() % 100) / 100 * range;
}


/* #include <iostream>
using namespace std;

int main()
{
    for (int i = 0; i < 1000; i++)
        cout << random_normal(0, 0.2) << endl;
    return 0;
} */