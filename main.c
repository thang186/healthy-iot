#include <main.h>
#include <16f877a.h>

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#Fuses HS

#define LCD_ENABLE_PIN  PIN_D2                                    ////
#define LCD_RS_PIN      PIN_D0                                    ////
#define LCD_RW_PIN      PIN_D1                                    ////
#define LCD_DATA4       PIN_D4                                    ////
#define LCD_DATA5       PIN_D5                                    ////
#define LCD_DATA6       PIN_D6                                    ////
#define LCD_DATA7       PIN_D7                                    ////

#include<lcd.c>

#define     adj         pin_b0

int32    t, adc, pt,duty,tt_adj;

void ktr_adj()
{
   if(input(adj)==0)
   {
      delay_ms(20);
      if(input(adj)==0)
      {
         tt_adj=tt_adj+10;       // 10% - 100%        pr2 = 1.2ms/((1/8M)*4*16)-1 = 149    
         if(tt_adj>99)        tt_adj=10;
         duty = tt_adj*6;   // 60 -  600                        //duty = 1.2ms/((1/8M)*4) = 600
         
         while(input(adj)==0);
      }
   }
}


void doc_lm35()
{
   set_adc_channel(0);
   t=read_adc();
   t=t/2.046;
}

void doc_adc()
{
   set_adc_channel(1);
   adc=read_adc();
   adc=adc/2.046;             // adc = 0 - 500        // 0 -100
   pt = adc/5;
}

void main()
{
   set_tris_a(0xff);        
   set_tris_d(0x00);
   set_tris_c(0x00);
   set_tris_b(0xff);
   set_tris_e(0xff);
   lcd_init();
   
   setup_timer_2(t2_div_by_16,149,1);
   setup_ccp1(ccp_off);
   setup_ccp2(ccp_off);
   output_low(pin_c2);
  
   
   setup_adc(adc_clock_div_2);
   
   
   
   tt_adj=10;
   duty = tt_adj*6;  
   set_pwm1_duty(duty);
   while(true)
   {
      ktr_adj();
      doc_lm35();
      doc_adc();
      
      lcd_gotoxy(1,1);
      lcd_putc("T=");
      lcd_putc(t/100+0x30);
      lcd_putc(t/10%10+0x30);
      lcd_putc(t%10+0x30);
      lcd_putc(0xdf);// do C
      lcd_putc("C");
      
      lcd_gotoxy(9,1);
      lcd_putc("PT=");
      lcd_putc(pt/100+0x30);
      lcd_putc(pt/10%10+0x30);
      lcd_putc(pt%10+0x30);
      lcd_putc(0xdf);
      lcd_putc("C");
      
      lcd_gotoxy(9,2);
      lcd_putc("Duty=");
      lcd_putc(tt_adj/10%10+0x30);
      lcd_putc(tt_adj%10+0x30);
  
      lcd_putc("%");
      
      if(t==pt)
      {
         setup_ccp1(ccp_off);
         setup_ccp2(ccp_off);
         output_low(pin_c2);
        
         lcd_gotoxy(1,2);
         lcd_putc("TATQUAT");
      }
      else if(t>pt)
      {
         setup_ccp1(ccp_pwm);
         output_high(pin_c2);
         
         set_pwm1_duty(duty);
         lcd_gotoxy(1,2);
         lcd_putc("Dir=FW  ");   
      }
     
   }
}



