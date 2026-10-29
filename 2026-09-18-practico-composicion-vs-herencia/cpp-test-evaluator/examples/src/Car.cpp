#include <Car.hpp>

Car::Car()
{
    // _gas = 0;
    setGas(0);
    // _lightStatus = false;
    setLightStatus(false);
}

Car::~Car()
{

}

void Car::setGas(int gas) 
{
    _gas = gas + 3;
}

int Car::getGas() 
{
    return _gas;
}

void Car::setLightStatus(bool status)
{
    _lightStatus = status;
}

bool Car::getLightStatus()
{
    return _lightStatus;
}

void Car::turnOnLights()
{
    _lightStatus = true;
}

void Car::turnOffLights()
{
    _lightStatus = false;
}