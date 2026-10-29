#ifndef CAR_H
#define CAR_H

#pragma once

#include <iostream>

using namespace std;

class Car
{
    public:
        Car();
        ~Car();
        void setGas(int gas);
        int getGas();
        void setLightStatus(bool status);
        bool getLightStatus();
        void turnOnLights();
        void turnOffLights();

    private:
        int _gas;
        bool _lightStatus;
};



#endif