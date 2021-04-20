/**************************************************
* Centro Universitário Da FEI
* Microprocessadores
* Exemplo utilização ADC com RL78
**************************************************/

// Includes para G13
#include "ior5f100le.h"
#include "ior5f100le_ext.h"

#include "intrinsics.h"
#include "myRL78.h"
// Configura watchdog
#pragma location = "OPTBYTE"
__root __far const char opbyte0 = WDT_OFF;
// Configura detector de baixa tensão
#pragma location = "OPTBYTE"
__root __far const char opbyte1 = LVD_OFF;
// oscilador 32MHz flash high speed
#pragma location = "OPTBYTE"
__root __far const char opbyte2 = FLASH_HS | CLK_32MHZ;
// debug ativado, com apagamento em caso de falha de autenticação
#pragma location = "OPTBYTE"
__root __far const char opbyte3 = DEBUG_ON_ERASE;
/* Configura security ID */
#pragma location = "SECUID"
__root __far const char senha[10] = {0,0,0,0,0,0,0,0,0,0};
/* declaração de variaveis globais do sistema, usada em mais de uma rotina*/
 unsigned int valor[20], val, nota=0, i=0, estagio, nota_anterior;

#pragma vector = INTAD_vect	//sinaliza ao compilador para colocar a chamada da rotina de interrupção no endereço 0x34 da memoria de programa
/* funções iniciadas em "__interrupt" são entendidas pelo compilador como subrotinas de interrupção*/

 __interrupt void trata_ADC(void)
{
	valor[i] = ADCR >> 6; 	// lê o resultado da conversão e guarda em val
	if(i==19)
        { 
          int j;
          i = 0;
          val = 0;
          
          for(j=0;j<20;j++)
          {
            val = val + valor[j];
          }
          val = val/20;
          estagio = 1;
        }
        else
        { 
          i++;    
          estagio = 0;
        }
}

#pragma vector = INTTM00_vect
__interrupt void trata_TAU0_canal0(void)
{
 //LED = !LED; // inverte o estado do led
}

int identifica_nota(void);
void init_AD_10b (void);
void motor_vibra(void);
void init_PWM(void);

void main(void)
{

	init_AD_10b();		// inicia ADC
        init_PWM();             // inicia PWM
	__enable_interrupt();	// habilita interrupção
	ADCS = 1; 		// inicia conversões
	TS0L = TAU_CH0 |TAU_CH1;// dispara os canais master/slave
	estagio = 0;		// estado para aguardo da interrupção
        
	while(1)
	{
		switch(estagio)
		{
		case 0:				// aguarda interrupção
			__no_operation();
			break;
		case 1:				// analisa o valor, converte para a nota
			ADCS = 0;	
			nota_anterior = nota;
                        nota = identifica_nota();
			if(nota != nota_anterior) mudancadenota = 1;
                        estagio = 2;
			break;
		case 2: 			// ativa as interfaces de saída
                        if(mudancadenota) 
                        { mudancadenota = 0;
                          motor_vibra();	
                        }
                        estagio = 3;
			break;
		case 3:				// reinicia conversões
			ADCS = 1;
			estagio = 0;
			break;
		default:
			break;
		}
	}

}



int identifica_nota(void)
{
	if(val<932 && val>849)  return 1;               // A
	else if(val<850 && val>748)     return 2;	// A#	
	else if(val<749 && val>562)     return 3;       // B
        else if(val<653 && val>553)     return 4;       // C
	else if(val<554 && val>469)     return 5;       // C#
	else if(val<470 && val>399)	return 6;       // D 
	else if(val<400 && val>338)     return 7;       // D#
	else if(val<339 && val>285)     return 8;       // E
	else if(val<286 && val>249)	return 9;       // F
	else if(val<250 && val>203)	return 10;      // F#
	else if(val<204 && val>175)	return 11;      // G
	else if(val<176 && val>148)	return 12;      // G#
	else return 0;
	
}


void motor_vibra(void)
{
	TT0L = TAU_CH0 |TAU_CH1;		// parada dos canais master/slave	
	
	switch(nota)
	{	
	case 1: TDR01 = 2000;			// ciclo ativo = 200ms - 1,000V
		break;
	case 2: TDR01 = 2470;			// ciclo ativo = 247ms - 1,236V
		break;	
	case 3: TDR01 = 2940;			// ciclo ativo = 294ms - 1,472V
		break;	
	case 4: TDR01 = 3420;			// ciclo ativo = 342ms - 1,708V
		break;	
	case 5: TDR01 = 3890;			// ciclo ativo = 389ms - 1,944V
		break;	
	case 6: TDR01 = 4360;			// ciclo ativo = 436ms - 2,180V
		break;
	case 7: TDR01 = 4830;			// ciclo ativo = 483ms - 2,416V
		break;	
	case 8: TDR01 = 5300;			// ciclo ativo = 530ms - 2,652V
		break;
	case 9: TDR01 = 5780;			// ciclo ativo = 578ms - 2,888V
		break;
	case 10:TDR01 = 6430;			// ciclo ativo = 643ms - 3,124V
		break;
	case 11:TDR01 = 6720;			// ciclo ativo = 672ms - 3,360V
		break;
	case 12:TDR01 = 7190;			// ciclo ativo = 719ms - 2,596V
		break;
	defaut:	TDR01 = 0000;			// ciclo ativo = 0ms - 0V
		break;
	}

	TS0L = TAU_CH0 |TAU_CH1;		// dispara os canais master/slave
}

void init_AD_10b(void)
{
//Ativar o conversor no registrador PER0
	ADCEN = 1;
//Configurar pinos que serão utilizados no ADC
	PM2_bit.no0 = 1; // setando P20 como entrada
	PM2_bit.no1 = 1; // setando P21 como entrada
	PM2_bit.no2 = 1; // setando P22 como entrada
	PM2_bit.no3 = 1; // setando P23 como entrada
	ADPC = 0x05;	 // P20, P21, P22 (não usa) e P23 (sensor) no modo analógico
//Configurar o ADC
	ADM0 = ADCLK_DIV16 | ADC_LV0 | bADCE;	// fad=fclk/16 | testab=2fad.tamostr=7fad | ativa comparador
	ADM1 = ADC_TRIG_SOFT;			// disparo por software (bit ADCS) | conversão sequencial  
	ADM2 = ADC_REFP_PIN | bADREFM; 		// REF+ = P20 | REF- = P21
	ADS = ADC_CH3;				// seleciona o canal 3 (P23)
	ADMK = 0;				// habilita interrupção
}

void init_PWM(void)
{
	PM1 = 0xBF;							// 0b10111111 - P16 como saída (TAU0 canal 1)
	TAU0EN = 1;							// habilita TAU0
	TPS0 = TAU_CK0_DIV32;						// fclk/32 para o CK0
	TMR00 = TAU_CK0 | TAU_TRIG_SOFT | TAU_MD_TIMER_TRIG_INT;	// configuração do canal 0 (master) de TAU0
	TMR01 = TAU_CK0 | TAU_TRIG_MASTER | TAU_MD_ONECOUNT_TRIG;	// configuração do canal 1 (slave) de TAU0
	TDR00 = 9999;							// período de PWM = 1ms
	TDR01 = 0000;							// ciclo ativo = 0ms;
	TOE0L = TAU_CH1;						// habilita canal 1 do TAU0
	TOM0L = TAU_CH1;						// saída canal 1 controlada por master/slave
}
