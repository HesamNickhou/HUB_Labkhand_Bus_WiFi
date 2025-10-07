#include "bsp.h"
#include <includes.h>
#include <WM.h>
#include "Config.h"
#include "key/Keypad.h"

#ifdef BUSDOOR_WIFI
	#include "BusDoorWifi.h"
	#include "esp8266/esp8266.h"
#else
	#include "BusDoor.h"
	#include "gps/gps.h"
	#include "gsm/gsm.h"
#endif

uint8_t GUI_Initialized   = 0;

int main(void) {
  BSP_Init();	
	MainAlef();
}

/**
* @brief  BSP_Background.
* @param  None
* @retval None'
*/ 
void BSP_Background (void) {
  static uint32_t LED_ticks = 0;
  static uint32_t POINTER_ticks = 0;
  
  if (LED_ticks++ > 100)
    LED_ticks = 0;

  if (POINTER_ticks++ > 50){
    POINTER_ticks = 0;
    /* Capture input event and updade cursor */
    if (GUI_Initialized == 1)
    {
      // BSP_Pointer_Update();
    }
  }
}  

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
