/**
 * @file TestEvaluator.hpp
 * @author Gabriel Ferreira (gferreiragabriel@abc.gob.ar)
 * @brief This file is part of CPPSET tools. It is a small utility to run "homemade" unit tests.
 * @version 0.1
 * @date 2024-04-26
 * 
 * @copyright Copyright (c) 2022 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */
#ifndef TESTEVALUATOR_H
#define TESTEVALUATOR_H

#pragma once

#include <iostream>

class TestEvaluator
{
    public:
        TestEvaluator() : tests_passed(0), tests_failed(0) {}
        ~TestEvaluator(){}
       
        void evaluate(const std::string& test_name, bool (test_expr)) 
        {
            if (test_expr) 
            {
                std::cout << "\t\x1B[32m[PASSED] Test: "+test_name+"\x1B[0m " << std::endl;
                tests_passed++;
            } else 
            {
                std::cout << "\t\x1B[31m[FAILED] Test: "+test_name+"\x1B[0m" << std::endl;
                tests_failed++;
            }
        }

        void title(const std::string& title)
        {
            std::cout << std::endl;
            std::cout << "\x1B[36m["+title+"]\x1B[0m" << std::endl;
        }

        void summary() 
        {
            std::cout << "\n\x1B[33mTests Summary:\x1B[0m" << std::endl;
            std::cout << "  \x1B[35mTotal tests run: \x1B[0m" << (tests_passed + tests_failed) << std::endl;
            std::cout << "  \x1B[32mTests passed:    \x1B[0m" << tests_passed << std::endl;
            std::cout << "  \x1B[31mTests failed:    \x1B[0m" << tests_failed << std::endl;
            std::cout << std::endl;
        }

    private:
        int tests_passed;
        int tests_failed;
};

/**
 * @brief Se puede usar esta 
 * función macro para realizar 
 * una simple aserción (ASSERT) y mostrar el resultado. * 
 */
// #define ASSERT(test_expr) te->evaluate(#test_expr, test_expr)

#endif
