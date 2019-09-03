
/* File: swig_module_libclient.i */
 #ifdef SWIG
%module libclient

%include "carrays.i"
%include "arrays_java.i"

%apply signed char* { const unsigned char *value };

struct gp_data {
    size_t length;
    const unsigned char *value;
};

%array_functions(int, intArray);
%array_functions(long, longArray);
%array_functions(unsigned int, unsigendIntArray);
%array_functions(long long, longLongArray);
%array_class(unsigned char, UnsignedCharArray);

%{
#include "gp/proto/gp_client_data.h"
#include "gp/proto/gp_client.h"
%}

 /* Parse the header file to generate wrappers */
%include "gp/proto/gp_client_data.h"
%include "gp/proto/gp_client.h"

#endif
