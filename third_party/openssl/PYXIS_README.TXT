Last updated by: Mark Young
Date: 2015-08-25

OpenSSL version 0.9.8h

The OpenSSL Project is a collaborative effort to develop a robust,
commercial-grade, full-featured, and Open Source toolkit
implementing the Secure Sockets Layer (SSL v2/v3) and
Transport Layer Security (TLS v1) protocols as well as a
full-strength general purpose cryptography library. 

- From 
	http://www.openssl.org/

- License
	See LICENCE file.

- Patents
	See README file.


The following changes have been made to allow libcurl to build with openssl:

1. In ossl_typ.h, comment out the line containing typedef struct ocsp_response_st OCSP_RESPONSE;
2. In ocsp.h after the struct ocsp_response_st definition, add typedef struct ocsp_response_st OCSP_RESPONSE;

We are moving the typedef forward declaration from ossl_typ.h to ocsp.h to avoid conflicts.