
/* File: swig_module_libclient.i */
 #ifdef SWIG
%module libclient
%{
#include "gp/proto/gp_client_data.h"
#include "gp/proto/gp_client.h"
%}

 /* Parse the header file to generate wrappers */
%include "gp/proto/gp_client_data.h"
%include "gp/proto/gp_client.h"
#endif
