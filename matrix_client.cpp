#include <iostream>
#include "matrix.hpp"
#include "help.hpp"
#include <string>

using namespace std;

//!  .data()引起的奇异改名现象
//!  对标准输入流的操纵

/*to implement:
0.create & store a var;    //get a string from console and store
                            it in an array with a pointer
*/

//待添加功能：解齐次线性方程组 i.e. ker(A)

#define FAIL string::npos

typedef struct{
    string name;
    matrix *reference;
} page;
typedef struct{
    const char *long_opt;
    const char *short_opt;
} option;

vector<page> dictionary;
const double fail = double(FAIL);

string strip(string& str, char ch = ' ');
double number(const string& name);
int search(const option* opts, const string& cmd);
void ErrMtxNotFound(const string& name);
void ErrMtxUninit(const string& name);
int search(const vector<page>& dic, const string& cmd);

option options[] = {
    {"exit",             "-e"}, //0
    {"help",             "-h"}, //1

    {"make_matrix",   "mkmtx"}, //2
    {"delete_matrix","delmtx"}, //3
    {"all_matrices", "allmtx"}, //4
    {"clear_matrices","clrmtx"},//5

    {"setheight",        "sh"}, //6
    {"setwidth",         "sw"}, //7
    {"size",              "s"}, //8
    {"isempty",       "isept"}, //9
    {"enter",           "etr"}, //10
    {"show",            "shw"}, //11
    {"plus",              "+"}, //12
    {"multiply",          "*"}, //13
    {"copy",              "c"}, //14

    {"transpose",         "t"}, //15
    {"inverse",         "inv"}, //16
    {"invertible",    "isinv"}, //17
    {"adjoint",         "adj"}, //18
    {"eliminate",       "elm"}, //19
    {"determinant",     "det"}, //20

    {"rank",              "r"}, //21
    {"kernel",          "ker"}, //22
    {"solve",           "slv"}, //23
    {"orthogonalize",  "orth"}, //24
    {"trace",            "tr"}, //25
    {'\0', '\0'},
};

int main()
{
    cout << welcome;
    string cmd;
    page pANS = {"ANS", new matrix};
    dictionary.push_back(pANS);
    pANS.reference = NULL;
    while(true) 
    {
        cout <<"cmd> ";
        cin >> cmd;
        switch(search(options, cmd))
        {
            case -1:
            {
                cerr << "Error: no such command as \"" << cmd << "\"!\nenter \"-h\" or \"help\" to display all avaiable commands\n";
                cin.clear();
                cin.sync();
                continue;
            }
        
            case 0: 
            { //aka exit 
                for(page& p : dictionary)
                    delete p.reference;
                dictionary.clear();
                exit(0);
                break;
            }
            

            case 1: 
            {
                cout << help;
                break;
            }
            
            case 2: 
            {
                string matrix_name;
                cin >> matrix_name;
                int loc = search(dictionary, matrix_name);
                while(loc != -1){
                    cout <<'"'<<matrix_name<<'"'<<" exists. Please use another name.\n";
                    cin >> matrix_name;
                    loc = search(dictionary, matrix_name);
                }
                matrix* mtx;
                string str;
                getline(cin, str);
                str = strip(str);
                int l = str.size();
                if(!l)
                    mtx = new matrix;
                else
                {
                    int matrix_height=0, matrix_width=0;
                    bool badsize = false;
                    int i;
                    for (i = 0; str[i] != ' ' && str[i] != '\t'; i++)
                    {   
                        if (str[i] < '0' || str[i] > '9')
                        {
                            cin.clear();
                            cin.sync();
                            badsize = true;
                        }
                        matrix_height = 10*matrix_height + str[i] - '0';
                    }
                    
                    while (str[i] == ' ' || str[i] == '\t')
                        i++;

                    for (; str[i] != '\0'; i++)
                    {   
                        if (str[i] < '0' || str[i] > '9')
                        {
                            cin.clear();
                            cin.sync();
                            badsize = true;
                        }
                        matrix_width = 10*matrix_width + str[i] - '0';
                    }
                    
                    if (!badsize)
                    {
                        mtx = new matrix(matrix_height, matrix_width);
                    }
                    else
                    {
                        mtx = new matrix;
                    }
                }
                page p = {matrix_name, mtx};
                dictionary.push_back(p);
                break;
            }

            case 3: 
            {
                string matrix_name;
                cin >> matrix_name;
                int loc = search(dictionary, matrix_name);
                if(loc == -1) {
                    ErrMtxNotFound(matrix_name);
                    continue;
                }
                else if(loc == 0) {
                    cerr << "Access denied\n";
                    cin.clear();
                    cin.sync();
                    continue;
                }
                else {
                    delete dictionary[loc].reference;
                    dictionary.erase(loc+dictionary.begin());
                }
                break;
            }

            case 4: 
            {
                cout << '\n';
                for(const page& p: dictionary) {
                    cout << p.name << ' ';
                    cout <<p.reference->Height()<<'x'<<p.reference->Width()<<':'<<endl;
                    if(p.reference->Empty())
                        cout << " uninitialized\n";
                    else
                        cout << *p.reference << endl; 
                } 
                break;
            }

            case 5: 
            {
                //clear all matrices
                pANS.reference = dictionary[0].reference;
                int n = dictionary.size();
                for(int i = 1; i < n; i++)
                    delete dictionary[i].reference;
                dictionary.clear();
                dictionary.push_back(pANS);
                pANS.reference = NULL;
                break;
            }   

            case 6: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                else if(loc == 0) {
                    cerr << "Access denied\n";
                    cin.clear();
                    cin.sync();
                    continue;
                }
                Plus newheight;
                cin >> newheight;
                dictionary[loc].reference->setHeight(newheight);
                break;
            }

            case 7: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                else if(loc == 0) {
                    cerr << "Access denied\n";
                    cin.clear();
                    cin.sync();
                    continue;
                }
                Plus newwidth;
                cin >> newwidth;
                dictionary[loc].reference->setWidth(newwidth);
                break;
            }

            case 8: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                cout << dictionary[loc].reference->Height() <<'x' \
                <<dictionary[loc].reference->Width() << endl;
                break;
            }

            case 9: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if(dictionary[loc].reference->Empty())
                    cout << "it's  empty\n";
                else 
                    cout << "it's NOT empty\n";
                break;
            }

            case 10: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                else if(loc == 0) {
                    cerr << "Access denied\n";
                    cin.clear();
                    cin.sync();
                    continue;
                }
                cout << dictionary[loc].reference->Height() <<'x' \
                    <<dictionary[loc].reference->Width() <<':'<< endl;
                cin >> *dictionary[loc].reference;
                break;
            }

            case 11: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                cout << name << ' ';
                cout <<dictionary[loc].reference->Height()<<'x'<<dictionary[loc].reference->Width()<<':'<<endl;
                if(dictionary[loc].reference->Empty())
                    cout << " uninitialized\n";
                else
                    cout << *dictionary[loc].reference << endl; 
                break;
            }

            case 12: 
            {
                string name1, name2;
                cin >> name1 >> name2;
                int loc1 = search(dictionary, name1);
                int loc2 = search(dictionary, name2);
                if(loc1 == -1) {
                    ErrMtxNotFound(name1);
                    continue;
                }
                if(loc2 == -1) {
                    ErrMtxNotFound(name2);
                    continue;
                }
                if(dictionary[loc1].reference->Empty()) {
                    ErrMtxUninit(name1);
                    continue;
                }
                if(dictionary[loc2].reference->Empty()) {
                    ErrMtxUninit(name2);
                    continue;
                }
                matrix temp(*dictionary[loc1].reference + *dictionary[loc2].reference);
                cout << temp;
                *dictionary[0].reference = temp;
                break;
            }

            case 13: 
            {
                string name1, name2;
                cin >> name1 >> name2;
                double isnumber1 = number(name1);
                double isnumber2 = number(name2);
                #define isstring1  (isnumber1 > fail-1e+10 && isnumber1 < fail+1e+10)
                #define isstring2  (isnumber2 > fail-1e+10 && isnumber2 < fail+1e+10)
                if( isstring1 && isstring2 )
                {
                    int loc1 = search(dictionary, name1);
                    if(loc1 == -1) {
                        ErrMtxNotFound(name1);
                        continue;
                    }
                    if(dictionary[loc1].reference->Empty()) {
                        ErrMtxUninit(name1);
                        continue;
                    }
                    int loc2 = search(dictionary, name2);
                    if(loc2 == -1) {
                        ErrMtxNotFound(name2);
                        continue;
                    }
                    if(dictionary[loc2].reference->Empty()) {
                        ErrMtxUninit(name2);
                        continue;
                    }
                    matrix rlt2(*dictionary[loc2].reference);
                    matrix rlt1(*dictionary[loc1].reference);
                    // cout << (rlt1*rlt2).Width() << " "<< (rlt1*rlt2).Height()<<endl;
                    *dictionary[0].reference = (rlt1*rlt2);
                    cout << *dictionary[0].reference;
                    continue;
                }
                else if( isstring1 && !isstring2 )
                {
                    int loc1 = search(dictionary, name1);
                    if(loc1 == -1) {
                        ErrMtxNotFound(name1);
                        continue;
                    }
                    if(dictionary[loc1].reference->Empty()) {
                        ErrMtxUninit(name1);
                        continue;
                    }
                    matrix rlt1(*dictionary[loc1].reference);
                    double rlt2 = isnumber2;
                    *dictionary[0].reference = rlt1*rlt2;
                    cout << *dictionary[0].reference;
                    continue;
                }
                else if(!isstring1 && !isstring2) 
                {
                    double rlt1 = isnumber1, rlt2 = isnumber2;
                    cout << rlt1*rlt2 <<endl;
                    continue;
                }

                int loc2 = search(dictionary, name2);
                if(loc2 == -1) {
                    ErrMtxNotFound(name2);
                    continue;
                }
                if(dictionary[loc2].reference->Empty()) {
                    ErrMtxUninit(name2);
                    continue;
                }
                matrix rlt2(*dictionary[loc2].reference);
                double rlt1 = isnumber1;
                *dictionary[0].reference = rlt1*rlt2;
                cout << *dictionary[0].reference;
                break;
            }

            case 14: 
            {
                string name1, name2;
                cin >> name1 >> name2;
                int loc1 = search(dictionary, name1);
                int loc2 = search(dictionary, name2);
                if(loc1 == -1) {
                    ErrMtxNotFound(name1);
                    continue;
                }
                else if (loc1 == 0)
                {
                    cerr << "Access denied\n";
                    cin.clear();
                    cin.sync();
                    continue;
                }
                if (loc2 == -1) {
                    ErrMtxNotFound(name2);
                    continue;
                }
                if(dictionary[loc2].reference->Empty()) {
                    ErrMtxUninit(name2);
                    continue;
                }
                *dictionary[loc1].reference = *dictionary[loc2].reference;
                break;
            }

            case 15: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                matrix temp(dictionary[loc].reference->transpose());
                cout << temp;
                *dictionary[0].reference = temp;
                break;
            }

            case 16: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                matrix temp(dictionary[loc].reference->inverse());
                cout << temp;
                *dictionary[0].reference = temp;
                break;
            }

            case 17: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                if(dictionary[loc].reference->invertible()) 
                    cout << name <<" is invertible\n";
                else 
                    cout << name <<" is NOT invertible\n";
                break;
            }

            case 18: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                matrix temp(dictionary[loc].reference->adjoint());
                cout << temp;
                *dictionary[0].reference = temp;
                break;
            }

            case 19: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                cout <<"pivots: " << dictionary[loc].reference->eliminate() << endl;
                cout << *dictionary[loc].reference;
                break;
            }

            case 20:
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) 
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                cout <<"det("<<name<<"): "<< dictionary[loc].reference->determinant()<<endl;
                break;
            }

            case 21:
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1) 
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                cout << "rank(" << name << "): "<< matrix(*dictionary[loc].reference).eliminate();
                break;
            }

            case 22: 
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1)
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                else
                {
                    // ANS is the matrix of nullspace 
                    *dictionary[0].reference = dictionary[loc].reference->kernel();
                    int width = dictionary[0].reference->Width();
                    int height = dictionary[0].reference->Height();
                    for (int coli = 0; coli < width; coli++)
                    {
                        cout << "C"<<coli+1<<"*[";
                        for (int rowi = 0; rowi < height; rowi++)
                        {
                            cout << (*dictionary[0].reference)[rowi][coli];
                            if (rowi < height-1)
                            {
                                cout << ' ';
                            }
                        }
                        cout << "]";
                        if (coli < width-1)
                        {
                            cout << " + \n";
                        }
                    }
                    cout << '\n';
                }
                break;
            }

            case 23:
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if(loc == -1)
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                else
                {
                    int width = dictionary[loc].reference->Width();
                    int height = dictionary[loc].reference->Height();
                    Vec b(height);
                    cout << "Solving Ax=b...\n";
                    cout << "\"A\"(" << height << "x" << width << "): " << name << endl; 
                    cout << "Please enter vector \"b\"(" << height << "x1): ";
                    for (int i = 0; i < height; i++)
                        cin >> b[i];
                    Vec x = dictionary[loc].reference->specialSolution(b);
                    if (x.empty())
                    {
                        cout << "NO solution\n";
                    }
                    else
                    {
                        cout << "solution: \n";
                        cout << "[";
                        for (const double & num: x)
                            cout << num << ' ';
                        cout << '\b' << ']';
                        // when a special solution exists, print other general solutions 
                        matrix mtxTemp(dictionary[loc].reference->kernel());
                        width = mtxTemp.Width();
                        height = mtxTemp.Height();
                        mtxTemp.approximate();
                        if (width == 1)
                        {
                            // check whether there is a general solution:
                            bool noGeneralSolution = true;
                            for (int rowi = 0; rowi < height; rowi++)
                            {
                                if (mtxTemp[rowi][0] > 1e-5 || mtxTemp[rowi][0] < -1e-5)
                                {
                                    noGeneralSolution = false;
                                    break;
                                }    
                            }
                            if (noGeneralSolution)
                            {
                                cout << endl;
                                continue;
                            }
                        }
                        cout << " + \n";
                        for (int coli = 0; coli < width; coli++)
                        {
                            cout << "C"<<coli+1<<"*[";
                            for (int rowi = 0; rowi < height; rowi++)
                            {
                                cout << mtxTemp[rowi][coli];
                                if (rowi < height-1)
                                {
                                    cout << ' ';
                                }
                            }
                            cout << "]";
                            if (coli < width-1)
                            {
                                cout << " + \n";
                            }
                        }
                        cout << '\n';
                    }
                    
                }
                break;
            }

            case 24:
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if (loc == -1)
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                else 
                {
                    *dictionary[0].reference = dictionary[loc].reference->orthogonalize();
                    cout << *dictionary[0].reference;
                }
                break;
            }

            case 25:
            {
                string name;
                cin >> name;
                int loc = search(dictionary, name);
                if (loc == -1)
                {
                    ErrMtxNotFound(name);
                    continue;
                }
                if (dictionary[loc].reference->Empty())
                {
                    ErrMtxUninit(name);
                    continue;
                }
                printf("The trace of \"%s\": %lf\n", name.data(), dictionary[loc].reference->trace());
                break;
            }
        }
    }  
}

double number(const string& name) 
{
    int l = name.size();
    int sign = 1;
    int start = 0;
    switch(name[0]) {
        case '+':
        start = 1;
        sign = 1;
        break;
        case '-':
        sign = -1;
        start = 1;
        break;
        default:
        if(name[0]<'0'||name[0]>'9')
            return FAIL;
    }
    for(int i = 1; i < l; i++) {
        if( (name[i]<'0'||name[i]>'9')&&name[i]!='.' )
            return FAIL;
    }
    long long dec = name.find('.');
    if(dec!=string::npos && name.find('.',dec+1) != string::npos)
        return FAIL;
    else if(dec == string::npos) {
        int ratio = 1;
        double rlt = 0.0;
        for(int i = 0; i < l-start; i++){
            rlt+=(name[l-i-1]-'0')*ratio;
            ratio*=10;
        }
        return rlt*sign;
    }
    double ratio = 1;
    double rlt = 0.0;
    for(int i = 0; i < dec-start; i++) {
        rlt += (name[dec-i-1]-'0')*ratio;
        ratio *= 10;
    }
    ratio = 0.1;
    for(int i = dec+1; i < l; i++) {
        rlt += (name[i]-'0')*ratio;
        ratio /= 10;
    }
    return rlt*sign;
}

int search(const option* opts, const string& cmd)
{
    int i = 0;
    while(opts[i].long_opt != '\0' && opts[i].short_opt != '\0')
    {
        if(opts[i].long_opt == cmd || opts[i].short_opt == cmd)
            return i;
        i++;
    }
    return -1;
}

void ErrMtxNotFound(const string& name)
{
    cerr << "Error: no such matrix as " << name <<"\nenter \"all_matrices\" or \"allmtx\" to see all matrices\n";
    cin.clear();
    cin.sync();
}

void ErrMtxUninit(const string& name)
{
    cerr << name << " is empty. Please initialize it before use\n";
    cin.clear();
    cin.sync();
}

int search(const vector<page>& dic, const string& cmd) 
{
    int n = dic.size();
    for(int i = 0; i < n; i++) 
        if(dic[i].name == cmd) 
            return i; 
    return -1;
};

string strip(string& str, char ch)
{
    int l = str.size();
    string s;
    int i, j;
    for(i = 0; i < l && str[i] == ch; i++);
    for(j = l-1; j > i && str[j] == ch; j--);
    s = str.substr(i, j-i+1);
    return s;
}
