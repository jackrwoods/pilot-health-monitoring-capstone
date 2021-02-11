#include <fstream>
#include <iostream>
#include <cmath>
using namespace std;
 
int main () 
{
    const int ArrSize = 60;
    int heartdata[ArrSize];
    ifstream inputFile;
    inputFile.open("heartdata.txt");
    int n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> heartdata[i];
    }
    /*
    for (int i = 0; i < 61; i++)
    {
        cout << heartdata[i] << " ";    
    }
    */
    /* calculate exponential moving average */
    int g = 1;
    float sum = float(heartdata[0]);
    while(g < 60)
    {
        /* cout << sum << endl; */ 
        sum = (1.0-.0328) * (float(heartdata[g-1])) + (.0328 * float(heartdata[g]));
        g+= 1;
    }
    cout << sum << endl;

    /* calculate variance */
    float num1 = 0.0;
    for (int i = 0; i < 61; i++)
    {
        num1 += pow(heartdata[i] - sum, 2);   
    }
    /* cout << num1 << endl; */
    float variance = num1 / 59; 
    cout << variance << endl;

    ofstream outputFile;
    outputFile.open("model.txt");
    outputFile << sum << std::endl;
    outputFile << variance << std::endl;
    outputFile.close();

    outputFile.open("realtimeeval.txt");
    outputFile << sum << std::endl;
    outputFile << variance << std::endl;
    outputFile.close();

    outputFile.open("datastore.txt");
    outputFile << sum << std::endl;
    outputFile << variance << std::endl;
    outputFile.close();

    inputFile.close();
    return 0;
}
