#! /bin/sh
xxd -i payload/payload >> includes/durex.h
xxd -i service/init_d >> includes/durex.h
xxd -i service/system_d >> includes/durex.h
echo "#endif" >> includes/durex.h