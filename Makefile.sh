#!/bin/bash
#rm *.o
#gcc -I/usr/include/libxml2 rainbow5_to_hdf5.c write_hdf5.c le_cabecalho_xml.c  -c
#g++ le_dados_blob.cpp -I/usr/include/qt4 -I/usr/include/libxml2 -c
g++ -I/usr/include/libxml2  -I/usr/include/qt4  rainbow5_to_hdf5.c  write_hdf5.c le_cabecalho_xml.c le_dados_blob.cpp -lhdf5  -lxml2 -lQtCore -g -w -o rainbow5_to_hdf5
ulimit -s unlimited
