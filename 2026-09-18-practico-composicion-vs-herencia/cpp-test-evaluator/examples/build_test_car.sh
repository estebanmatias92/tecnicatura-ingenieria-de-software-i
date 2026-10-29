#!/bin/bash
##Borro el directorio build si ya existiera
if [ -d "./build" ];
	then rm -r ./build;
fi

##Creo el directorio build
mkdir ./build

## Compilo código objeto
g++ -Wall -std=c++14 -c ./src/Car.cpp -I./include/ 
g++ -Wall -std=c++14 -c test_car.cpp -I./include/ 

# ## Compilo el Binario
g++ -Wall -std=c++14 Car.o test_car.o -o ./build/test_car.bin 

## Limpio los códigos objeto
rm ./*.o

# ## Permisos de Ejecución para el binario
chmod +x ./build/test_car.bin

#ejecutar:
./build/test_car.bin