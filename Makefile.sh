#!/bin/bash
g++ -I/usr/include/libxml2  -I/usr/include/qt4 -I/usr/include/hdf5/serial rainbow5_to_hdf5.c  write_hdf5.c le_cabecalho_xml.c le_dados_blob.cpp -lhdf5_serial  -lxml2 -lQtCore -g -w -o rainbow5_to_hdf5
ulimit -s unlimited
