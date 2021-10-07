 /**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F7xx HAL API Template project 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	/* WINDOWS and Matrix Initialize*/
	#define WINDOWS_WIDTH 465
	#define WINDOWS_HEIGHT 255
	
	#define MATRIX_WIDTH 415
	#define MATRIX_HEIGHT 255
	
	#define MATRIX_WIDTH_NUM 15
	#define MATRIX_HEIGHT_NUM 8
	int bomb;
  int d[MATRIX_WIDTH_NUM + 2][MATRIX_HEIGHT_NUM + 2];
	int mark[MATRIX_WIDTH_NUM + 2][MATRIX_HEIGHT_NUM + 2];
  int W = (MATRIX_WIDTH/MATRIX_WIDTH_NUM);
	int H = (MATRIX_HEIGHT/MATRIX_HEIGHT_NUM);
	int touch_x, touch_y;
RNG_HandleTypeDef RNG_Handler;	
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng){
   __HAL_RCC_RNG_CLK_ENABLE();
}
uint8_t RNG_Init(void){

  uint16_t retry=0;
  
  RNG_Handler.Instance=RNG;
  HAL_RNG_Init(&RNG_Handler);
  
  while(__HAL_RNG_GET_FLAG(&RNG_Handler,RNG_FLAG_DRDY)==RESET&&retry<10000){
    retry++;
    HAL_Delay(10);
  }
  
  if(retry>=10000) return 1;
  
  return 0;

}

uint32_t RNG_Get_RandomNum(void){
  return HAL_RNG_GetRandomNumber(&RNG_Handler);
}

void init_bombs_map(void){
	// bomb = 99, edge = -8, init = -1, 1~8 = the number of bomb
  // step but no bomb = 0

	int i, g, k, temp1, temp2;
  
	for (g = 0; g <= MATRIX_WIDTH_NUM + 1; g++){
		for (k = 0; k <= MATRIX_HEIGHT_NUM + 1; k++){
			if (g == 0 || k == 0 || g == MATRIX_WIDTH_NUM + 1 || k == MATRIX_HEIGHT_NUM + 1){
				d[g][k] = -8;
				mark[g][k] = -8;
			}else{
				d[g][k] = -1;
				mark[g][k] = -1;
			}
		}
	}

  /* random bumb position*/
	RNG_Init();
	uint32_t random;
  for(i = 0; i < bomb; i++){
		//srand(time(NULL));
		random =RNG_Get_RandomNum(); 
    temp1 = (random% MATRIX_WIDTH_NUM) + 1;
		random =RNG_Get_RandomNum(); 
    temp2 = (random% MATRIX_HEIGHT_NUM) + 1;
		if (d[temp1][temp2] == 99)
		{
			//bomb = bomb - 1;
			i = i -1;
		}
		d[temp1][temp2] = 99;// put this and put i = i -1; in else  is better
  }		
	
}

void draw(void){
	int i, j;
	/* Clear the LCD */	
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  // Display line   
  BSP_LCD_DrawRect(10, 10, 40, 80);
  BSP_LCD_DrawRect(10, 10 + (WINDOWS_HEIGHT/2) - 20, 40, 40);		
  BSP_LCD_DrawRect(10, 10 + (WINDOWS_HEIGHT) - 80, 40, 80);
  for(i = 0; i < MATRIX_WIDTH_NUM; i++){		
    for(j = 0; j < MATRIX_HEIGHT_NUM; j++){
      BSP_LCD_DrawRect( 60 + i * W, 10 + j * H, W, H);
    }	
  }
	
  // Display bomb numbers 
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	//BSP_LCD_DisplayStringAt(0, 30, (uint8_t*)bomb, LEFT_MODE);
	BSP_LCD_DisplayChar(10, 10 + (WINDOWS_HEIGHT) - 80, (bomb/10)+48);
	BSP_LCD_DisplayChar(10 + 20, 10 + (WINDOWS_HEIGHT) - 80, (bomb%10)+48);

  // Display position
  for(i = 0; i <= MATRIX_WIDTH_NUM; i++)	{
  	for (j = 0; j <= MATRIX_HEIGHT_NUM ; j++) {
      if (d[i][j] == -1 || d[i][j] == 99) {
  	 		 //No step(can step, nothing to do)
   	  }
      else if (d[i][j] == 0) {
        //Step(didn't have bomb, turn white)
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_DisplayChar(60 + (i-1) * W, 10 + (j-1) * H, d[i][j]+48);
      }
      else if (d[i][j] == -8) {
        //wall (nothing to do )
      }
      else {
        //print the bomb number 
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_DisplayChar(60 + (i-1) * W, 10 + (j-1) * H, d[i][j]+48);
      }
    }	
  }
}

void step_on(int x, int y){
	int i, j, count,o;
	count = 0;
	//count the number of bombs
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			if (d[i + x][j + y] == 99) {
				count = count + 1;
			}
		}
	}
	//Recursive
	if (count == 0) {
		d[x][y] = 0; // user is steped the position but no bomb
		for (j = -1; j <= 1; j++) {
			for (i = -1; i <= 1; i++) {
				if (d[i + x][j + y] == -1) {
					/*
  		    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
					o = ((i + x)/ 10)+48;
  		    BSP_LCD_DisplayChar(10, 10, o);	
					o = ((i + x)% 10)+48;
					BSP_LCD_DisplayChar(25, 10, o);	
					draw();
					HAL_Delay(100);*/
					step_on(i + x, j + y);
				}
			}	
    }			
	}else {
		d[x][y] = count;
	}	
}

int win_judge(void) {
	int win_or_not = 0;
	int i, j;
	int correct_items = 0, untreated_items = 0;

	for (i = 1; i <= MATRIX_WIDTH_NUM; i++) {
		for (j = 1; j <= MATRIX_HEIGHT_NUM; j++) {
			//if (mark[i][j] == 1 && d[i][j] == 99) {
			if (d[i][j] == 99) {
				correct_items = correct_items + 1;
			}
			else if (d[i][j] == -1) {
				untreated_items = untreated_items + 1;
			}
		}
	}

	if ((correct_items == bomb) && (untreated_items == 0)) {
		win_or_not = 1;
	}
	return win_or_not;
}

void get_touch_position(int TS_x, int TS_y){
   /* controller*/
   touch_x = (TS_x - 60)/W+1;
	 touch_y = (TS_y - 10)/H+1;
   BSP_LCD_DisplayChar(10, 10 + (WINDOWS_HEIGHT/2) - 20, (touch_x%10)+48);
	 BSP_LCD_DisplayChar(30, 10 + (WINDOWS_HEIGHT/2) - 20, touch_y+48);
}
void display_bomb_number(){
  //Display the number keyboard
	int i, j;
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DisplayStringAt(10, 10, (uint8_t *)"Choose the number of bombs.", LEFT_MODE);

  for(j = 0; j < 3; j++){
		for(i = 0; i < 3; i++){
       BSP_LCD_DisplayChar(150 + (i+1) * W, 10 + (4 + j)*H, i+1+j*3+48);		
		}
  }
	BSP_LCD_DisplayChar(150 + 4 * W, 10 + 4*H, 0+48);
	BSP_LCD_DisplayChar(150 + 4 * W, 10 + 5*H, 45);
	BSP_LCD_DisplayChar(150 + 4 * W, 10 + 6*H, 69);	
}
void decide_bomb_number(){
	/*Decide the Numbers of bombs*/
	int i, j;
	int enter;
	int temp1, temp2;
	temp1 = -1;
	temp2 = -1;
  enter = 0;
  display_bomb_number();
	static TOUCH_STATE TS_State;
	do{

     Touch_GetState(&TS_State);
     if(TS_State.pressed==1){
			 BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			 get_touch_position(TS_State.x, TS_State.y);

			 for(j = 0; j < 3; j++){
      		 for(i = 0; i < 3; i++){
						    //1~9
                if(touch_x == i+5 && touch_y == j+5){
         		       temp1 = i+3*j+1;
         		    }
     					  // 0
                if(touch_x == 8 && touch_y == 5){
                   temp1 = 0;
                }								
					 }
			 }
			 if(temp1 != -1 && temp2 ==-1){
				   //digits
			     temp2 = temp1;
				   temp1 = -1;
				   BSP_LCD_Clear(LCD_COLOR_BLACK);
				   display_bomb_number();	
				   BSP_LCD_DisplayChar(150 + 4 * W, 10 + 2*H, temp2+48);
           HAL_Delay(100);
			 }
       if(temp1 != -1 && temp2 != -1){
				   //tens
				   BSP_LCD_DisplayChar(150 + 3 * W, 10 + 2*H, temp2+48);
				   BSP_LCD_DisplayChar(150 + 4 * W, 10 + 2*H, temp1+48);
           HAL_Delay(100);           
       }
			 //enter
     	 if(touch_x == 8 && touch_y == 7){
				 
				 bomb = (temp1==-1) ? temp2 : temp1+temp2*10;
				 enter = 1;
			 }
     	 // -
     	 if(touch_x == 8 && touch_y == 6){
          temp1 = -1;
 				  temp2 = -1;
          BSP_LCD_Clear(LCD_COLOR_BLACK);
          display_bomb_number();								 
			 }
    }
	}while(enter != 1);	
}
int main(void)
{
  /* This project template calls firstly CPU_CACHE_Enable() in order to enable the CPU Cache.
     This function is provided as template implementation that User may integrate 
     in his application, to enhance the performance in case of use of AXI interface 
     with several masters. */

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32F7xx HAL library initialization:
       - Configure the Flash ART accelerator on ITCM interface
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the System clock to have a frequency of 216 MHz */
  SystemClock_Config();

  /* Add your application code here */
	SystemCoreClockUpdate();
  BSP_LCD_Init();  
  /* LCD Layer Initialization */
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS);   
  /* Select the LCD Layer */
  BSP_LCD_SelectLayer(1);  
  /* Enable the display */
  BSP_LCD_DisplayOn();
	BSP_LCD_Clear(LCD_COLOR_BLACK);  
	BSP_LCD_DrawPixel(0,0,LCD_COLOR_WHITE);
	BSP_LCD_DrawPixel(1,1,LCD_COLOR_WHITE);
	BSP_LCD_DrawPixel(2,2,LCD_COLOR_WHITE);
	BSP_LCD_DrawPixel(478,270,LCD_COLOR_WHITE);
	BSP_LCD_DrawPixel(479,271,LCD_COLOR_WHITE);
	
	/* Touch Initialize*/
	SystemClock_Config();
  Touch_Initialize();
  int i, j;
  /*Decide the inital bomb number*/
	decide_bomb_number();
	/*Test the bomb number is exist*/
	/*
	BSP_LCD_DisplayStringAt(60, 10 + 6*H, (uint8_t *)"The Number of Bomb is ", LEFT_MODE);
	BSP_LCD_DisplayChar(60, 10 + 7*H, tens+48);
	BSP_LCD_DisplayChar(65, 10 + 7*H, digits+48);
	*/
  init_bombs_map();
	draw();
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	draw();
	HAL_Delay(1000);
	int win;
	
  /* Infinite loop */
  while (1)
  {	
		/* Interface*/	
		 draw();
		 
 	   static TOUCH_STATE TS_State;
     Touch_GetState(&TS_State);
     if(TS_State.pressed==1){
			 
       if(TS_State.x<60 || TS_State.x>460 ||TS_State.y<10 || TS_State.y>265){
         //illegal touch!
				 BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	   		 BSP_LCD_DisplayStringAtLine(10, (uint8_t *)"illegal touch!");
				 HAL_Delay(500);
				 BSP_LCD_Clear(LCD_COLOR_BLACK);
				 continue;
       }else{
				 get_touch_position(TS_State.x, TS_State.y);
         BSP_LCD_DrawPixel(TS_State.x,TS_State.y,LCD_COLOR_WHITE);
				 //bomb!!!
	   	   if(d[touch_x][touch_y]==99){	   			 
	   			 BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	   			 BSP_LCD_DisplayStringAtLine(10, (uint8_t *)"bomb!!!");
           draw();
           for(i = 0; i <= MATRIX_WIDTH_NUM; i++)	{
             for (j = 0; j <= MATRIX_HEIGHT_NUM ; j++) {
					     if (d[i][j] == 99) {
								 //print bombs as *
  	             BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  	             BSP_LCD_DisplayChar(60 + (i-1) * W, 10 + (j-1) * H, 42);					        
					     }
					   }
				   }
					 HAL_Delay(1000);
	   			 //break;
	   		 }
	   		 step_on(touch_x, touch_y);
	   		 draw();
       }
     }		 
     win = win_judge();
		if (win == 1){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DisplayStringAtLine(10, (uint8_t *)"WIN!!!!!");
			HAL_Delay(1000);
			break;
		}

  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
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
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
