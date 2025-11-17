#include "Process.h"

u8 run;
int main(void)
{
	run=MQTT;
	Init();
	while(1)
	{
		switch(run)
		{
			case MQTT:
				MQTT_Process();
				break;
			case HTTP:
				HTTP_Process();
				break;
			case REBOOT:
				Sys_Soft_Reset();
				break;
			case WIFI_MQTT:
				WIFI_MQTT_Process();
				break;
			case WIFI_HTTP:
				WIFI_HTTP_Process();
				break;
		}
	}
}






