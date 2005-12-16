// g++ --shared dll.cpp -o dll.dll

#define EXPORT_IMPORT __declspec(dllexport)
#include "app.hpp"
#include "dll.hpp"

#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>

void EXPORT_IMPORT report_exception_in_dll()
{
    std::cout << "  Rethrowing in dll" << std::endl;
    try {throw;}
    catch(std::exception const& e)
        {std::cout << "  Caught in dll: " << e.what() << std::endl;}
    catch(...)
        {std::cout << "  Caught in dll: unknown exception " << std::endl;}
}

void EXPORT_IMPORT throw_from_dll()
{
    throw std::runtime_error("Exception from dll.");
}

void (*throw_from_app_via_callback)();
void (*report_exception_in_app_via_callback)();

void EXPORT_IMPORT set_callbacks(void (*f)(), void (*g)())
{
    report_exception_in_app_via_callback = f;
    throw_from_app_via_callback = g;
}

//////////////////////////////////////////////
// In this module, any use of               //
//   report_exception_in_app_via_callback() //
// causes an abend.                         //
//////////////////////////////////////////////

// FAILS: Thrown from app, reported in app.
void EXPORT_IMPORT test_d_aa()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_app_via_callback();}
    catch(...)
        {
        std::cout << "  This will fail..." << std::endl;
        report_exception_in_app_via_callback();
        std::cout << "SURPRISE: this line was reached." << std::endl;
        }
}

// FAILS: Thrown from dll, reported in app.
void EXPORT_IMPORT test_d_da()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_dll();}
    catch(...)
        {
        std::cout << "  This will fail..." << std::endl;
        report_exception_in_app_via_callback();
        std::cout << "SURPRISE: this line was reached." << std::endl;
        }
}

// OK: Thrown from app, reported in dll.
void EXPORT_IMPORT test_d_ad()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_app_via_callback();}
    catch(...) {report_exception_in_dll();}
}

// OK: Thrown from dll, reported in dll.
void EXPORT_IMPORT test_d_dd()
{
    std::cout << __FUNCTION__ << std::endl;
    try {throw_from_dll();}
    catch(...) {report_exception_in_dll();}
}

