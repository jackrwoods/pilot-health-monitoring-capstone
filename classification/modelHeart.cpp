#include <fstream>
#include <iostream>
#include <cmath>
using namespace std;

int main()
{
    cout << "start of model training" << endl;
    const int ArrSize = 60;
    float avg[ArrSize];
    ifstream inputFile;
    inputFile.open("modelHeartavg.txt");
    int n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> avg[i];
    }
    inputFile.close();

    float actual[ArrSize];
    inputFile.open("actual.txt");
    n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> actual[i];
    }
    inputFile.close();
    
    float var[ArrSize];
    inputFile.open("modelHeartvar.txt");
    n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> var[i];
    }
    inputFile.close();

    // cout << var[20] <<endl;
    /*for (int i = 0; i < 61; i++)
    {
        cout << heartdata[i] << " ";    
    }
    */

    float e = 2.718;
    float a0 = 0; 
    float a1 = 0;
    float a2 = 0;
    float a = 0.1;
    float error;
    float errorRepo[ArrSize];
    float a0Repo[ArrSize];
    float a1Repo[ArrSize];
    float a2Repo[ArrSize];

    for (int b = 0; b <60; b++)
    {
        float x = -(a2 * var[b] + a1 * avg[b] + a0);
        float prediction = 1/(1+pow(e , x));
        error = actual[b] - prediction;
        a0 = a0 - ((1-prediction) * 1.0 * prediction *error * a);
        a0Repo[b] = a0;
        a1 = a1 + ((1-prediction) * avg[b] * prediction *error * a);
        a1Repo[b] = a1;
        a2 = a2 + ((1-prediction) * var[b] * prediction *error * a);
        a2Repo[b] = a2;
        errorRepo[b] = error;
    }
    float errorCheck;
    int errorNum;
    int k = 0;
    while (k < 61)
    {
        if (errorCheck < errorRepo[k])
        {
        errorCheck = errorRepo[k];
            errorNum = k;
            k = k + 1;
        
        }
        k = k + 1;
    }
    cout << "a0 = " << a0Repo[errorNum] <<endl;
    cout << "a1 = " << a1Repo[errorNum] <<endl;
    cout << "a2 = " << a2Repo[errorNum] <<endl;
    return 0;
}