#include "rainbow5_to_hdf5.h"

int main(int argc, char **argv)
   {
   char buffer[MAX_BUFFER_SIZE];
   char line[MAX_LINE_SIZE];
   char *line2 = NULL;
   size_t nbytes = MAX_LINE_SIZE - 1, addr = 0;
   int charread = 0;
   FILE *fp = NULL;
   
   struct volume_how v_how;
   struct volume_what v_what;
   struct volume_where v_where;
   struct scan_how s_how[MAX_SCANS];
   struct scan_what s_what[MAX_SCANS];
   struct data dados[MAX_SCANS];
   
   if (argc != 2)
      {
      printf("Uso: %s arquivo de entrada\n", argv[0]);
      return 1;
      }

   fp = fopen(argv[1], "r");
   if (NULL == fp)
      {
      printf("Erro abrindo arquivo %s\n", argv[1]);
      return 1;
      }


   memset(&v_how, 0, sizeof(v_how));
   memset(&v_what, 0, sizeof(v_what));
   memset(&v_where, 0, sizeof(v_where));
   memset(s_how, 0, sizeof(s_how));
   memset(s_what, 0, sizeof(s_what));
   memset(dados, 0, sizeof(dados));   
   memset(line, 0, MAX_LINE_SIZE);
   memset(buffer, 0, MAX_BUFFER_SIZE);
   while (NULL == strstr(line, "<!-- END XML -->"))
      {
      memset(line, 0, MAX_LINE_SIZE);
      charread = getline(&line2, &nbytes , fp);      
      if (charread < 0)
         {
         return 1;
         }
      memcpy(&buffer[addr], line2, charread);
      addr = addr + charread;
      strcpy(line, line2);
      }
   fclose(fp);
   
   if (le_cabecalho_xml(buffer, &v_how, &v_what, &v_where,
                        s_how, s_what, dados))
      {
      printf("Erro na leitura do header - Saindo\n");
      return 1;
      }


   printf("Leitura do header ok\n");
   
   
   if (le_dados_blob(argv[1], v_what.sets, &v_how, s_how, dados))
      {
      printf("Erro na leitura dos dados - Saindo\n");
      return 1;
      }
   
   printf("Leitura dos dados ok\n");
   
   if (write_hdf5(&v_how, &v_what, &v_where,
                  s_how, s_what, dados))
      {
      printf("Erro na escrita do arquivo - Saindo\n");
      return 1;
      }
   printf("Escrita dos dados ok\n");

   return 0;
   }
