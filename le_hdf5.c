
#include <hdf5.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
   {
   char date_time[255];
   hid_t file, group, vol, attr, dataspace, memspace, how,
      header, elev0, elev1, azim0, azim1, what, memtype, where,
      time;
   
   int status;
   file = H5Fopen("h5ex_teste.h5", H5F_ACC_RDONLY, H5P_DEFAULT);
   what = H5Gopen(file, "/what", H5P_DEFAULT);
   attr = H5Aopen(what, "date", H5P_DEFAULT);
   memtype = H5Tcopy(H5T_C_S1);
   H5Tset_size(memtype, 255);
   status = H5Aread(attr, memtype, &date_time);
   H5Aclose(attr);
   printf("%s %d\n", date_time, strlen(date_time));
   
   return 0;
   
   }

