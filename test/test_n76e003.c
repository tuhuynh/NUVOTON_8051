#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"

void main()
{
	
	while(1){
		
		P00=1;
		Timer1_Delay10ms(1);
		TH0=10;
		TL0=10;
		TR0=1;
		if(TF0==1) {TR0=0;}
	}
	
}