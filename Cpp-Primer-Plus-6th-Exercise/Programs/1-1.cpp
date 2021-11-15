#include<iostream>

const double Factor = 220.0;

int main(){
    double dist = 0.0;
    std::cout<<"Enter a distance (long): ";
    std::cin>>dist;
    std::cout<<dist<<" long = "<<dist * Factor<<" yd\n";
    return 0;
}