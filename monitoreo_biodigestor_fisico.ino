// Prototipo fisico
// Sistema de monitoreo de biodigestor
// Equipo G

// Librerias
#include <Wire.h> // Libreria para comunicacion I2C
#include <LiquidCrystal_I2C.h> // Libreria para LCD

// Definir pines
#define LMPin A0 // Asignar pin A0 a sensor de temperatura
#define MQPin A1 // Asignar pin A1 a sensor de gas
const int echoPin = 12; // Pin Echo del sensor ultrasonico
const int pingPin = 13; // Pin Trigger del sensor ultrasonico
// Cableado: SDA conectado a A4 y SCL a A5.
// Conectar LCD via I2C, direccion default 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change to (0x27,20,4) for 20x4 LCD.
const int ledR = 2; // Pin del actuador ledR
const int ledV = 3; // Pin del actuador ledR
// Pantalla 7 segmentos
#define a 5
#define b 6
#define c 7
#define d 8
#define e 9
#define f 10
#define g 11

//#define d1 A0 // No hay espacio
//#define d2 A1 // No hay espacio
#define d1 A2
#define d2 A3

//Constantes
float MCurve[3] = {2.3,0.21,-0.47};
float Ro = 10;
#define GASMet 0
#define RAL 9.83 // Resistencia del mq2 en el aire limpio
#define RL_V 5 // Resistencia que mide la carga del mq-2 en kOhms
long n = 0;
int x = 100;
int count = 0;

void setup() {
  Serial.begin(9600); // Abre el puerto serial a 9600 bps
  // Inicializacion de LCD
  lcd.init();
  lcd.backlight();
  // Sensor ultrasonico
  pinMode(echoPin, INPUT); 
  pinMode(pingPin, OUTPUT);
  // Sensor de gas
  Serial.print("Calibrando MQ-2...\n");
  //Ro = Calibracion();
  Serial.print("Calibracion finalizada...\n");
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
  // LEDs
  pinMode(ledR, OUTPUT);
  pinMode(ledV, OUTPUT);
  // Display 7 segmentos
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(d1, OUTPUT);
  pinMode(d2, OUTPUT);

}

void loop() {
  // ------- Sensor Ultrasonico & LED Rojo-------- //
  String dist = "";
  long duracion, distCM;
  // Enviar ping o sonido
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  // Usar funcion pulseIn para calcular el tiempo en que llega el eco
  duracion = pulseIn(echoPin, HIGH); 
  // Convierte el tiempo a cm
  distCM = microsegACm(duracion); 
  // Imprimir la distancia en LCD
  lcd.setCursor(0,0);
  dist += "Dist=";
  dist += distCM;
  lcd.print(dist);
  // Al alcanza cierta distancia, enciende el LED Rojo
  if(distCM <= 10){
    digitalWrite(ledR, HIGH);
  }
  else{
    digitalWrite(ledR, LOW);
  }

  // ------- Sensor de Temperatura y Display -------- //
  String tp = "";
  float temp = analogRead(LMPin); // Leer valor del sensor
  temp = ((temp*500)/1024)-180; // Calcular temperatura en °C
  lcd.setCursor(0, 1); // fila 0, col 1
  // Mostrar en LCD
  tp += "Temperatura=";
  tp += temp;
  lcd.print(tp);

  // ------- Sensor MQ2 y LED Verde -------- //
  float gas = analogRead(MQPin); // Leer valor del sensor
  lcd.setCursor(8, 0); // fila 0, col 0
  // Mostrar en LCD
  lcd.print("CH4=");
  //lcd.print(porcentaje_gas(lecturaMQ()/Ro,GASMet));
  lcd.print(gas);
  lcd.print( "ppm" );
  lcd.print("\n");
  // Al alcanza cierto nivel de gas, el LED verde enciende
  if(gas > 500){
    digitalWrite(ledV, HIGH);
  }
  else{
    digitalWrite(ledV, LOW);
  }
  delay(1000);
  lcd.clear();

  // ------- Display de 4 digitos 7 segmentos-------- //
  // Indica el tiempo que tardara el biol
  // Fermentacion Psicrofilica
  if (temp >= 4 && temp < 25){
    clearLEDs();
    digitalWrite(d1, LOW);
    digitalWrite(d2, LOW);
    numero(9);
  }
  // Fermentacion Mesofilica
  if (temp >= 25 && temp < 45){
    clearLEDs();
    digitalWrite(d1, LOW);
    digitalWrite(d2, LOW);
    numero(4);
  }
    // Fermentacion Termofilica
  if(temp >= 45 && temp <= 80){
    clearLEDs();
    digitalWrite(d1, LOW);
    digitalWrite(d2, LOW);
    numero(1);
  } 
}

// Metodo para convertir microseg a cm
long microsegACm(long microseg) {
   return microseg / 29 / 2;
}

// Calcular resistencia MQ-2
float calc_res(int raw_adc){
  return ( ((float)RL_V*(1023-raw_adc)/raw_adc));
}

// Calibracion del MQ-2
float Calibracion(){
  int i;
  float val=0;
    for (i=0;i<50;i++) {                                                                               //tomar múltiples muestras
    val += calc_res(analogRead(MQPin));
    delay(500);
  }
  val = val/50;                                                                                         //calcular el valor medio
  val = val/RAL;
  return val;
}

// Porcentaje de gas metano
int porcentaje_gas(float rs_ro_ratio, int gas_id){
   if ( gas_id == GASMet ) {
     return porcentaje_gas(rs_ro_ratio,MCurve);
   }
  return 0;
}

int porcentaje_gas(float rs_ro_ratio, float *pcurve){
  return (pow(10, (((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

// Lectura MQ
float lecturaMQ(){
  int i;
  float rs=0;
  for (i=0;i<5;i++) {
    rs += calc_res(analogRead(MQPin));
    delay(50);
  }
rs = rs/5;
return rs;
}

// Limpiar pantalla 4x7
void clearLEDs() {
  digitalWrite(a, LOW);
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

// Nuemros pantalla 4x7
void uno() {
  digitalWrite(a, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

void dos() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, LOW);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, LOW);
  digitalWrite(g, HIGH);
}

void tres() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, HIGH);
}

void cuatro() {
  digitalWrite(a, LOW);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void cinco() {
  digitalWrite(a, HIGH);
  digitalWrite(b, LOW);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, LOW);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void seis() {
  digitalWrite(a, HIGH);
  digitalWrite(b, LOW);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void siete() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

void ocho() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void nueve() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, LOW);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void zero() {
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, LOW);
}

// Numero 
void numero(int x) {
  switch (x) {
    default: zero();   break;
    case 1: uno();     break;
    case 2: dos();     break;
    case 3: tres();  break;
    case 4: cuatro();    break;
    case 5: cinco();     break;
    case 6: seis();       break;
    case 7: siete(); break;
    case 8: ocho();  break;
    case 9: nueve();   break;
  }
}

// Digito
void digito(int x) {
  digitalWrite(d1, HIGH);
  digitalWrite(d2, HIGH);
  switch (x) {
    case 1: digitalWrite(d2, LOW);  break;
    default: digitalWrite(d1, LOW); break;
  }
}

void add()
{
  // Toggle LED
  count ++;
  if(count == 10)
  {
    count = 0;
    n++;
    if(n == 10000)
    {
      n = 0;
    }
  }
}