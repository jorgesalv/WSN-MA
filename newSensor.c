#include "contiki.h"
#include "random.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "sys/energest.h"
#include "dev/dht22.h"
#include "lib/fs/fat/ff.h"
#include "dev/disk/mmc/mmc-arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

uint16_t wait = 60;
struct paketMsg{
  uint16_t emisor;
  uint16_t temperature;
  uint16_t timestamp;
  uint16_t numPkt;
};
uint8_t K = 2;
static struct etimer et;
struct paketMsg paquete;
uint16_t Told=5;
uint16_t Tnew;
uint16_t pktNumber = 1;
uint16_t randTemp = 1; 
int16_t temperaturesToSend[] = {14.882492,14.602492,14.432492,14.182492,14.1924925,13.982492,13.752492,13.4424925,13.372492,13.522492,13.782492,14.272492,14.6924925,14.542492,17.482492,18.002493,18.392492,17.732492,16.352493,16.182493,15.662492,15.392492,15.182492,14.792492,14.782492,14.902493,14.992492,14.452492,13.242492,12.722492,12.672492,12.682492,13.482492,14.632492,15.172492,16.222492,15.482492,17.332493,17.602493,17.662493,16.592493,16.342493,16.012493,16.272493,15.652493,15.472492,15.262492,15.032492,14.892492,14.952492,14.842492,15.252492,14.962492,15.002492,14.822492,14.622492,14.492492,14.472492,14.572492,14.712492,15.242492,14.952492,15.902493,16.202492,16.342493,16.012493,16.182493,15.842492,15.732492,15.542492,15.662492,15.622492,15.632492,15.392492,15.682492,15.682492,15.352492,15.172492,15.012492,14.982492,15.262492,16.952494,18.222492,19.022493,18.312492,17.602493,20.202494,19.822493,19.642492,18.812492,18.082493,16.762493,16.032492,15.602492,15.472492,15.4424925,15.522492,15.662492,15.772492,15.702492,15.4424925,15.122492,14.922492,14.792492,14.772492,15.112493,15.462492,15.512492,15.1924925,15.102492,15.152493,14.882492,15.092492,14.632492,14.382492,14.092492,13.922492,14.102492,14.322492,14.312492,14.262492,14.182492,14.282492,14.072492,13.832492,13.932492,13.892492,13.772492,14.372492,15.952492,16.022493,16.612494,17.472492,17.922493,17.132494,17.242493,16.822493,16.642492,15.932492,15.432492,14.772492,14.292492,14.282492,14.432492,13.572492,13.432492,13.172492,12.972492,12.962492,12.912492,12.842492,12.622492,13.272492,15.912492,18.852493,19.902493,20.632494,20.772493,20.922493,21.272493,21.122494,20.972492,20.472492,19.612494,18.622494,17.322493,16.492493,15.932492,15.462492,14.742492,14.1924925,13.552492,13.462492,13.732492,13.852492,13.712492,14.162492,16.432493,18.902493,19.812492,20.132494,20.582493,20.412493,20.372494,20.172493,19.972492,19.592493,18.762493,17.702494,16.612494,16.222492,15.872492,15.602492,15.332492,15.172492,15.052492,14.902493,14.562492,14.282492,13.852492,14.132492,15.782492,16.872494,17.542494,17.262493,16.472492,18.862494,18.952494,18.902493,18.652493,18.322493,17.892492,17.312492,16.652493,16.372492,16.082493,15.722492,15.342492,15.062492,15.072492,14.862493,14.662492,14.632492,14.222492,14.592492,16.332493,17.882494,19.002493,19.352493,19.392492,19.752493,19.952494,19.782494,19.522493,19.362494,18.842493,18.042494,17.112494,16.702494,16.252493,15.792492,15.372492,14.782492,14.842492,15.062492,15.052492,15.112493,14.772492,15.332492,16.732492,17.682493,18.782494,18.992493,18.462494,19.682493,19.092493,18.972492,19.432493,20.062492,19.652493,19.172493,18.532494,17.722492,17.642492,17.212494,16.932493,16.372492,15.972492,15.552492,15.132492,14.172492,13.502492,14.362493,16.212492,17.192493,16.062492,16.822493,17.472492,18.642492,20.452494,21.742493,23.062492,23.392492,21.092493,18.462494,17.482492,16.822493,16.182493,15.732492,15.232492,15.042492,14.6924925,14.952492,15.232492,14.912492,14.922492,15.422492,15.882492,16.722492,17.402493,18.162493,18.882494,19.422493,20.342493,20.902493,20.562492,20.212494,19.652493,18.552492,17.472492,17.032494,17.062492,17.142492,16.422493,15.662492,15.382492,15.382492,15.242492,15.362493,15.322492,16.622494,18.502493,20.622494,20.932493,22.662493,23.332493,24.152493,24.732492,24.592493,24.832493,24.862494,24.532494,19.762493,18.272493,17.642492,17.152493};
/*---------------------------------------------------------------------------*/
//Energest method
static unsigned long to_seconds(uint64_t time){
  return (unsigned long)(time / CLOCK_SECOND);
}
/*---------------------------------------------------------------------------*/
//PROCESS(example_unicast_process, "ZOLERTIA SENSOR");
//AUTOSTART_PROCESSES(&example_unicast_process);
PROCESS(example_broadcast_process, "ZOLERTIA SENSOR");
AUTOSTART_PROCESSES(&example_broadcast_process);
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

/* Write new data from sensors in the microSD */
void write_data_sd(uint8_t msg)
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
  printf("[SD] Writing %u\n",msg);
  f_printf(&fil,"%u", msg);
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
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from){
  char *rcvPkt = (char *)packetbuf_dataptr();
  
  printf("[ZOLERTIA SENSOR] Message received from %d.%d:  Decision: %s \n",from->u8[0], from->u8[1],rcvPkt);
  //Check what is the response and what to do with the transmision time
  if( strcmp(rcvPkt,"1") == 0){
    write_serial_sd("[ZOLERTIA SENSOR] Decision: 1");
    if(wait != 600){
      write_serial_sd("[ZOLERTIA SENSOR] Increasing T");
      wait = wait + 60;
    }
  }
  if( strcmp(rcvPkt,"2") == 0){ 
    write_serial_sd("[ZOLERTIA SENSOR] Decision: 2");
      if(wait != 60){
        write_serial_sd("[ZOLERTIA SENSOR] Reducing T");
      wait = wait - 60;
    }
  }

 printf("[ZOLERTIA SENSOR] Current waiting TtimeT: # %d \n",wait);
 write_serial_sd("[ZOLERTIA SENSOR] Current waiting TtimeT: \n"+wait);
}

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from){ 
  char *rcvPkt = (char *)packetbuf_dataptr();
  
  printf("[ZOLERTIA SENSOR] Message received from %d.%d:  Decision: %s \n",from->u8[0], from->u8[1],rcvPkt);
  
  //Check what is the response and what to do with the transmision time
  if( strcmp(rcvPkt,"1") == 0){
    write_serial_sd("[ZOLERTIA SENSOR] Decision: 1");
    if(wait != 600){
      write_serial_sd("[ZOLERTIA SENSOR] Increasing T");
      wait = wait + 60;
    }
  }
  if( strcmp(rcvPkt,"2") == 0){ 
    write_serial_sd("[ZOLERTIA SENSOR] Decision: 2");
      if(wait != 60){
        write_serial_sd("[ZOLERTIA SENSOR] Reducing T");
      wait = wait - 60;
    }
  }
  printf("[ZOLERTIA SENSOR] Current waiting TtimeT: # %d \n",wait);
  write_serial_sd("[ZOLERTIA SENSOR] Current waiting TtimeT:  \n"+wait);
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
int16_t temperature, humidity;	
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(/*example_unicast_process, ev, data*/example_broadcast_process, ev, data){
  SENSORS_ACTIVATE(dht22);
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
  //PROCESS_EXITHANDLER(unicast_close(&uc);)
  PROCESS_BEGIN();
  broadcast_open(&broadcast, 129, &broadcast_call);
  unicast_open(&uc, 146, &unicast_callbacks);  
  
  linkaddr_t addr;
  init_sd();
  while(1) {
    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      printf("[DHT22] Temperature %02d.%02d ºC, ", temperature / 10, temperature % 10);
      printf("Humidity %02d.%02d RH\n", humidity / 10, humidity % 10);
    }else{
        printf("[DHT22] Failed to read the sensor\n");
    }
    etimer_set(&et, CLOCK_SECOND*wait);//*wait
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
   
    paquete.emisor = 0;
   paquete.temperature = temperature / 10, temperature % 10;//temperaturesToSend[pktNumber];// randTemp;
   paquete.timestamp = wait;
   paquete.numPkt = pktNumber;
   randTemp = rand()%10 + 20;
   packetbuf_copyfrom(&paquete, sizeof(paquete ));
   addr.u8[0] = 1;
   addr.u8[1] = 0;

   if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
     //unicast_send(&uc, &addr);
      broadcast_send(&broadcast);
     //Send packet 
     printf("**** SENSOR START SEND****\n");
     write_serial_sd("**** SENSOR START SEND****\n");
     printf("[ZOLERTIA SENSOR] Current Time: %d\n",wait);
     write_serial_sd("[ZOLERTIA SENSOR] Current Time: \n"+wait);
     printf("[ZOLERTIA SENSOR] Message sent to: %d.%d\n", addr.u8[0],addr.u8[1]);
     write_serial_sd("[ZOLERTIA SENSOR] Message sent to: \n"+addr.u8[0]+addr.u8[1]);
     printf("[ZOLERTIA SENSOR] Temperature: %02d.%02d ºC \n",temperature / 10, temperature % 10);
     write_serial_sd("[ZOLERTIA SENSOR] Temperature: ºC \n" +temperature / 10 +temperature % 10);
     printf("[ZOLERTIA SENSOR] Paquet Number: %d \n", paquete.numPkt);
     write_serial_sd("[ZOLERTIA SENSOR] Paquet Number:  \n"+paquete.numPkt);
     //printf("**** SENSOR SENDING %02d.%02d,  Pkt:%d, Wait: %d \n",temperature / 10, temperature % 10,paquete.numPkt,wait/60);
     //write_serial_sd("**** SENSOR SENDING %02d.%02d,  Pkt:%d, Wait: %d \n"temperature / 10, temperature % 10paquete.numPktwait/60);
     printf("**** SENSOR END SEND****\n\n");
     write_serial_sd("**** SENSOR END SEND****\n\n");
     //Told=Tnew;temp/10,temp-(temp/10)*10
     pktNumber +=1;
   }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

