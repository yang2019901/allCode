#include <string>
using namespace std;
string welcome = "Welcome to matrix.exe! \n\
This program can help you do simple but useful real-number matrices manipulation  \n\
\n\
Abstract: \n\
0. create a matrx (with or without size) \n\
1. initialize a matrix (by using command \"enter\")\n\
2. have an overview of all matrices \n\
3. delete a certain matrix or all matrices \n\
4. check and set a certain matrix's info \n\
\t .0 check height & width \n\
\t .1 check whether initialized \n\
\t .2 set height & width \n\
\t .3 calculate rank \n\
\t .4 calculate kernel(i.e. nullspace) \n\
5. do alrithmetic operations \n\
\t .0 matrices addition \n\
\t .1 matrices multiplication \n\
\t .2 number-matrix multiplication \n\
\t .3 assignment from one matrix to another \n\
\t .4 Gauss-elimination \n\
\t .5 transpose \n\
\t .6 invertible \n\
\t .7 inverse \n\
\t .8 determinant \n\
\t .9 adjoint \n\
\t .10 solve system of linear equations \n\
6. see last matrix-calculation result in \"ANS\" matrix \n\
7. exit the program \n\
\t\t\t\t\t\t\t\t responsible developer: Yang Ming \n\
\t\t\t\t\t\t\t\t email-address: 1308592371@qq.com \n\
\n\
NOTE: enter help or -h to see further info(usage)\n\
";

string help = "All available commands:\n\
expanded     abbreviated (same effect)   usage(m1,m2 are two example)\n\
{exit,               -e},                exit\n\
{help,               -h},                help\n\
{make_matrix,     mkmtx},                (without size:) mkmtx m1\n\
                                         (with size:) mkmtx m1 2 2\n\
{delete_matrix,  delmtx},                delmtx m1\n\
{all_matrices,   allmtx},                allmtx\n\
{clear_matrices, clrmtx},                clrmtx\n\
{setheight,          sh},                sh m1 2\n\
{setwidth,           sw},                sw m1 2\n\
{size,                s},                s m1\n\
{isempty,         isept},                isempty m1\n\
{enter,             etr},                etr m1\n\
{show,              shw},                shw m1\n\
{plus,                +},                plus m1 m2\n\
{multiply,            *},                multiply m1 m2\n\
{copy,                c},                copy m1 m2 (assign m1 from m2)\n\
{transpose,           t},                t m1\n\
{inverse,           inv},                inv m1\n\
{invertible,      isinv},                isinv m1\n\
{adjoint,           adj},                adj m1\n\
{eliminate,         elm},                elm m1\n\
{determinant,       det},                det m1\n\
{rank,                r},                rank m1\n\
{kernel,            ker},                ker m1\n\
{solve,             slv},                solve m1 (note: proceed with following guidance)\n\
";
