#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <chrono>
#include <iostream>
#include <vector>
#include <omp.h>
#include <thread>

using namespace std;
using namespace std::chrono;

struct Point {
    double x, y;
};



double calculateArea(const std::vector<Point>& polygon, int numberOfThreads, int num_points) {
    double area = 0.0;

    #pragma omp parallel for reduction(+:area) num_threads(numberOfThreads)
    for (int i = 0; i < num_points; i++) {
        int j = (i + 1) % num_points;
        area += (polygon[i].x + polygon[j].x) * (polygon[i].y - polygon[j].y);
    }

    return abs(area) / 2.0;
}


int main() {
    int num_points;

    cout<<"Input number of points to generate: " << endl;
    cin>>num_points;

    vector<Point> polygon(num_points);

    const int numberOfThreads = thread::hardware_concurrency();
    cout << "Detected " << numberOfThreads << " threads in CPU." << endl;

    for (int i=0; i<num_points; i++)
    {
        polygon[i].x = 300 + i * cos(2 * M_PI * i / num_points);
        polygon[i].y = 300 + i * sin(2 * M_PI * i / num_points);;
    }

    freopen("output.txt", "w", stdout);
    for (int i=1; i<=numberOfThreads; i++)
    {
        auto start = high_resolution_clock::now();
        double area = calculateArea(polygon, i, num_points);
        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time to execute " << num_points << " points with " << i << " threads" << " in microseconds "  << duration.count() << endl;
        cout << "Area: " << area << endl;
    }

    return 0;
}
