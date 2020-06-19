#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
//#include "dev/dht22.h"
#include "lib/fs/fat/ff.h"
#include "dev/disk/mmc/mmc-arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sys/energest.h"

//#if LOG_SD
#define LOG_FILE        "log.csv"
#define ROUTES_FILE       "routes.csv"
#define CONFIG_FILE       "config.csv"
//#endif

/* microSD card */
//#if LOG_SD
static FATFS FatFs;             // Work area (file system object) for logical drive
FIL fil, fil2, fil3;                  // File object
char line[82];                  // Line buffer
FRESULT fr;                     // FatFs return code
//#endif

struct paketMsg{
  char emisor;
  uint16_t temperature;
  uint16_t timestamp;
  uint16_t numPkt;
};
uint16_t numberOfPredictions;
uint16_t currWait;
uint16_t W = 5;
uint32_t temperaturaSumatory;
uint16_t tempArray[5];
uint16_t packetNumber = 1;
uint16_t packetNumberHelper = 0;
uint32_t time;
uint16_t alpha = 1;
uint16_t betha = 3;
uint16_t predictionValidation;
linkaddr_t addrToRespond;
/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "GATEWAY");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
//SD CARD METHODS
/* micro SD initialization */
static void init_sd(){
  /* Register work area to the default drive */
  f_mount(&FatFs, "", 0);

  /* Create the log file */
  fr = f_open(&fil, LOG_FILE, FA_WRITE | FA_OPEN_ALWAYS);
  if(fr){
    printf("[SD] microSD card could not be initialized -- f_open() error: %d\n", fr);
    if (fr == 17){
      printf("[SD] Not enough memory for the operation. Reduce internal arrays\n");
    }
  }
  else{
    printf("[SD] microSD OK \n");
  }

  f_lseek(&fil, f_size(&fil));
  f_printf(&fil, "//---------------------------------\n");
  f_printf(&fil, "//--LOG FILE FROM LAST ACTIVATION--\n");
  f_printf(&fil, "//---------------------------------\n");

  /* Close the log file */
  f_close(&fil);
}
void write_serial_sd(char *string){
  leds_off(LEDS_ALL);
  leds_on(LEDS_RED);

  /* Open the log file */
  fr = f_open(&fil, LOG_FILE, FA_WRITE);
  if(fr){
    //printf("[SD] microSD card could not be initialized\n");
  }
  else{
    //printf("[SD] Writing new EVENT in log file\n");
  }

  f_lseek(&fil, f_size(&fil));
  f_printf(&fil, "%s\n", string);

  /* Close the log file */
  f_close(&fil);
}
/* Write new data from sensors in the microSD */
void write_data_sd(char msg)
{
  uint8_t res, node_id;

  /* Open the log file */
  fr = f_open(&fil, LOG_FILE, FA_WRITE);
  if(fr){
    printf("[SD] microSD card could not be initialized\n");
  }
  else{
    printf("[SD] Writing new DATA in log file\n");
  }

  f_lseek(&fil, f_size(&fil));

  /* Write data */
  printf("[SD] Writing %s\n",msg);
  f_printf(&fil,"%s", msg);
  f_close(&fil);
  
}

/*Method to define if the prediction is good or bad*/
static void checkPrediction(uint32_t temperature, uint32_t prediction){
  int calculateDif = temperature - prediction;
  if(calculateDif <= alpha){
   predictionValidation = 1;
  }else if(calculateDif > alpha && calculateDif <= betha ){
   predictionValidation = 0;
  }else{
   predictionValidation = 2;
  }
}
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from){

  static struct paketMsg *rcvPkt;
  rcvPkt = packetbuf_dataptr();
  printf("**** GATEWAY START RECEIVE****\n");
  printf("[ZOLERTIA GW] Message received from: %d.%d\n",from->u8[0], from->u8[1]);
  printf("[ZOLERTIA GW] Temperature: %02d.%02d ºC\n",(rcvPkt->temperature), (rcvPkt->temperature) );
  printf("[ZOLERTIA GW] Paquet Number: %d\n",rcvPkt->numPkt);
  
  /*write_serial_sd("[ZOLERTIA GW] Sensor Period Time: %d\n"+rcvPkt->timestamp/60);
  write_serial_sd("**** GATEWAY START RECEIVE****\n");
  write_serial_sd("[ZOLERTIA GW] Temperature: %02d.%02d ºC\n"+(rcvPkt->temperature)+ (rcvPkt->temperature) );
  write_serial_sd("[ZOLERTIA GW] Paquet Number: %d\n"+rcvPkt->numPkt);
  write_serial_sd("[ZOLERTIA GW] Sensor Period Time: %d\n"+rcvPkt->timestamp/60);*/
  currWait = rcvPkt->timestamp;
  packetNumberHelper += 1;
  
  if(packetNumberHelper == W){
    printf("---reseting pcktNhelper\n");
    write_serial_sd("---reseting pcktNhelper\n");
    packetNumberHelper = 0;
  }
 printf("---adding to buffer %d\n",rcvPkt->temperature);  
 write_serial_sd("---adding to buffer %d\n"+rcvPkt->temperature);  
 uint16_t currTemp = rcvPkt->temperature;
  tempArray[packetNumberHelper] =(rcvPkt->temperature); 
  linkaddr_t receiverAddr;
  addrToRespond.u8[0] = from->u8[0];
  addrToRespond.u8[1] = from->u8[1];
 printf("---calculating avergafe of %d,%d,%d,%d,%d\n",tempArray[0] + tempArray[1] + tempArray[2]+ tempArray[3]+ tempArray[4]);  
    temperaturaSumatory = tempArray[0] + tempArray[1] + tempArray[2]+ tempArray[3]+ tempArray[4];
    printf("---Sumatory %d\n",temperaturaSumatory);
    printf("---Current Estimation  Avg %d\n",temperaturaSumatory/W);
    write_serial_sd("---Sumatory %d\n"+temperaturaSumatory);
    write_serial_sd("---Current Estimation  Avg %d\n"+temperaturaSumatory/W);
    
    checkPrediction(currTemp,temperaturaSumatory/W);

}
//static void recv_uc(struct unicast_conn *c, const linkaddr_t *from){
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){
  
  static struct paketMsg *rcvPkt;
  rcvPkt = packetbuf_dataptr();
  printf("**** GATEWAY START RECEIVE****\n");
  printf("[ZOLERTIA GW] Message received from: %d.%d\n",from->u8[0], from->u8[1]);
  printf("[ZOLERTIA GW] Temperature: %02d.%02d ºC\n",(rcvPkt->temperature), (rcvPkt->temperature) );
  printf("[ZOLERTIA GW] Paquet Number: %d\n",rcvPkt->numPkt);
  
  write_serial_sd("[ZOLERTIA GW] Sensor Period Time: %d\n"+rcvPkt->timestamp/60);
  write_serial_sd("**** GATEWAY START RECEIVE****\n");
  write_serial_sd("[ZOLERTIA GW] Message received from: %d.%d\n"+from->u8[0]+ from->u8[1]);
  write_serial_sd("[ZOLERTIA GW] Temperature: %02d.%02d ºC\n"+(rcvPkt->temperature)+ (rcvPkt->temperature) );
  write_serial_sd("[ZOLERTIA GW] Paquet Number: %d\n"+rcvPkt->numPkt);
  write_serial_sd("[ZOLERTIA GW] Sensor Period Time: %d\n"+rcvPkt->timestamp/60);
  currWait = rcvPkt->timestamp;
  packetNumberHelper += 1;
  
  if(packetNumberHelper == W){
    printf("---reseting pcktNhelper\n");
    write_serial_sd("---reseting pcktNhelper\n");
    packetNumberHelper = 0;
  }
 printf("---adding to buffer %d\n",rcvPkt->temperature);  
 write_serial_sd("---adding to buffer %d\n"+rcvPkt->temperature);  
 uint16_t currTemp = rcvPkt->temperature;
  tempArray[packetNumberHelper] =(rcvPkt->temperature); 
  linkaddr_t receiverAddr;
  addrToRespond.u8[0] = from->u8[0];
  addrToRespond.u8[1] = from->u8[1];
 printf("---calculating avergafe of %d,%d,%d,%d,%d\n",tempArray[0] + tempArray[1] + tempArray[2]+ tempArray[3]+ tempArray[4]);  
    temperaturaSumatory = tempArray[0] + tempArray[1] + tempArray[2]+ tempArray[3]+ tempArray[4];
    printf("---Sumatory %d\n",temperaturaSumatory);
    printf("---Current Estimation  Avg %d\n",temperaturaSumatory/W);
    write_serial_sd("---Sumatory %d\n"+temperaturaSumatory);
    write_serial_sd("---Current Estimation  Avg %d\n"+temperaturaSumatory/W);
    
    checkPrediction(currTemp,temperaturaSumatory/W);

}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
static struct paketMsg paquete;
int16_t temperature, humidity;
linkaddr_t addr,myaddr;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data){
  //uint8_t r = rand()%2;
  uint8_t longaddr[8];
 
  PROCESS_EXITHANDLER(unicast_close(&uc);)
  PROCESS_BEGIN();
  unicast_open(&uc, 146, &unicast_callbacks);
  broadcast_open(&broadcast, 129, &broadcast_call);
  leds_toggle(LEDS_ALL);
  myaddr.u8[0] = 1;
  myaddr.u8[1] = 0;
  linkaddr_set_node_addr(&myaddr);
  memset(longaddr, 0, sizeof(myaddr));
  linkaddr_copy((linkaddr_t *) &longaddr, &linkaddr_node_addr);
  init_sd();
  printf("[ZOLERTIA GW] Starting...\n[ZOLERTIA GW] RIME address set: %d.%d\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
  write_serial_sd("[ZOLERTIA GW] Starting...\n");
  while(1) {
    
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND*60);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    leds_blink();
    //Put the information defined on the receive functio on the response packet
    addr.u8[0] = addrToRespond.u8[0];
    addr.u8[1] = addrToRespond.u8[1];

    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)){
      if(packetNumber >= W){
        printf("---Curr Packet %d\n",packetNumber);
        printf("---Temp Sum %d\n",temperaturaSumatory);
        write_serial_sd("---Curr Packet %d\n"+packetNumber);
        write_serial_sd("---Temp Sum %d\n"+temperaturaSumatory);
        if( predictionValidation == 1){
          printf("---Curr Time %d\n",currWait);
          printf("---New Time %d\n",currWait+60);
          printf("---Predicts %d\n",(currWait+60)/60);
          write_serial_sd("---Curr Time %d\n"+currWait);
          write_serial_sd("---New Time %d\n"+currWait+60);
          write_serial_sd("---Predicts %d\n"+(currWait+60)/60);
          numberOfPredictions = (currWait+60)/60;
        }
        if(numberOfPredictions > 0){
          packetNumberHelper += 1;
          tempArray[packetNumberHelper] = temperaturaSumatory/W;
          numberOfPredictions = numberOfPredictions - 1 ;
          printf("---adding to buffer prediction %d\n",temperaturaSumatory/W);
          write_serial_sd("---adding to buffer prediction %d\n"+temperaturaSumatory/W);
          printf("&& GW PREDICT %d, PKT: %d\n",temperaturaSumatory/W,packetNumber);
          write_serial_sd("&& GW PREDICT %d, PKT: %d\n"+temperaturaSumatory/W+packetNumber);
        }
        printf("---Check Validity %d\n",predictionValidation);

        write_serial_sd("---Check Validity %d\n"+predictionValidation);
      }
      if(packetNumberHelper == W){
        printf("---reseting pcktNhelper\n");
        write_serial_sd("---reseting pcktNhelper\n");
        packetNumberHelper = 0;
      }
      packetNumber += 1;
      printf("**** GATWAY END RECEIVE****\n\n");
      write_serial_sd("**** GATWAY END RECEIVE****\n\n");
      //write_data_sd();
      if(predictionValidation == 1){
        packetbuf_copyfrom("1", 2);  
      }else if(predictionValidation == 2){
        packetbuf_copyfrom("2", 2);
      }else{
        packetbuf_copyfrom("0", 2);
      }
      //packetbuf_copyfrom("t", 2  );
      if(packetNumber >= W){
        unicast_send(&uc, &addr);
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

