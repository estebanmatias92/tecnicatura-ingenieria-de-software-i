#include <iostream>
#include <string>
#include <memory>
#include "TestEvaluator.hpp"

using namespace std;

//Función macro para realizar una simple aserción (ASSERT) y mostrar el resultado
#define ASSERT(test_expr) te->evaluate(#test_expr, test_expr)

int main() 
{
    /**
     * @brief make_unique construye un objeto de tipo TestEvaluator
     * lo envuelve en un std::unique_ptr.
     * std::unique_ptr es un puntero inteligente
     * que posee y administra otro objeto
     * a través de un puntero y elimina ese objeto
     * cuando unique_ptr sale del alcance.
     */
    auto te = make_unique<TestEvaluator>();

    //Imprimir el título del Caso de Pruebas
    te->title("Math operation tests:");

    // Realizar pruebas unitarias
    te->evaluate("Evaluando que 2 + 2 es cuatro", 2 + 2 == 4);

    // Realizar pruebas usando la función macro
    ASSERT(3 * 7 == 21);
    ASSERT(3 * 7 == 24);// Esta prueba fallará
    ASSERT(5 - 2 == 4); // Esta prueba fallará

    // Imprimir resumen de pruebas
    te->summary();

    return 0;
}
