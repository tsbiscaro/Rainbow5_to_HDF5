#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <hdf5.h>

#define VAR_CZ   0
#define VAR_UZ   VAR_CZ + 1
#define VAR_V    VAR_CZ + 2
#define VAR_W    VAR_CZ + 3
#define VAR_ZDR  VAR_CZ + 4
#define VAR_PHI  VAR_CZ + 5
#define VAR_UPHI VAR_CZ + 6
#define VAR_KDP  VAR_CZ + 7
#define VAR_RHO  VAR_CZ + 8
#define MAX_VARS VAR_RHO + 1

#define MAX_STRING 256
#define MAX_SCANS 18

#define TYPE_8  1
#define TYPE_16 2

/*maximo de resolucao azimutal sera de 0.25 grau*/
#define MAX_RAYS 361*4
/*maximo de resolucao radial sera de 100m em 250 km de raio*/
#define MAX_BINS 2500

#define RAYS 1000
#define BINS RAYS

#define MAX_BUFFER_SIZE 1024*128
#define MAX_LINE_SIZE 512

#define MAX_RAYINFO_REFID 8

//int moments = 5;
//char moments_validos[MAX_VARS] = {1, 1, 1, 1, 1, 0, 1, 1, 1};

struct volume_how
   {
   char host_name[MAX_STRING];
   char sdp_name[MAX_STRING];
   char sdp_version[MAX_STRING];
   char simulated[MAX_STRING];
   char site_name[MAX_STRING];
   char software[MAX_STRING];
   char sw_version[MAX_STRING];
   char template_name[MAX_STRING];
   char vars_validas[MAX_VARS];
   char arq_original[MAX_STRING];
   double beamwidth;
   double padding_F[7];
   };


struct volume_what
   {
   char date[MAX_STRING];
   char object[MAX_STRING];
   unsigned int sets;
   unsigned int sets_scheduled;
   unsigned int version;
   unsigned int padding_USI[5];
   };


struct volume_where
   {
   double height;
   double lat;
   double lon;
   double padding_F[5];
   };


struct ray_header_complete
   {
   double azimuth_start;
   double azimuth_stop;
   double elevation_start;
   double elevation_stop;
   double az_speed;
   double el_speed;
   double burst_freq;
   double burst_power;
   double pci_dma_tranfer_rate;
   double pci_dma_fifo_fill;
   double tcp_tranfer_rate;
   double host_throughput;
   double pc_gain;
   double padding_F[3];
   unsigned long long int timestamp;
   unsigned long int tm_stop;
   unsigned long int ifd_power_flags;
   unsigned long int adc_overflow_flags;
   unsigned int cur_prf;
   unsigned int num_pulses;
   unsigned int shortpulse_bins;
   unsigned int afc_mode;
   unsigned int afc_status;
   unsigned int padding_USI[3];
   };

struct ray_header
   {
   double azimuth_start;
   double azimuth_stop;
   double elevation_start;
   double elevation_stop;
   double az_speed;
   unsigned long long int timestamp;
   unsigned int txpower;
//   unsigned int padding_UI[2];
   };


struct scan_how
   {
   unsigned int PRF;
   unsigned int angle_sync;
   unsigned int bin_count;
   unsigned int filter;
   unsigned int malfunc;
   unsigned int range_samples;
   unsigned int ray_count;
   unsigned int time_samples;
   unsigned int unfolding;
   unsigned int blob_data_id;
   unsigned int blob_startangle_id;
   unsigned int blob_stopangle_id;
   unsigned int blob_numpulses_id;
   unsigned int blob_timestamp_id;
   unsigned int blob_txpower_id;
   unsigned int padding_USI[9];
   double angle_step;
   double azi_start;
   double azi_stop;
   double elevation;
   double pulse_width_us;
   double radar_wave_length;
   double range;
   double range_start;
   double range_step;
   double scan_speed;
   double padding_F[6];
   char timestamp[MAX_STRING];
   char sizeZDR;
   char sizeRho;
   unsigned char long_pulse;
   unsigned char half_resolution;
   struct ray_header r_header[MAX_RAYS];
   };


struct scan_how_extended
   {
   unsigned int IIRFilter;
   unsigned int InterpolationMode;
   unsigned int AcfMode;
   unsigned int ClutterFilterMask;
   unsigned int FilterDelay;
   int CCORThreshold;
   int ClutterMicroSuppressionThreshold;
   int NOISEThreshold;
   int adc_hh_temperature;
   int adc_hl_temperature;
   int adc_vh_temperature;
   int adc_vl_temperature;
   int cpu_temp_0;
   int cpu_temp_1;
   int cpu_temp_2;
   int cpu_temp_3;
   int mb_temp_0;
   int mb_temp_1;
   int mb_temp_2;
   int mb_temp_3;
   int noisePowerH;
   int SIGPOWThreshold;
   int SpeckleRemoverMinClear;
   int SpeckleRemoverMinFill;
   int TaperWindow;
   double SQI1Threshold;
   double SQI2Threshold;
   double SecondTripSQIRatio;
   double SecondTripSQIThreshold;
   double dbz0h;
   double nyquist_velocity;
   double unambiguous_velocity;
   char SpeckleRemoverV[MAX_STRING];
   char SpeckleRemoverW[MAX_STRING];
   char SpeckleRemoverZ[MAX_STRING];
   char ThresholdUV[MAX_STRING];
   char ThresholdUW[MAX_STRING];
   char ThresholdUZ[MAX_STRING];
   char ThresholdV[MAX_STRING];
   char ThresholdW[MAX_STRING];
   char ThresholdZ[MAX_STRING];
   char unfolding_str[MAX_STRING];
   char SecondTripRemoval[MAX_STRING];
   char RangeNormalization[MAX_STRING];
   char ClutterMicroSuppression[MAX_STRING];
   char DataFormatStr[MAX_STRING];
   char GasAttenuationCorrection[MAX_STRING];
   unsigned int fan_status;
   };

struct scan_what
   {
   unsigned int descriptor_count;
   char product[MAX_STRING];
   char scan_type[MAX_STRING];
   };

struct moment_header
   {
   double dyn_range_max;
   double dyn_range_min;
   double padding_F[6];
   char format[MAX_STRING];
   char moment[MAX_STRING];
   char unit[MAX_STRING];
   };

struct data
   {
   struct moment_header header[MAX_VARS];
   unsigned char dBZ[MAX_RAYS][MAX_BINS];
   unsigned char dBuZ[MAX_RAYS][MAX_BINS];
   unsigned char V[MAX_RAYS][MAX_BINS];
   unsigned char W[MAX_RAYS][MAX_BINS];
   unsigned char ZDR[MAX_RAYS][MAX_BINS];
   unsigned char RhoHV[MAX_RAYS][MAX_BINS];
   unsigned short int PhiDP[MAX_RAYS][MAX_BINS];
   unsigned short int uPhiDP[MAX_RAYS][MAX_BINS];
   unsigned short int KDP[MAX_RAYS][MAX_BINS];
   unsigned short int ZDR_16[MAX_RAYS][MAX_BINS];
   unsigned short int RhoHV_16[MAX_RAYS][MAX_BINS];
   };

struct params_slice
   {
   /*number of rays for this slice*/
   unsigned int rays;
   /*number of bins for this slice*/
   unsigned int bins;
   /*dbz, dbuz, zdr, v, w, etc*/
   char var;
   /*record size: 8, 16 or 32*/
   char depth;
   };
int le_dados_blob(char *nome, int numele, struct volume_how *v_how,
                  struct scan_how s_how[], struct data dados[]);
   
char *retorna_atributos(xmlNode *node, char *texto);
char *retorna_valor(xmlNode *node, char *texto);
xmlNode *retorna_node(xmlNode *parent_node, char *texto);
int le_cabecalho_xml(char *buffer, struct volume_how *v_how,
                     struct volume_what *v_what, struct volume_where *v_where,
                     struct scan_how s_how[], struct scan_what s_what[],
                     struct data dados[]);

int retorna_ponteiro_arquivos(FILE *fin[], char *nome);
int write_hdf5(struct volume_how *v_how,
               struct volume_what *v_what, struct volume_where *v_where,
               struct scan_how s_how[], struct scan_what s_what[],
               struct data dados[]);
void return_value(char *source, char *result);
void return_info(char *line, char *pattern, char *result);

void write_attr_text(hid_t loc_id,char *name, char *value);
void write_attr_uint(hid_t loc_id,char *name, int value);
void write_attr_double(hid_t loc_id,char *name, double  value);
