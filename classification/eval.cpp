#include <fstream>
#include <iostream>
#include <cmath>
using namespace std;

int main()
{
    cout << "start of evaluation" << endl;

    const int ArrSize = 60;

    float po2avg[ArrSize];
    ifstream inputFile;
    inputFile.open("modelPO2avg.txt");
    int n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> po2avg[i];
    }
    inputFile.close();

    float po2var[ArrSize];
    inputFile.open("modelPO2var.txt");
    n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> po2var[i];
    }
    inputFile.close();

    float heartavg[ArrSize];
    inputFile.open("modelHeartavg.txt");
    n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> heartavg[i];
    }
    inputFile.close();

        
    float heartvar[ArrSize];
    inputFile.open("modelHeartvar.txt");
    n = 0;
    for(int i = 0; i <61; i++)
    {
        inputFile >> heartvar[i];
    }
    inputFile.close();

    // enter intercepts for Heart Data
    float a0 = .5;
    float a1 = -.01235;
    float a2 = .418;

    // enter intercepts for Po2 Data
    float b0 = .5;
    float b1 = -.0125;
    float b2 = 1.04825;

    int flag = 0;
    for(int i = 0; i <61; i++)
    {
        double predictionHeart = a0 + (a1 * heartavg[i]) + (a2 * heartvar[i]);
        double predictionPO2 = b0 + (b1 * po2avg[i]) + (b2 * po2var[i]);
        if (predictionHeart && predictionPO2 > .5)
        {
            flag = 1;
        }
    }

    string sql = "INSERT INTO (TABLE_NAME) (column1, column2..) VALUES( ";
    if (flag == 1)
    {
        sql.append("1 );");
    }
    if (flag == 0)
    {
        sql.append("0 );");
    }

    ofstream outputFile;
    outputFile.open("database.txt");
    outputFile << flag << std::endl;
    outputFile << sql << std::endl;
    outputFile.close();




    return 0;
}