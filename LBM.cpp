#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>
#include <omp.h>  


struct DataPoint {
    double x, y, z;
    double WSS, OSI, TAWSS;
};


double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) + std::pow(z2 - z1, 2));
}


double calculateMeanNeighboringValues(const std::vector<DataPoint>& data, const DataPoint& point, double searchRadius, char attribute) {
    double sum = 0.0;
    int count = 0;

    for (const auto& neighbor : data) {
        double distance = calculateDistance(point.x, point.y, point.z, neighbor.x, neighbor.y, neighbor.z);
        if (distance > 0 && distance <= searchRadius) {
            switch (attribute) {
                case 'W':
                    sum += neighbor.WSS;
                    break;
                case 'O':
                    sum += neighbor.OSI;
                    break;
                case 'T':
                    sum += neighbor.TAWSS;
                    break;
            }
            count++;
        }
    }

    if (count > 0) {
        return sum / count;
    } else {
        switch (attribute) {
            case 'W':
                return point.WSS;
            case 'O':
                return point.OSI;
            case 'T':
                return point.TAWSS;
        }
    }

    return 0.0;
}


void smoothData(std::vector<DataPoint>& data, double neighborSearchRadius) {
    std::vector<DataPoint> modifiedData = data;


    #pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i) {
        const auto& point = data[i];

        double meanWSS = calculateMeanNeighboringValues(data, point, neighborSearchRadius, 'W');
        double meanOSI = calculateMeanNeighboringValues(data, point, neighborSearchRadius, 'O');
        double meanTAWSS = calculateMeanNeighboringValues(data, point, neighborSearchRadius, 'T');

        modifiedData[i].WSS = meanWSS;
        modifiedData[i].OSI = meanOSI;
        modifiedData[i].TAWSS = meanTAWSS;
    }

    data = modifiedData;
}

void printElapsedTime(std::chrono::steady_clock::time_point startTime) {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);

    auto hours = std::chrono::duration_cast<std::chrono::hours>(elapsedTime);
    elapsedTime -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsedTime);
    elapsedTime -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime);
    elapsedTime -= seconds;
    auto milliseconds = elapsedTime.count();

    std::cout << "\r" 
        << "Runtime: "
        << std::setw(2) << std::setfill('0') << hours.count() << ":"
        << std::setw(2) << std::setfill('0') << minutes.count() << ":"
        << std::setw(2) << std::setfill('0') << seconds.count() << ":"
        << std::setw(3) << std::setfill('0') << milliseconds
        << std::flush;  
}

int main() {
    std::ifstream inputFile("FlowHemodynamics.csv");
    std::ofstream outputFile("AD_smoothed.csv");
    std::string line;
    std::vector<DataPoint> data;


    std::getline(inputFile, line);
    outputFile << line << std::endl; 

    auto startTime = std::chrono::steady_clock::now();

    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string temp;
        DataPoint point;


        std::getline(ss, temp, ',');
        point.x = std::stod(temp);

        std::getline(ss, temp, ',');
        point.y = std::stod(temp);

        std::getline(ss, temp, ',');
        point.z = std::stod(temp);


        std::getline(ss, temp, ',');
        point.WSS = std::stod(temp);

        std::getline(ss, temp, ',');
        point.OSI = std::stod(temp);

        std::getline(ss, temp, ',');
        point.TAWSS = std::stod(temp);

        data.push_back(point);

        printElapsedTime(startTime);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));  
    }

    double neighborSearchRadius = 0.5; 
    smoothData(data, neighborSearchRadius);

    for (const auto& point : data) {
       outputFile << point.x << "," << point.y << "," << point.z << ","
                  << point.WSS << "," << point.OSI << "," << point.TAWSS << std::endl;
    }

    inputFile.close();
    outputFile.close();

    std::cout << "Data has been smoothed and saved to 'smoothed_data.csv'." << std::endl;

    return 0;
}


