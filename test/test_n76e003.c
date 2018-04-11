#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"


void main()
{
	
	while(1){
		
	P00=1;
	Timer3_Delay100ms(5);
	P00=0;
	Timer3_Delay100ms(5);
	}
	
}