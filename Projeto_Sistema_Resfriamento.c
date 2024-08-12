#include <18F4550.h>
#fuses HS, CPUDIV1,PLL5,USBDIV
#device adc=10
#use delay(clock=20Mhz)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)

//Inicio configuracao LCD
#define LCD_DB0   PIN_D0
#define LCD_DB1   PIN_D1
#define LCD_DB2   PIN_D2
#define LCD_DB3   PIN_D3
#define LCD_DB4   PIN_D4
#define LCD_DB5   PIN_D5
#define LCD_DB6   PIN_D6
#define LCD_DB7   PIN_D7
#define LCD_E     PIN_E1
#define LCD_RS    PIN_E2
#include <flex_lcd2.c>
//Fim das definicoes para LCD

//Variaveis Gerais
boolean flag;
int contador;
//Fim das variaveis gerais

int LM(){
   
   unsigned int32 Sensor_LM, Conversor;

   Sensor_LM = read_adc();
   delay_ms(10);
   //Conversor = Sensor_LM*0.488758; //igual a (5/1023)*100
   // Por questoes de conversao, sera somente de 0C a 100C
   Conversor = Sensor_LM*0.09775171; //igual a (1/1023)*100 = 0.09775171
   // Se esse projeto fosse usado realmente, o valor de conversao DEVE ser: 0.488758
   
   return Conversor;
}

int Controle_Velocidade(unsigned int32 graus){

   static int speed;
   
   if(input(pin_B0) == 0){
      flag = !flag;
      delay_ms(200);
   }

// ajuste de velocidade do pwm (Cooler)---------------------------------------//
   if (flag == 0){
   // if a temperatura for menor de 50C, o cooler desliga
      if(graus < 50){speed = 0;}
   // if a temperatura for maior de 50C, o cooler liga
      if(graus >= 50){speed = ((graus-50)*2);} //O aumento da velocidade a 2%
                                               //ao chegar a 100C, a velocidade
                                               //sera 100%
   }
//---------------------------------------------------------------------------//
   
   
// Configuracao manual da velocidade ----------------------------------------//
   if(flag == 1){
      lcd_gotoxy(1,2);
      printf(lcd_putc,"  Vel. Manual   "); 
      printf(lcd_putc," Vel. Fan: %i%%  ", speed);
      
      // Comunicacao serial ----------------------//
      if (contador == 255){
      printf("#        Modo Manual ativado!        #\n");
      printf("#------------------------------------#\n");
      printf("# Velocidade Cooler: %i%%  ", speed);             
      printf("######################################\n");
      }
      //------------------------------------------//
     
      if(input(pin_B1) == 0){
      speed = speed + 10;
         if (speed > 100){speed = 0;}
      delay_ms(200);
      }
   }
//---------------------------------------------------------------------------//
   
   speed = speed*2.5; // Converte a velocidade em velocidade de pwm
   set_pwm1_duty(speed); //aqui e mandado a velocidade do pwm para o pwm1.
   speed = speed/2.5; // e necessario voltar para velocidade normal para printar
   
   return speed;
}


void main(){
   port_b_pullups(true);   // Liga os pull ups
   output_d(0b00000000);   // Desliga todas as portas D
   SET_TRIS_B(0xFF);       // Seta o grupo B como entrada.
   SET_TRIS_D(0x00);       // Seta o grupo D como saida.
   
   lcd_init();
   lcd_gotoxy(1,3);
   printf(lcd_putc,"\t  Sistema de\t");
   printf(lcd_putc,"\t\t  Resfriamento");
   printf("######################################\n");
   printf("# Sistema de Resfriamento automatico #\n");
   printf("######################################\n");
   printf("#        Iniciando o sistema...      #\n");
   delay_ms(2000);
   printf("#------------------------------------#\n");
   
   setup_adc_ports(an0);
   setup_adc(adc_clock_internal);
   set_adc_channel(0);
   
   /////////////////////////////   
   setup_ccp1(CCP_PWM); //habilita o uso do PWM
   setup_timer_2(T2_DIV_BY_4, 249, 1); //timer do pwm
   /////////////////////////////
   
   float Graus;
   int velocidade = 0;
   
   while(true){
      
      Graus = LM();
      velocidade = Controle_Velocidade(Graus);
            
      if(flag == 0){
      lcd_gotoxy(1,1);
      printf(lcd_putc," Temp.:  %.0f %cC ", Graus, 0xdf);
      printf(lcd_putc,"  Vel. Fan:  %i%%  ", velocidade);
      
//--- Inicio da comunicacao serial ------------------------------------------//
      if (contador == 255){
         printf("# Temperatura: %f \n", Graus); // Serial
         printf("#------------------------------------#\n");
         
         printf("# Velocidade cooler: %i%%  \n", velocidade); // Comunicacao serial      
         printf("######################################\n");
         
         contador = 0;
      }
//--- Fim da comunicacao serial ---------------------------------------------//
      }
      
      contador++;
   }
}
