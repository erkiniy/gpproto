
/* File: swig_module_libclient.i */
 #ifdef SWIG
%module libclient

%include "carrays.i"
%array_functions(int, intArray);
%array_functions(long, longArray);
%array_functions(unsigned int, unsigendIntArray);

%{
#include "gp/proto/gp_client_data.h"
#include "gp/proto/gp_client.h"
%}

 /* Parse the header file to generate wrappers */
%include "gp/proto/gp_client_data.h"
%include "gp/proto/gp_client.h"

#endif
