/*
*
* Thiago Biscaro - thiago.biscaro@cptec.inpe.br
*
* */

#include <QtCore/QByteArray>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "rainbow5_to_hdf5.h"

//using namespace std;



int le_dados_blob(char *nome, int numele, struct volume_how *v_how,
                  struct scan_how s_how[], struct data dados[])
   {
   int tem_stopangle = 0;
   
   QByteArray bufComp;
   QByteArray bufOut;
   FILE *fin[MAX_VARS];
   FILE *fp = NULL;
   char line[MAX_LINE_SIZE];
   char *line2 = NULL, *buf = NULL;
   unsigned short int *buffer_16 = NULL;
   unsigned int *buffer_32 = NULL;
   unsigned char *buffer_8 = NULL;
   
   size_t nbytes = MAX_LINE_SIZE - 1;
   char begin_pargroup = 0, end_pargroup = 0,
      begin_slice = 0, end_slice = 0;
   char tmp_str[MAX_LINE_SIZE];
   struct params_slice p_slice[MAX_SCANS];
   
   char scan_type = 0;
   int slice = 0;
   int i = 0, j = 0, arq = 0;
   unsigned short int stagger = 0;
   unsigned short int blobid = 0;
   unsigned short int depth = 0;
   unsigned short int num_arquivos = 0;
   unsigned int size_blob = 0;
   float step = 0, max = 0, min = 0;
   float value_obs = 0;
   
   char filename[1024];
   
   unsigned char temp_8[MAX_RAYS][MAX_BINS];
   unsigned short int temp_16[MAX_RAYS][MAX_BINS];


   /*verifica a qtd de arquivos*/
   num_arquivos = retorna_ponteiro_arquivos(fin, nome);
   printf("num arq %d\n", num_arquivos);
   if (0 == num_arquivos)
      {
      return 1;
      }
   
   for (arq = 0; arq < num_arquivos; arq++)
      {
      fp = fin[arq];
      if (NULL == fp)
         {
         return 1;
         }
      line2 = (char *) malloc (nbytes + 1);
      if (NULL == line2)
         {
         return 1;
         }
   
      memset(line, 0, MAX_LINE_SIZE);
      slice = 0;
      while (NULL == strstr(line, "<!-- END XML -->"))
         {
         memset(line, 0, MAX_LINE_SIZE);

         if (getline(&line2, &nbytes, fp) < 0)
            {
            free(line2);
            return 1;
            }
         strncpy(line, line2, MAX_LINE_SIZE);
         
         if (NULL != strstr(line, "<rawdata "))
            {         
            strcpy(tmp_str, "rays");
            return_info(line, tmp_str, tmp_str);
            p_slice[slice].rays = atoi(tmp_str);
         
            strcpy(tmp_str, "bins");
            return_info(line, tmp_str, tmp_str);
            p_slice[slice].bins = atoi(tmp_str);
         
            strcpy(tmp_str, "type");
            return_info(line, tmp_str, tmp_str);
            if (strcmp(tmp_str, "dBZ") == 0)
               p_slice[slice].var = VAR_CZ;
            if (strcmp(tmp_str, "dBuZ") == 0)
               p_slice[slice].var = VAR_UZ;
            if (strcmp(tmp_str, "V") == 0)
               p_slice[slice].var = VAR_V;
            if (strcmp(tmp_str, "W") == 0)
               p_slice[slice].var = VAR_W;
            if (strcmp(tmp_str, "ZDR") == 0)
               p_slice[slice].var = VAR_ZDR;
            if (strcmp(tmp_str, "PhiDP") == 0)
               p_slice[slice].var = VAR_PHI;
            if (strcmp(tmp_str, "uPhiDP") == 0)
               p_slice[slice].var = VAR_UPHI;
            if (strcmp(tmp_str, "KDP") == 0)
               p_slice[slice].var = VAR_KDP;
            if (strcmp(tmp_str, "RhoHV") == 0)
               p_slice[slice].var = VAR_RHO;

            slice++;
            }

         }

      if (numele != slice)
         {
         printf("Erro: Numero de slices (%d) invalido - deveria ser %d\n", slice, numele);
         free(line2);
         return 1;
         }
   
      slice = 0;
      
      while (feof(fp) == 0)
         {
         /*reads binary information (and headers)*/
         memset(line, 0, MAX_LINE_SIZE);
         if (getline(&line2, &nbytes , fp) < 0)
            {
            free(line2);
            return 1;
            }
         strncpy(line, line2, MAX_LINE_SIZE);  
         /*Now reads BLOB ID info*/
      
         if (NULL != strstr(line, "<BLOB "))
            {
            strcpy(tmp_str, "blobid");
            return_info(line, tmp_str, tmp_str);
            blobid = atoi(tmp_str);
            strcpy(tmp_str, "size");
            return_info(line, tmp_str, tmp_str);
            size_blob = atoi(tmp_str);
            /*reads binary data*/
            buf = (char *) malloc(size_blob);

            if (NULL == buf)
               {
               printf("Erro alocando buffer para leitura\n");
               return 1;
               }
            
            if ((blobid == s_how[slice].blob_stopangle_id) ||
                (blobid == s_how[slice].blob_startangle_id) ||
                (blobid == s_how[slice].blob_numpulses_id))
               {
               depth = 16;
               }
            else if (blobid == s_how[slice].blob_timestamp_id)
               {
               depth = 32;
               }
            else
               {
               if ((VAR_PHI ==  p_slice[slice].var) ||
                   (VAR_UPHI ==  p_slice[slice].var) ||
                   (VAR_KDP ==  p_slice[slice].var))
                  {
                  depth = 16;
                  }
               else
                  {
                  depth = 8;
                  }
               }
            
         
            if (fread(buf, size_blob, 1, fp))
               {
            
               /*This is actually the only C++ part of the code*/
               bufComp = QByteArray(buf, size_blob);
               bufOut = qUncompress(bufComp);
            
               size_blob = bufOut.size();
               buffer_8 = (unsigned char *) malloc(size_blob);
               memcpy(buffer_8, bufOut.data(), size_blob);
            
               if (16 == depth)
                  {
                  /*16 bit data (put on Little Endian order)*/
                  buffer_16 = (unsigned short int *) malloc(size_blob);

                  memcpy(buffer_16, bufOut.data(), size_blob);
                  for (i = 0; i < size_blob/2; i++)
                     {
                     buffer_16[i] = (buffer_16[i] << 8) | (buffer_16[i] >> 8 );
                     }
                  }
               
               if (32 == depth)
                  {
                  /*16 bit data (put on Little Endian order)*/
                  buffer_32 = (unsigned int *) malloc(size_blob);
                  memcpy(buffer_32, bufOut.data(), size_blob);
                  for (i = 0; i < size_blob/4; i++)
                     {
                     buffer_32[i] = ((buffer_32[i] << 8) & 0xFF00FF00 ) |
                        ((buffer_32[i] >> 8) & 0xFF00FF);                     
                     }
                  }
               /*END OF C++ part of the code*/

               /*verifica se o BLOBID eh de dados ou de rayinfo
               por enquanto so le os dados de angulos*/
               if (blobid == s_how[slice].blob_startangle_id)
                  {
                  step = 360.0/65536.0;
                  for (i = 0; i < p_slice[slice].rays; i++)
                     {
                     s_how[slice].r_header[i].azimuth_start =
                        (float) (buffer_16[i] * step);
                     if (-1 == s_how[slice].blob_stopangle_id)
                        {
                        /*nao tem stopangle, usa o mesmo*/
                        s_how[slice].r_header[i].azimuth_stop =
                           s_how[slice].r_header[i].azimuth_start;
                        }
                     }
                  s_how[slice].azi_start =
                     s_how[slice].r_header[0].azimuth_start;
                  s_how[slice].azi_stop =
                     s_how[slice].r_header[p_slice[slice].rays-1].azimuth_start;
                  
                  }

               if (blobid == s_how[slice].blob_stopangle_id)
                  {
                  step = 360.0/65536.0;
                  for (i = 0; i < p_slice[slice].rays; i++)
                     {
                     s_how[slice].r_header[i].azimuth_stop =
                        (float) (buffer_16[i] * step);
                     }
                  }

               if (blobid == s_how[slice].blob_timestamp_id)
                  {
                  for (i = 0; i < p_slice[slice].rays; i++)
                     {
                     s_how[slice].r_header[i].timestamp =
                        (long int) buffer_32[i];
                     }
                  }
               
               if (blobid == s_how[slice].blob_data_id)
                  {
                  if (8 == depth)
                     {
                     for (i = 0; i < p_slice[slice].rays; i++)
                        {
                        for (j = 0; j < p_slice[slice].bins; j++)
                           {
                           temp_8[i][j] = buffer_8[j + i*p_slice[slice].bins];
                           }
                        }
                     }
                  else
                     {
                     for (i = 0; i < p_slice[slice].rays; i++)
                        {
                        for (j = 0; j < p_slice[slice].bins; j++)
                           {
                           temp_16[i][j] = buffer_16[j + i*p_slice[slice].bins];
                           }
                        }
                     }
                  
                  switch (p_slice[slice].var)
                     {
                     case VAR_CZ:
                        {
                        memcpy(&dados[slice].dBZ[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     case VAR_UZ:
                        {
                        memcpy(&dados[slice].dBuZ[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     case VAR_V:
                        {
                        memcpy(&dados[slice].V[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     case VAR_W:
                        {
                        memcpy(&dados[slice].W[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     case VAR_ZDR:
                        {
                        memcpy(&dados[slice].ZDR[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     case VAR_PHI:
                        {
                        memcpy(&dados[slice].PhiDP[0][0], &temp_16[0][0],
                               sizeof(temp_16));
                        break;
                        }
                     case VAR_UPHI:
                        {
                        memcpy(&dados[slice].uPhiDP[0][0], &temp_16[0][0],
                               sizeof(temp_16));
                        break;
                        }
                     case VAR_KDP:
                        {
                        memcpy(&dados[slice].KDP[0][0], &temp_16[0][0],
                               sizeof(temp_16));
                        break;
                        }
                     case VAR_RHO:
                        {
                        memcpy(&dados[slice].RhoHV[0][0], &temp_8[0][0],
                               sizeof(temp_8));
                        break;
                        }
                     default:
                        {
                        printf("ERRO: LENDO VARIAVEL DESCONHECIDA\n");
                        break;
                        }
                     }
                  /*dados são sempre o ultimo BLOB a ser lido, soma 1 no slice*/
                  v_how->vars_validas[p_slice[slice].var] = 1;
                  slice++;
                  }

               free(buffer_8);

               if (16 == depth)
                  {
                  free(buffer_16);
                  }
               if (32 == depth)
                  {
                  free(buffer_32);
                  }
               }
            else
               {
               return 1;
               }
            free(buf);
            }
         }      
      fclose(fp);
      free(line2);
      }
   return 0;
   }



void return_value(char *source, char *result)
   {
   int i=0, j=0;

   memset(result, 0, MAX_LINE_SIZE);
   
   for (i = 0; i < MAX_LINE_SIZE; i++)
      {
      if (source[i] == '>')
         {
         while (source[++i] != '<')
            {
            result[j++] = source[i];
            }
         return;
         }
      }
   }


void return_info(char *line, char *pattern, char *result)
   {
   /*return values between " "*/
   int begin = 0;
   char *tmp = NULL;
   int size_str = 0;
   int i = 0, j = 0;
   
   tmp = strstr(line, pattern);

   if (NULL == tmp)
      {
      return;
      }
   
   size_str = strlen(pattern);
   /*info starts at position strlen + 2 (to account for ' =" ') */
   i = size_str + 2;
   j = 0;
   memset(result, 0, MAX_LINE_SIZE);
   
   while (tmp[i] != '\"')
      result[j++] = tmp[i++];
   
   return;
   }

int retorna_ponteiro_arquivos(FILE *fin[], char *nome)
   {
   int i = 0, arq_valido = 0;

   /*nao leremos o uPhiDP*/
   char *sufixo[MAX_VARS] = {"dBZ", "dBuZ", "V", "W", "ZDR",
                             "PhiDP", "NAO_LER", "KDP", "RhoHV"};
   
   char arquivo[MAX_LINE_SIZE];
   char diretorio[MAX_LINE_SIZE];
   char file[MAX_LINE_SIZE];

   FILE *fp = NULL;
   
   int num_arquivos = 0;
   
   for (i = 0; i < MAX_VARS; i++)
      {
      if (NULL != strstr(nome, sufixo[i]))
         {
         break;
         }
      }
   if (MAX_VARS == i)
      {
      printf("Arquivo com nome invalido\n");
      return 0;
      }
   /*separa o nome do arquivo do nome do diretorio, se houver*/
   memset(arquivo, 0, sizeof(arquivo));
   memset(diretorio, 0, sizeof(diretorio));
   
   if (NULL != strstr(nome, "/"))
      {
      for (i = strlen(nome); i > 0; i--)
         {
         if (nome[i] == '/')
            {
            break;
            }
         }
      memcpy(arquivo, &nome[i+1], 16);
      memcpy(diretorio, &nome[0], i);
      }
   else
      {
      strncpy(arquivo, nome, 16);
      strcpy(diretorio, ".");
      }
   
   for (i = 0; i < MAX_VARS; i++)
      {
      memset(file, 0, sizeof(file));
      sprintf(file, "%s/%s%s.vol", diretorio, arquivo, sufixo[i]);
      fin[i] = NULL;
      fp = fopen(file, "r");
      
      if (NULL != fp)
         {
         printf("lendo arquivo %s\n", file);
         fin[num_arquivos] = fp;
         num_arquivos++;
         }
      
      }
   
   return num_arquivos;
   
   
   }
