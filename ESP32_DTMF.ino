#include <driver/dac.h>

#define SECONDS_2_MICRO 1000000

// coleccion de tonos con frecuencias f1 y f2
//{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10(*), 11(#), 12(A), 13(B), 14(C), 15(D)}
const int tones[12][2] = {
  {941,1336}, // 0
  {697,1209},{697,1336},{697,1477}, // 1, 2, 3
  {770,1209},{770,1336},{770,1477}, // 4, 5, 6
  {852,1209},{852,1336},{852,1477}, // 7, 8, 9 
  {941,1209},{941,1477} // *, #
};

// lista contiene el mensaje a enviar
const int length = 10;
// * 2 0 1 9 0 9 2 8 #
const int message[length] = {
  10,2,0,1,9,0,9,2,8,11
};

// periodo de muestreo
const float sampling_period = 10e-06;

// variables de modulacion FM
float Vm = 10;
float Df = 0.01;
float Ac = 0.5;

// variables de programa
float lastTime_sample;
float samplingTime;
double acum;

volatile int counter;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  if(counter >= length - 1) {
    counter = 0;
  } else {
    counter++;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  // monitor serial 
  Serial.begin(115200);
  while(!Serial){}
  delay(500);

  dac_output_enable(DAC_CHANNEL_1);
  dac_output_enable(DAC_CHANNEL_2);

  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer,&onTimer,true);
  timerAlarmWrite(timer,1000000,true);
  timerAlarmEnable(timer); 
  delay(500);
}

void loop() {
  float t_sample = micros() - lastTime_sample;

  int t = message[counter];
  int f1, f2;
  f1 = tones[t][0];
  f2 = tones[t][1];
  
  // muestreo de senal
  if(t_sample > sampling_period*SECONDS_2_MICRO) {

    // senal mensaje
    double m = Vm*(sin(2*PI*f1*samplingTime)
        +sin(2*PI*f2*samplingTime));
    //double m = Vm*sin(2*PI*100.0*samplingTime);
    
    // integrador discreto
    acum += m;

    // parte x(t) y y(t)
    double x = Ac*(cos(Df*acum)+1);
    double y = Ac*(sin(Df*acum)+1);

    int x_val = x*255;
    int y_val = y*255;

    dac_output_voltage(DAC_CHANNEL_1, x_val);
    dac_output_voltage(DAC_CHANNEL_2, y_val);

    /*Serial.print(x_val);
    Serial.printf(",");
    Serial.println(y_val);

    Serial.print(f1);
    Serial.printf(",");
    Serial.println(f2);*/
    
    samplingTime += sampling_period;
    lastTime_sample = micros();
  }
}
