// g++ app.cpp dll.dll

#define EXPORT_IMPORT __declspec(dllimport)
#include "app.hpp"
#include "dll.hpp"

#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>

void report_exception_in_app()
{
    std::cout << "  Rethrowing in app" << std::endl;
    try {throw;}
    catch(std::exception const& e)
        {std::cout << "  Caught in app: " << e.what() << std::endl;}
    catch(...)
        {std::cout << "  Caught in app: unknown exception " << std::endl;}
}

void throw_from_app()
{
    throw std::runtime_error("Exception from app.");
}

/////////////////////////////////
// In this module, any use of  //
//   report_exception_in_dll() //
// causes an abend.            //
/////////////////////////////////

// OK: Thrown from app, reported in app.
void test_a_aa()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_app();}
    catch(...) {report_exception_in_app();}
}

// OK: Thrown from dll, reported in app.
void test_a_da()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_dll();}
    catch(...) {report_exception_in_app();}
}

// FAILS: Thrown from app, reported in dll.
void test_a_ad()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_app();}
    catch(...)
        {
        std::cout << "  This will fail..." << std::endl;
        report_exception_in_dll();
        std::cout << "SURPRISE: this line was reached." << std::endl;
        }
}

// FAILS: Thrown from dll, reported in dll.
void test_a_dd()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_dll();}
    catch(...)
        {
        std::cout << "  This will fail..." << std::endl;
        report_exception_in_dll();
        std::cout << "SURPRISE: this line was reached." << std::endl;
        }
}

// An app mustn't invoke a  dll function that rethrows the current exception.
// An dll mustn't invoke an app function that rethrows the current exception.

int main()
{
    set_callbacks(report_exception_in_app, throw_from_app);

    test_a_aa();
    test_a_da();
//    test_a_ad(); // FAILS
//    test_a_dd(); // FAILS

//    test_d_aa(); // FAILS
//    test_d_da(); // FAILS
    test_d_ad();
    test_d_dd();
}

