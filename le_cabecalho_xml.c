#include "rainbow5_to_hdf5.h"

int le_cabecalho_xml(char *buffer, struct volume_how *v_how,
                     struct volume_what *v_what, struct volume_where *v_where,
                     struct scan_how s_how[], struct scan_what s_what[],
                     struct data dados[])
   {
   xmlDoc *doc = NULL;
   xmlNode *root_element = NULL;
   xmlNode *volume = NULL, *scan = NULL, *pargroup = NULL, *slice[MAX_SCANS];
   xmlNode *sensorinfo = NULL, *slicedata = NULL;
   xmlNode *rayinfo = NULL, *rawdata = NULL, *dynrange = NULL;

   /*variaveis para as informacoes dos raios*/
   xmlNode *starangle = NULL, *stopangle = NULL;
   xmlNode *timestamp = NULL, *numpulses = NULL;
   
   int numele = 0, i = 0, scn = 0, var = 0, ray = 0;
   char *texto = NULL, *texto2 = NULL;
   char date[MAX_STRING], time[MAX_STRING];

   /*
   -- definir na ordem dos DEFINES do rainbow5_to_hdf5.h --
   
   dynphi e dynrho nao vem no arquivo, mas colocamos aqui
   para manter o loop na hora da leitura. Esses limites sao fixos e sao
   incluidos depois (hard-coded)
   */
   char *dynrange_vars[MAX_VARS] = {"dynz", "dynz", "dynv", "dynw", 
                                    "dynzdr", "dynphi", "dynphi", "dynkdp",
                                    "dynrho"};
   
   doc = xmlReadMemory(buffer, strlen(buffer), "noname.xml", NULL, 0);
   if (doc == NULL)
      {
      return 1;
      }
   
   
   /*Pegando o no raiz */
   volume = xmlDocGetRootElement(doc);
   if (xmlStrcmp(volume->name, (const xmlChar *) "volume"))
      {
      return 1;
      }
   /*armazenando todos os ponteiros para os nodes XML pais*/
   scan = retorna_node(volume, "scan");
   if (NULL == scan) return 1;
   sensorinfo = retorna_node(volume, "sensorinfo");
   if (NULL == sensorinfo) return 1;
   pargroup = retorna_node(scan, "pargroup");
   if (NULL == pargroup) return 1;
   texto = retorna_valor(pargroup, "numele");
   if (NULL == texto) return 1;
   numele = atoi(texto);

   /*faz um loop para salvar os ponteiros das elevacoes*/
   slice[0] = retorna_node(scan, "slice");
   if (NULL == slice[0]) return 1;
   texto = retorna_atributos(slice[0], "refid");
   if (NULL == texto) return 1;
   if (atoi(texto) != 0) return 1;

   for (i = 1; i < numele; i++)
      {
      slice[i] = slice[i - 1]->next->next;
      if (NULL == slice[i]) return 1;
      texto = retorna_atributos(slice[i], "refid");
      if (NULL == texto) return 1;
      if (atoi(texto) != i) return 1;
      }

   /*comeca a leitura do cabecalho*/

   /*VOLUME HOW*/
   texto = retorna_atributos(volume, "version");
   if (NULL == texto) return 1;
   else strcpy(v_how->sw_version, texto);
   texto = retorna_atributos(scan, "name");
   if (NULL == texto) return 1;
   else strcpy(v_how->template_name, texto);
   texto = retorna_valor(sensorinfo, "beamwidth");
   if (NULL == texto) return 1;
   else v_how->beamwidth = atof(texto);
   texto = retorna_atributos(sensorinfo, "type");
   if (NULL == texto) return 1;
   else strcpy(v_how->sdp_name, texto);
   texto = retorna_atributos(sensorinfo, "id");
   if (NULL == texto) return 1;
   else strcpy(v_how->host_name, texto);
   strcpy(v_how->software, "Rainbow");
   /*VOLUME WHAT*/
   memset(date, 0, sizeof(date));
   memset(time, 0, sizeof(time));
   
   texto = retorna_atributos(scan, "time");
   if (NULL == texto) return 1;
   else strcpy(time, texto);
   texto = retorna_atributos(scan, "date");
   if (NULL == texto) return 1;
   else strcpy(date, texto);
   sprintf(v_what->date, "%sT%s.000Z", date, time);
   
   v_what->sets = numele;
   v_what->sets_scheduled = v_what->sets;
   
   texto = retorna_atributos(volume, "type");
   if (NULL == texto) return 1;
   else
      {
      if (0 == strcmp(texto, "vol"))
         {
         strcpy(v_what->object, "PVOL");
         }
      }
   
   /*VOLUME WHERE*/
   texto = retorna_valor(sensorinfo, "lon");
   if (NULL == texto) return 1;
   else v_where->lon = atof(texto);
   texto = retorna_valor(sensorinfo, "lat");
   if (NULL == texto) return 1;
   else v_where->lat = atof(texto);
   texto = retorna_valor(sensorinfo, "alt");
   if (NULL == texto) return 1;
   else v_where->height = atof(texto);

   /*SCAN HOW, header dos dados, SCAN WHAT*/

   /*le primeiro os atributos do scan 0, depois copia para todos os outros
   scans. O Rainbow soh guarda no cabecalho dos outros scans os valores que
   sao diferentes entre eles*/
   scn = 0;
   texto = retorna_valor(slice[scn], "highprf");
   if (NULL == texto) return 1;
   else s_how[scn].PRF = atoi(texto);
   
   slicedata = retorna_node(slice[scn], "slicedata");
   if (NULL == slicedata) return 1;

   rayinfo = retorna_node(slicedata, "rayinfo");
   if (NULL == rayinfo) return 1;
   texto = retorna_atributos(rayinfo, "refid");
   texto2 = retorna_atributos(rayinfo, "blobid");
   s_how[scn].blob_startangle_id = 9999;
   s_how[scn].blob_stopangle_id = 9999;
   s_how[scn].blob_timestamp_id = 9999;
   s_how[scn].blob_numpulses_id = 9999;
   s_how[scn].blob_txpower_id = 9999;  
   if (0 == strcmp(texto, "startangle"))
      {
      s_how[scn].blob_startangle_id = atoi(texto2);
      }
   if (0 == strcmp(texto, "stopangle"))
      {
      s_how[scn].blob_stopangle_id = atoi(texto2);
      }
   if (0 == strcmp(texto, "numpulses"))
      {
      s_how[scn].blob_numpulses_id = atoi(texto2);
      }
   if (0 == strcmp(texto, "timestamp"))
      {
      s_how[scn].blob_timestamp_id = atoi(texto2);
      }
   if (0 == strcmp(texto, "txpower"))
      {
      s_how[scn].blob_txpower_id = atoi(texto2);
      }
   /*procura por outros atributos de rayinfo*/
   
   for (i = 0; i < MAX_RAYINFO_REFID; i++)
      {
      rayinfo = rayinfo->next->next;
      if (NULL == rayinfo)
         break;
      texto = retorna_atributos(rayinfo, "refid");
      texto2 = retorna_atributos(rayinfo, "blobid");
      if (NULL != texto)
         {
         if (0 == strcmp(texto, "startangle"))
            {
            s_how[scn].blob_startangle_id = atoi(texto2);
            }
         if (0 == strcmp(texto, "stopangle"))
            {
            s_how[scn].blob_stopangle_id = atoi(texto2);
            }
         if (0 == strcmp(texto, "numpulses"))
            {
            s_how[scn].blob_numpulses_id = atoi(texto2);
            }
         if (0 == strcmp(texto, "timestamp"))
            {
            s_how[scn].blob_timestamp_id = atoi(texto2);
            }
         if (0 == strcmp(texto, "txpower"))
            {
            s_how[scn].blob_txpower_id = atoi(texto2);
            }
         }
      }
   
   rawdata = retorna_node(slicedata, "rawdata");
   if (NULL == rawdata) return 1;
   
   s_how[scn].angle_sync = 1;

   memset(date, 0, sizeof(date));
   memset(time, 0, sizeof(time));
   texto = retorna_atributos(slicedata, "time");
   if (NULL == texto) return 1;
   else strcpy(time, texto);
   texto = retorna_atributos(slicedata, "date");
   if (NULL == texto) return 1;
   else strcpy(date, texto);
   sprintf(s_how[scn].timestamp, "%sT%s.000Z", date, time);
   
   texto = retorna_atributos(rawdata, "bins");
   if (NULL == texto) return 1;
   else s_how[scn].bin_count = atoi(texto);
   texto = retorna_atributos(rawdata, "rays");
   if (NULL == texto) return 1;
   else s_how[scn].ray_count = atoi(texto);
   texto = retorna_atributos(rawdata, "blobid");
   if (NULL == texto) return 1;
   else s_how[scn].blob_data_id = atoi(texto);
   texto = retorna_valor(slice[scn], "rangesamp");
   if (NULL == texto) return 1;
   else s_how[scn].range_samples = atoi(texto);
   texto = retorna_valor(slice[scn], "timesamp");
   if (NULL == texto) return 1;
   else s_how[scn].time_samples = atoi(texto);


   texto = retorna_valor(slice[scn], "gdrxanctxfreq");
   if (NULL == texto) return 1;
   if (atof(texto) < 1)
      {
      s_how[scn].malfunc = 1;
      }   
   
   texto = retorna_valor(slice[scn], "anglestep");
   if (NULL == texto) return 1;
   else s_how[scn].angle_step = atof(texto);
   texto = retorna_valor(slice[scn], "posangle");
   if (NULL == texto) return 1;
   else s_how[scn].elevation = atof(texto);
   texto = retorna_valor(slice[scn], "posangle");
   if (NULL == texto) return 1;
   else s_how[scn].elevation = atof(texto);
   texto = retorna_valor(slice[scn], "pw_index");
   if (NULL == texto) return 1;
   else
      {
      switch (atoi(texto))
         {
         case 0:
            {
            s_how[scn].long_pulse = 0;
            s_how[scn].pulse_width_us = 0.5;
            break;
            }
         case 1:
            {
            s_how[scn].long_pulse = 1;
            s_how[scn].pulse_width_us = 1.0;
            break;
            }
         case 2:
            {
            s_how[scn].long_pulse = 1;
            s_how[scn].pulse_width_us = 2.0;
            break;
            }
         default:
            break;
         }
      }
   
   texto = retorna_valor(sensorinfo, "wavelen");
   if (NULL == texto) return 1;
   else s_how[scn].radar_wave_length = atof(texto);
   
   texto = retorna_valor(slice[scn], "stoprange");
   if (NULL == texto) return 1;
   else s_how[scn].range = atof(texto)*1000;
   texto = retorna_valor(slice[scn], "start_range");
   if (NULL == texto) return 1;
   else s_how[scn].range_start = atof(texto)*1000;
   texto = retorna_valor(slice[scn], "rangestep");
   if (NULL == texto) return 1;
   else s_how[scn].range_step = atof(texto)*1000;
   texto = retorna_valor(slice[scn], "antspeed");
   if (NULL == texto) return 1;
   else s_how[scn].scan_speed = atof(texto);

   texto = retorna_valor(slice[scn], "stagger");
   if (NULL == texto) return 1;
   if (!strncmp(texto, "Non", 3)) s_how[scn].unfolding = 0;
   if (!strncmp(texto, "2/3", 3)) s_how[scn].unfolding = 1;
   if (!strncmp(texto, "3/4", 3)) s_how[scn].unfolding = 2;
   if (!strncmp(texto, "5/4", 3)) s_how[scn].unfolding = 3;
   
   
   for (var = 0; var < MAX_VARS; var++)
      {
      dynrange = retorna_node(slice[scn], dynrange_vars[var]);
      if (NULL != dynrange)
         {
         texto = retorna_atributos(dynrange, "min");
         if (NULL == texto) return 1;
         else dados[scn].header[var].dyn_range_min = atof(texto);
         
         texto = retorna_atributos(dynrange, "max");
         if (NULL == texto) return 1;
         else dados[scn].header[var].dyn_range_max = atof(texto);         
         }
      }
   
   dados[scn].header[VAR_PHI].dyn_range_min = 0;
   dados[scn].header[VAR_PHI].dyn_range_max = 360;
   dados[scn].header[VAR_UPHI].dyn_range_min = 0;
   dados[scn].header[VAR_UPHI].dyn_range_max = 360;
   dados[scn].header[VAR_RHO].dyn_range_min = 0;
   dados[scn].header[VAR_RHO].dyn_range_max = 1;

   /*8/16 bits de acordo com o manual da Gematronik*/
   strcpy(dados[scn].header[VAR_UZ].format, "UV8");
   strcpy(dados[scn].header[VAR_CZ].format, "UV8");
   strcpy(dados[scn].header[VAR_V].format, "UV8");
   strcpy(dados[scn].header[VAR_W].format, "UV8");
   strcpy(dados[scn].header[VAR_ZDR].format, "UV8");
   strcpy(dados[scn].header[VAR_PHI].format, "UV16");
   strcpy(dados[scn].header[VAR_UPHI].format, "UV16");
   strcpy(dados[scn].header[VAR_KDP].format, "UV16");
   strcpy(dados[scn].header[VAR_RHO].format, "UV8");
   
   strcpy(dados[scn].header[VAR_UZ].moment, "UZ");
   strcpy(dados[scn].header[VAR_CZ].moment, "Z");
   strcpy(dados[scn].header[VAR_V].moment, "V");
   strcpy(dados[scn].header[VAR_W].moment, "W");
   strcpy(dados[scn].header[VAR_ZDR].moment, "ZDR");
   strcpy(dados[scn].header[VAR_PHI].moment, "PhiDP");
   strcpy(dados[scn].header[VAR_UPHI].moment, "uPhiDP");
   strcpy(dados[scn].header[VAR_KDP].moment, "KDP");
   strcpy(dados[scn].header[VAR_RHO].moment, "RhoHV");
   
   strcpy(dados[scn].header[VAR_UZ].unit, "dBZ");
   strcpy(dados[scn].header[VAR_CZ].unit, "dBZ");
   strcpy(dados[scn].header[VAR_V].unit, "m/s");
   strcpy(dados[scn].header[VAR_W].unit, "m/s");
   strcpy(dados[scn].header[VAR_ZDR].unit, "dBZ");
   strcpy(dados[scn].header[VAR_PHI].unit, "deg");
   strcpy(dados[scn].header[VAR_UPHI].unit, "deg");
   strcpy(dados[scn].header[VAR_KDP].unit, "deg/km");
   strcpy(dados[scn].header[VAR_RHO].unit, "dimensionless");

   /*colocar -1 porque nao estamos convertendo uPhiDP*/
   s_what[scn].descriptor_count = MAX_VARS - 1;
   
   strcpy(s_what[scn].scan_type, "PPI");
   strcpy(s_what[scn].product, "SCAN");

   for (scn = 1; scn < numele; scn++)
      {
      /*cabecalho preenchido para a elevacao 0. Copia o cabecalho anterior
      para a elevacao corrente*/
      memcpy(&s_how[scn], &s_how[scn-1], sizeof(struct scan_how));
      memcpy(&s_what[scn], &s_what[scn-1], sizeof(struct scan_what));
      memcpy(&dados[scn], &dados[scn-1], sizeof(struct data));


      /*preenche o cabecalho das outras elevacoes. Agora nao retorna 1 se nao
      encontrar o parametro, se ele nao existe eh porque eh igual o da elevacao
      zero. Se nao encontra apenas nao grava*/
      

      texto = retorna_valor(slice[scn], "highprf");
      if (NULL != texto) s_how[scn].PRF = atoi(texto);
      
      slicedata = retorna_node(slice[scn], "slicedata");
      if (NULL == slicedata) return 1;
      rayinfo = retorna_node(slicedata, "rayinfo");
      if (NULL == rayinfo) return 1;
      texto = retorna_atributos(rayinfo, "refid");
      texto2 = retorna_atributos(rayinfo, "blobid");

      s_how[scn].blob_startangle_id = 9999;
      s_how[scn].blob_stopangle_id = 9999;
      s_how[scn].blob_timestamp_id = 9999;
      s_how[scn].blob_numpulses_id = 9999;
      s_how[scn].blob_txpower_id = 9999;
      
      if (0 == strcmp(texto, "startangle"))
         {
         s_how[scn].blob_startangle_id = atoi(texto2);
         }
      if (0 == strcmp(texto, "stopangle"))
         {
         s_how[scn].blob_stopangle_id = atoi(texto2);
         }
      if (0 == strcmp(texto, "numpulses"))
         {
         s_how[scn].blob_numpulses_id = atoi(texto2);
         }
      if (0 == strcmp(texto, "timestamp"))
         {
         s_how[scn].blob_timestamp_id = atoi(texto2);
         }
      /*procura por outros atributos de rayinfo*/
      
      for (i = 0; i < MAX_RAYINFO_REFID; i++)
         {
         rayinfo = rayinfo->next->next;
         if (NULL == rayinfo)
            break;
         texto = retorna_atributos(rayinfo, "refid");
         texto2 = retorna_atributos(rayinfo, "blobid");
         if (NULL != texto)
            {
            if (0 == strcmp(texto, "startangle"))
               {
               s_how[scn].blob_startangle_id = atoi(texto2);
               }
            if (0 == strcmp(texto, "stopangle"))
               {
               s_how[scn].blob_stopangle_id = atoi(texto2);
               }
            if (0 == strcmp(texto, "numpulses"))
               {
               s_how[scn].blob_numpulses_id = atoi(texto2);
               }
            if (0 == strcmp(texto, "timestamp"))
               {
               s_how[scn].blob_timestamp_id = atoi(texto2);
               }
            if (0 == strcmp(texto, "txpower"))
               {
               s_how[scn].blob_txpower_id = atoi(texto2);
               }
            }
         }
      
      rawdata = retorna_node(slicedata, "rawdata");
      if (NULL == rawdata) return 1;
      s_how[scn].angle_sync = 1;
      texto = retorna_atributos(rawdata, "bins");
      
      memset(date, 0, sizeof(date));
      memset(time, 0, sizeof(time));
      texto = retorna_atributos(slicedata, "time");
      if (NULL != texto)  strcpy(time, texto);
      texto = retorna_atributos(slicedata, "date");
      if (NULL != texto) strcpy(date, texto);
      sprintf(s_how[scn].timestamp, "%sT%s.000Z", date, time);
      texto = retorna_atributos(rawdata, "bins");
      if (NULL != texto) s_how[scn].bin_count = atoi(texto);
      texto = retorna_atributos(rawdata, "rays");
      if (NULL != texto) s_how[scn].ray_count = atoi(texto);

      texto = retorna_atributos(rawdata, "blobid");
      if (NULL != texto) s_how[scn].blob_data_id = atoi(texto);

      texto = retorna_valor(slice[scn], "rangesamp");
      if (NULL != texto) s_how[scn].range_samples = atoi(texto);
      texto = retorna_valor(slice[scn], "timesamp");
      if (NULL != texto) s_how[scn].time_samples = atoi(texto);
      
      texto = retorna_valor(slice[scn], "gdrxanctxfreq");
      s_how[scn].malfunc = 0;
      if (NULL != texto)
         {
         if (atof(texto) < 1)
            {
            s_how[scn].malfunc = 1;
            }
         }
      
      texto = retorna_valor(slice[scn], "anglestep");
      if (NULL != texto) s_how[scn].angle_step = atof(texto);
      texto = retorna_valor(slice[scn], "posangle");
      if (NULL != texto) s_how[scn].elevation = atof(texto);
      texto = retorna_valor(slice[scn], "posangle");
      if (NULL != texto) s_how[scn].elevation = atof(texto);
      texto = retorna_valor(slice[scn], "pw_index");
      if (NULL != texto)
         {
         switch (atoi(texto))
            {
            case 0:
               {
               s_how[scn].long_pulse = 0;
               s_how[scn].pulse_width_us = 0.5;
               break;
               }
            case 1:
               {
               s_how[scn].long_pulse = 1;            
               s_how[scn].pulse_width_us = 1.0;
               break;
               }
            case 2:
               {
               s_how[scn].long_pulse = 1;
               s_how[scn].pulse_width_us = 2.0;
               break;
               }
            default:
               break;
            }
         }
      
      texto = retorna_valor(sensorinfo, "wavelen");
      if (NULL != texto) s_how[scn].radar_wave_length = atof(texto);
      
      texto = retorna_valor(slice[scn], "stoprange");
      if (NULL != texto) s_how[scn].range = atof(texto)*1000;
      texto = retorna_valor(slice[scn], "start_range");
      if (NULL != texto) s_how[scn].range_start = atof(texto)*1000;
      texto = retorna_valor(slice[scn], "rangestep");
      if (NULL != texto) s_how[scn].range_step = atof(texto)*1000;
      texto = retorna_valor(slice[scn], "antspeed");
      if (NULL != texto) s_how[scn].scan_speed = atof(texto);
      
      texto = retorna_valor(slice[scn], "stagger");
      if (NULL != texto)
         {
         if (!strncmp(texto, "Non", 3)) s_how[scn].unfolding = 0;
         if (!strncmp(texto, "2/3", 3)) s_how[scn].unfolding = 1;
         if (!strncmp(texto, "3/4", 3)) s_how[scn].unfolding = 2;
         if (!strncmp(texto, "5/4", 3)) s_how[scn].unfolding = 3;
         }
      
      for (var = 0; var < MAX_VARS; var++)
         {
         dynrange = retorna_node(slice[scn], dynrange_vars[var]);
         if (NULL != dynrange)
            {
            texto = retorna_atributos(dynrange, "min");
            if (NULL != texto)
               dados[scn].header[var].dyn_range_min = atof(texto);
            
            texto = retorna_atributos(dynrange, "max");
            if (NULL != texto)
               dados[scn].header[var].dyn_range_max = atof(texto);         
            }
         }
      }



   /* preenche o cabecalho dos rays*/
   for (scn = 0; scn < numele; scn++)
      {
      for (ray = 0; ray < s_how[scn].ray_count; ray++)
         {
         s_how[scn].r_header[ray].elevation_start = s_how[scn].elevation;
         s_how[scn].r_header[ray].elevation_stop = s_how[scn].elevation;
         }
      }
   
   
   xmlFreeDoc(doc);
   xmlCleanupParser();
   return 0;
   }


char *retorna_atributos(xmlNode *node, char *texto)
   {
   xmlAttr *properties = NULL;
   properties = node->properties;
   char encontrado = 0;

   if (NULL == node)
      {   
      return NULL;
      }
   
   while (NULL != properties)
      {
      if (! xmlStrcmp(properties->name, (const xmlChar *) texto))
         {
         return (char *) properties->children->content;
         }
      else
         {
         properties = properties->next;
         }
      }
   
   return NULL;
   }


char *retorna_valor(xmlNode *node, char *texto)
   {
   xmlNode *children = NULL;
   children = node->children->next;
   
   if (NULL == children)
      {   
      return NULL;
      }
   
   while (NULL != children)
      {
      if (! xmlStrcmp(children->name, (const xmlChar *) texto))
         {
         return (char *) children->children->content;
         }
      else
         {
         children = children->next;
         }
      }
   return NULL;   
   }


xmlNode *retorna_node(xmlNode *parent_node, char *texto)
   {
   
   xmlNode *children = NULL;
   
   children = parent_node->children->next;
   
   while (NULL != children)
      {
      if (! xmlStrcmp(children->name, (const xmlChar *) texto))
         {
         return children;
         }
      else
         {
         children = children->next;
         }
      }
   return NULL;
   }
