#pragma once

#include "pch.h"

#include <string>
#include <system_error>
#include <comdef.h>

std::string PrintErrorMessageFromHRESULT(HRESULT hr)
{
   _com_error err(hr);
   return std::string(_bstr_t(err.ErrorMessage()));
}

bool print_error_if_failed(HRESULT hr) 
{ 
   if (FAILED(hr)) 
   { 
       _com_error err(hr);
       LPCTSTR errMsg = err.ErrorMessage();
       printf("Error: %ls\n", errMsg);
       return true;
   }
   return false;
}