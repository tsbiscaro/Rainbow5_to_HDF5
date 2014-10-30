#include "rainbow5_to_hdf5.h"
#include <stdlib.h>


int write_hdf5(struct volume_how *v_how,
               struct volume_what *v_what, struct volume_where *v_where,
               struct scan_how s_how[], struct scan_what s_what[],
               struct data dados[])
   {

   unsigned int szip_options_mask;
   unsigned int szip_pixels_per_block;
   
   hid_t file, space, space_header, attr, how, dataspace, mtype;
   hid_t where, what, ftype, dataset, dset;
   hid_t extended, fpid, gpid;
   hid_t scan[MAX_SCANS], plist_id;
   herr_t status;
   hsize_t dims = 1, dims_data[2], dim_header[1], cdims[2];
   
   char nome_scan[MAX_STRING], nome_moment[MAX_STRING];
   
   int i, j, k, idx_moments = 0, cur_moment = 0;
   int ray = 0, bin = 0, offs = 0, offs2 = 0;
   
   unsigned char **data_U8;
   unsigned short int **data_U16;
   unsigned char temp_8[MAX_RAYS][MAX_BINS];
   unsigned short int temp_16[MAX_RAYS][MAX_BINS];

   char filename[1024];
   
/*   
   memset(&v_how, 0, sizeof(struct volume_how));
   memset(&v_what, 0, sizeof(struct volume_what));
   memset(&v_where, 0, sizeof(struct volume_where));
   memset(s_how, 0, MAX_SCANS * sizeof(struct scan_how));
   memset(s_what, 0, MAX_SCANS * sizeof(struct scan_what));
   memset(s_extended, 0, MAX_SCANS * sizeof(struct scan_how_extended));
   memset(dados, 0, MAX_SCANS * sizeof(struct data));

   v_what->sets = 12;
*/

   memset(filename, 0, sizeof(filename));
   sprintf(filename, "%s-%s.HDF5", v_how->host_name, v_how->arq_original);
   
   /*cria o space*/
   space = H5Screate_simple (1, &dims, NULL);
   /*cria o arquivo*/
   fpid = H5Pcreate (H5P_FILE_ACCESS);
   status = H5Pset_libver_bounds (fpid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
   file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fpid);
   
   /**********************escreve o HOW do volume************************/
   how = H5Gcreate(file, "/how", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

   write_attr_text(how, "host_name", v_how->host_name);
   write_attr_text(how, "sdp_name", v_how->sdp_name);
   write_attr_text(how, "software", v_how->software);
   write_attr_text(how, "sw_version", v_how->sw_version);
   write_attr_text(how, "template_name", v_how->template_name);
   write_attr_double(how, "azimuth_beam", v_how->beamwidth);
   write_attr_double(how, "elevation_beam", v_how->beamwidth);
   
   /*fim da gravacao do grupo HOW*/
   status = H5Gclose(how);

   /**********************escreve o WHERE do volume************************/
   where = H5Gcreate(file, "/where", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

   write_attr_double(where, "height", v_where->height);
   write_attr_double(where, "lat", v_where->lat);
   write_attr_double(where, "lon", v_where->lon);
   
   /*fim da gravacao do grupo WHERE*/
   status = H5Gclose(where);   

   /**********************escreve o WHAT do volume************************/
   
   what = H5Gcreate(file, "/what", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
   
   write_attr_text(what, "date", v_what->date);
   write_attr_text(what, "object", v_what->object);
   
   write_attr_uint(what, "sets", v_what->sets);
   write_attr_uint(what, "version", v_what->version);
   
   /*fim da gravacao do grupo WHAT*/
   status = H5Gclose(what);


   /*fechar o arquivo para nao dar erro de memoria*/
   status = H5Fclose(file);

   /*cria e grava os dados dos scans*/
   /**********************escreve os SCANS************************/

   for (i = 0; i < v_what->sets; i++)
      {
      memset(nome_scan, 0, sizeof(nome_scan));
      sprintf(nome_scan, "scan%d", i);

      file = H5Fopen(filename, H5F_ACC_RDWR, fpid); 
      scan[i] = H5Gcreate(file, nome_scan, H5P_DEFAULT, H5P_DEFAULT,
                          H5P_DEFAULT);

      how = H5Gcreate(scan[i], "how", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

      write_attr_uint(how, "PRF", s_how[i].PRF);
      
      attr = H5Acreate(how, "angle_sync", H5T_NATIVE_UCHAR, space, H5P_DEFAULT,
                       H5P_DEFAULT);
      status = H5Awrite(attr, H5T_NATIVE_UCHAR, &s_how[i].angle_sync);
      H5Aclose(attr);
      
      write_attr_uint(how, "bin_count", s_how[i].bin_count);
      write_attr_uint(how, "filter", s_how[i].filter);
      


      attr = H5Acreate(how, "half_resolution", H5T_NATIVE_UCHAR, space,
                       H5P_DEFAULT, H5P_DEFAULT);
      status = H5Awrite(attr, H5T_NATIVE_UCHAR, &s_how[i].half_resolution);
      H5Aclose(attr);      
      attr = H5Acreate(how, "long_pulse", H5T_NATIVE_UCHAR, space, H5P_DEFAULT,
                       H5P_DEFAULT);
      status = H5Awrite(attr, H5T_NATIVE_UCHAR, &s_how[i].long_pulse);
      H5Aclose(attr);

      write_attr_uint(how, "range_samples", s_how[i].range_samples);
      write_attr_uint(how, "ray_count", s_how[i].ray_count);
      write_attr_uint(how, "time_samples", s_how[i].time_samples);
      write_attr_uint(how, "unfolding", s_how[i].unfolding);

      write_attr_double(how, "angle_step", s_how[i].angle_step);
      
      if (0 != strcmp(v_what->object, "PELE"))
         {   
         write_attr_double(how, "azi_start", s_how[i].azi_start);
         write_attr_double(how, "azi_stop", s_how[i].azi_stop);
         write_attr_double(how, "elevation", s_how[i].elevation);
         }
      else
         {
         write_attr_double(how, "ele_start", s_how[i].azi_start);
         write_attr_double(how, "ele_stop", s_how[i].azi_stop);
         write_attr_double(how, "azimuth", s_how[i].elevation);
         }
      
         
      write_attr_double(how, "pulse_width_us", s_how[i].pulse_width_us);
      write_attr_double(how, "radar_wave_length", s_how[i].radar_wave_length);
      write_attr_double(how, "range", s_how[i].range);
      write_attr_double(how, "range_start", s_how[i].range_start);
      write_attr_double(how, "range_step", s_how[i].range_step);
      write_attr_double(how, "scan_speed", s_how[i].scan_speed);

      write_attr_text(how, "timestamp", s_how[i].timestamp);
      printf("slice %02d, timestamp: %s\n", i,s_how[i].timestamp);
      

      /*fim da gravacao do grupo HOW*/
      status = H5Gclose(how);
      
      /*cria o scan WHAT*/
      what = H5Gcreate(scan[i], "what", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      write_attr_uint(what, "descriptor_count", s_what[i].descriptor_count);
      write_attr_text(what, "product", s_what[i].product);
      write_attr_text(what, "scan_type", s_what[i].scan_type);

      /*fim da gravacao do grupo WHAT*/
      status = H5Gclose(what);
      
      /*Grava os datasets (Z, V, W, etc...)*/      
      idx_moments = 0;
      for (j = 0; j < MAX_VARS; j++)
         {
         if (0 == v_how->vars_validas[j])
            {
            continue;
            }
         dims_data[0] = s_how[i].ray_count;
         dims_data[1] = s_how[i].bin_count;         
         memset(nome_moment, 0, sizeof(nome_moment));
         sprintf(nome_moment, "moment_%d", idx_moments);

         /*parametros para compressao do dataset*/
         plist_id  = H5Pcreate (H5P_DATASET_CREATE);
         cdims[0] = dims_data[0];
         cdims[1] = dims_data[1];
         status = H5Pset_chunk (plist_id, 2, cdims);
         status = H5Pset_deflate (plist_id, 8); 
         
         if (0 == strncmp("UV8", dados[i].header[j].format, 3))
            {
            data_U8 = (unsigned char **) calloc(s_how[i].ray_count,
                                                sizeof(unsigned char *));
            
            data_U8[0] = (unsigned char *)  calloc(s_how[i].ray_count *
                                                   s_how[i].bin_count,
                                                   sizeof(unsigned char));
            for (k = 1; k < s_how[i].ray_count; k++)
               {
               data_U8[k] = data_U8[0] + k*s_how[i].bin_count;
               if (NULL == data_U8[k])
                  {
                  printf("ERRO ALOCANDO MEMORIA\n");
                  return -1;
                  }
               }
            
         switch (j)
            {
            case VAR_CZ:
               {
               memcpy(&temp_8[0][0], &dados[i].dBZ[0][0], 
                      sizeof(temp_8));
               break;
               }
            case VAR_UZ:
               {
               memcpy(&temp_8[0][0], &dados[i].dBuZ[0][0],
                      sizeof(temp_8));
               break;
               }
            case VAR_V:
               {
               memcpy(&temp_8[0][0], &dados[i].V[0][0], 
                      sizeof(temp_8));
               break;
               }
            case VAR_W:
               {
               memcpy(&temp_8[0][0], &dados[i].W[0][0], 
                      sizeof(temp_8));
               break;
               }
            case VAR_ZDR:
               {
               memcpy(&temp_8[0][0], &dados[i].ZDR[0][0], 
                      sizeof(temp_8));
               break;
               }
            case VAR_RHO:
               {
               memcpy(&temp_8[0][0], &dados[i].RhoHV[0][0], 
                      sizeof(temp_8));
               break;
               }
            default:
               {
               printf("ERRO: LENDO VARIAVEL DESCONHECIDA 8 %d\n", j);
               break;
               }
            }

            for (ray = 0; ray < s_how[i].ray_count; ray++)
               {
               for (bin = 0; bin < s_how[i].bin_count; bin++)
                  {
                  data_U8[ray][bin] = temp_8[ray][bin];
                  }
               }
            
            dataspace = H5Screate_simple (2, dims_data, NULL);
            dataset = H5Dcreate (scan[i], nome_moment, H5T_STD_U8LE, dataspace,
                                 H5P_DEFAULT, plist_id, H5P_DEFAULT);
            status = H5Dwrite (dataset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL,
                               H5P_DEFAULT, &data_U8[0][0]);
            H5Sclose(dataspace);
            if (-1 == status)
               {
               printf("ERRO GRAVANDO %d %d\n", i, j);
               return -1;
               }
            }
         else
            {
            /*dados de 16 bits*/
            data_U16 = (unsigned short int **) calloc(s_how[i].ray_count, sizeof(unsigned short int *));
            
            data_U16[0] = (unsigned short int *)  calloc(s_how[i].ray_count*s_how[i].bin_count,
                                                         sizeof(unsigned short int));
            switch (j)
               {
               case VAR_PHI:
                  {
                  memcpy(&temp_16[0][0], &dados[i].PhiDP[0][0], 
                         sizeof(temp_16));
                  break;
                  }
               case VAR_UPHI:
                  {
                  memcpy(&temp_16[0][0], &dados[i].uPhiDP[0][0], 
                         sizeof(temp_16));
                  break;
                  }
               case VAR_KDP:
                  {
                  memcpy(&temp_16[0][0], &dados[i].KDP[0][0],
                         sizeof(temp_16));
                  break;
                  }               
               default:
                  {
                  printf("ERRO: LENDO VARIAVEL DESCONHECIDA %d %d\n",i, j);
                  break;
                  }
               }
            
            for (k = 1; k < s_how[i].ray_count; k++)
               {
               data_U16[k] = data_U16[0] + k*s_how[i].bin_count;
               if (NULL == data_U16[k])
                  {
                  printf("ERRO ALOCANDO MEMORIA\n");
                  return -1;
                  }
               }
            
            
            for (ray = 0; ray < s_how[i].ray_count; ray++)
               {
               for (bin = 0; bin < s_how[i].bin_count; bin++)
                  {
                  data_U16[ray][bin] = temp_16[ray][bin];
                  }
               }
            
            dataspace = H5Screate_simple (2, dims_data, NULL);
            dataset = H5Dcreate (scan[i], nome_moment, H5T_STD_U16LE,
                                 dataspace,
                                 H5P_DEFAULT, plist_id, H5P_DEFAULT);
            status = H5Dwrite (dataset, H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                               H5P_DEFAULT, &data_U16[0][0]);
            H5Sclose(dataspace);
            }
         H5Pclose (plist_id);

         write_attr_double(dataset, "dyn_range_max",
                         dados[i].header[j].dyn_range_max);
         write_attr_double(dataset, "dyn_range_min",
                         dados[i].header[j].dyn_range_min);
         
         write_attr_text(dataset, "format", dados[i].header[j].format);
         write_attr_text(dataset, "moment", dados[i].header[j].moment);
         write_attr_text(dataset, "unit", dados[i].header[j].unit);

         /*fim da gravacao dos datasets*/
         H5Dclose(dataset);

         /*
         if (0 == strncmp("UV8", dados[i].header[j].unit, 3))
            {
            delete[] data_U8;
            }
         else
            {
            delete[] data_U16;
            }
         */
         idx_moments++;
         }

      /*grava o Ray Header para volumes e azimuth_scan (ppi)*/


      mtype = H5Tcreate (H5T_COMPOUND, sizeof (struct ray_header));

      if (0 != strcmp(v_what->object, "PELE"))
         {
         status = H5Tinsert (mtype, "azimuth_start",
                             HOFFSET (struct ray_header, azimuth_start),
                             H5T_NATIVE_DOUBLE);
         if (9999 != s_how[i].blob_stopangle_id)
            {   
            status = H5Tinsert (mtype, "azimuth_stop",
                                HOFFSET (struct ray_header, azimuth_stop),
                                H5T_NATIVE_DOUBLE);
            }
         
         status = H5Tinsert (mtype, "elevation_start",
                             HOFFSET (struct ray_header, elevation_start),
                             H5T_NATIVE_DOUBLE);
         status = H5Tinsert (mtype, "elevation_stop",
                             HOFFSET (struct ray_header, elevation_stop),
                             H5T_NATIVE_DOUBLE);
         }
      else
         /*grava o Ray Header para rhi*/
         {
         /*angulo do RHI varia entre -5 e 185*/
         for (ray = 0; ray < s_how[i].ray_count; ray++)
            {
            if (s_how[i].r_header[ray].azimuth_start > 185)
               {
               s_how[i].r_header[ray].azimuth_start -= 360;
               }
            }
         
         status = H5Tinsert (mtype, "elevation_start",
                             HOFFSET (struct ray_header, azimuth_start),
                             H5T_NATIVE_DOUBLE);

         if (9999 == s_how[i].blob_stopangle_id)
            {
            for (ray = 0; ray < s_how[i].ray_count; ray++)
               {
               s_how[i].r_header[ray].azimuth_stop = s_how[i].r_header[ray].azimuth_start;
               }
            }
         
         status = H5Tinsert (mtype, "elevation_stop",
                             HOFFSET (struct ray_header, azimuth_stop),
                             H5T_NATIVE_DOUBLE);
         status = H5Tinsert (mtype, "azimuth_start",
                             HOFFSET (struct ray_header, elevation_start),
                             H5T_NATIVE_DOUBLE);
         status = H5Tinsert (mtype, "azimuth_stop",
                             HOFFSET (struct ray_header, elevation_stop),
                             H5T_NATIVE_DOUBLE);
         }
      
      if (9999 != s_how[i].blob_timestamp_id)
         {   
         status = H5Tinsert (mtype, "timestamp",
                             HOFFSET (struct ray_header, timestamp),
                             H5T_NATIVE_INT);
         }

      
      if (9999 != s_how[i].blob_txpower_id)
         {
         status = H5Tinsert (mtype, "txpower",
                             HOFFSET (struct ray_header, txpower),
                             H5T_NATIVE_INT);
         }
      
         /*
      * Create the compound datatype for the file.  Because the standard
      * types we are using for the file may have different sizes than
      * the corresponding native types, we must manually calculate the
      * offset of each member.
      */
      offs = 0;
      ftype = H5Tcreate (H5T_COMPOUND, sizeof(struct ray_header));
      status = H5Tinsert (ftype, "azimuth_start", 0, H5T_NATIVE_DOUBLE);
      if (9999 != s_how[i].blob_stopangle_id)
         {   
         status = H5Tinsert (ftype, "azimuth_stop",
                             ++offs*sizeof(double), H5T_NATIVE_DOUBLE);
         }
      
      status = H5Tinsert (ftype, "elevation_start",
                          ++offs*sizeof(double), H5T_NATIVE_DOUBLE);
      status = H5Tinsert (ftype, "elevation_stop",
                          ++offs*sizeof(double), H5T_NATIVE_DOUBLE);
      
      if (9999 != s_how[i].blob_timestamp_id)
         {   
         status = H5Tinsert (ftype, "timestamp",
                             ++offs*sizeof(double), H5T_NATIVE_INT);
         }

      if (9999 != s_how[i].blob_txpower_id)
         {
         if (9999 != s_how[i].blob_timestamp_id)
            {
            status = H5Tinsert (ftype, "txpower",
                                ++offs*sizeof(unsigned int), H5T_NATIVE_INT);
            }
         else
            {
            status = H5Tinsert (ftype, "txpower",
                                ++offs*sizeof(double), H5T_NATIVE_INT);
            }
         }
      
      /*
      * Create dataspace.  Setting maximum size to NULL sets the maximum
      * size to be the current size.
      */
      dim_header[0] = s_how[i].ray_count;
      space_header = H5Screate_simple (1, dim_header, NULL);
      
      /*
      * Create the dataset and write the compound data to it.
      */
      dset = H5Dcreate (scan[i], "ray_header", ftype, space_header,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      status = H5Dwrite (dset, mtype, H5S_ALL,
                         H5S_ALL, H5P_DEFAULT, s_how[i].r_header);
      
      status = H5Dclose (dset);
      status = H5Sclose (space_header);
      status = H5Tclose(ftype);
      status = H5Tclose(mtype);
 
      
      /*fim da gravacao do scan*/
      status = H5Gclose(scan[i]);
      status = H5Fclose(file);
      }
   
   
   
   /*fim de gravacao*/
   status = H5Sclose(space);

   return 0;
   
   }

void write_attr_text(hid_t loc_id,char *name, char *value)
   {//write atribute text
   herr_t status;
   hid_t attr_id;
   const hsize_t dims[1] = {1};
   hid_t memtype;
   hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

   memtype = H5Tcopy(H5T_C_S1);   
   H5Tset_size(memtype, H5T_VARIABLE);

   attr_id = H5Acreate(loc_id, name, memtype, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
   status = H5Awrite(attr_id, memtype, &value);
   status = H5Aclose(attr_id);
   status = H5Sclose(dataspace_id);
   }

void write_attr_double(hid_t loc_id,char *name,double value)
   {//write atribute double
   herr_t status;
   const hsize_t len=1;
   hid_t dataspace_id = H5Screate_simple(1, &len, NULL);
   hid_t attr_id=H5Acreate(loc_id,name, H5T_NATIVE_DOUBLE, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
   
   status = H5Awrite(attr_id,H5T_NATIVE_DOUBLE, &value);
   status = H5Aclose(attr_id);
   status = H5Sclose(dataspace_id);   
   }

void write_attr_uint(hid_t loc_id,char *name,int value)
   {//write atribute unsignet int
   herr_t status; 
   const hsize_t len = 1;
   hid_t dataspace_id = H5Screate_simple(1, &len, NULL);
   hid_t attr_id=H5Acreate(loc_id, name, H5T_NATIVE_UINT, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT);
   status = H5Awrite(attr_id, H5T_NATIVE_UINT, &value);
   status = H5Aclose(attr_id);
   status = H5Sclose(dataspace_id); 
   }
