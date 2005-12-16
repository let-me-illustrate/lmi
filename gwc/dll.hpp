#ifndef dll_hpp
#define dll_hpp
void EXPORT_IMPORT throw_from_dll();
void EXPORT_IMPORT report_exception_in_dll();
void EXPORT_IMPORT set_callbacks(void (*)(), void (*)());
void EXPORT_IMPORT test_d_aa();
void EXPORT_IMPORT test_d_da();
void EXPORT_IMPORT test_d_ad();
void EXPORT_IMPORT test_d_dd();
#endif // dll.hpp
