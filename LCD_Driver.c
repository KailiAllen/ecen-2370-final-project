/*
 * LCD_Driver.c
 *
 *  Created on: Nov 14, 2023
 *      Author: Mathias, modified by Xavion
 *      
 */

#include "LCD_Driver.h"

bool pressed = 0;
uint32_t timelvl1 = 0;
uint32_t timelvl2 = 0;
uint32_t timelvl3_best = 0;
uint32_t timelvl3_temp = 0;
int counter = 0;

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;

static SPI_HandleTypeDef SpiHandle;
uint32_t SpiTimeout = SPI_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */

//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.

//static void MX_LTDC_Init(void);
//static void MX_SPI5_Init(void);
static void SPI_MspInit(SPI_HandleTypeDef *hspi);
static void SPI_Error(void);

/* Provided Functions and API  - MOTIFY ONLY WITH EXTREME CAUTION!!! */

void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}


void LTCD__Init(void)
{
  	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
  LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}

}

// Draws a single pixel, should be useds only within this fileset and should not be seen by external clients. 
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.

}


void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

// Displays Char
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_DrawChar(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

// Draw Circle Filled
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
  for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

// Draw Vertical Line
void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(i+x, y, color);
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
  if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
}

// MINE
void LCD_Draw_Square_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t len, uint16_t color){

	for(uint16_t i = 0; i < len; i++){
		LCD_Draw_Vertical_Line(Xpos+i, Ypos, len, color);
	}
}

void LCD_Draw_Triangle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t heightlen, uint16_t baselen, uint16_t color){

	for(uint16_t i = 0; i < baselen; i++){
		LCD_Draw_Vertical_Line(Xpos+i, Ypos, heightlen-i, color);
	}
}

void LCD_Error_Handler(void)
{
  for(;;); // Something went wrong
}

uint32_t prime(uint32_t num){
	// pretty sure i just loop and mod i
	for(uint32_t i = 2; i <= num/2; i++){
		if(num % i == 0){
			return 0;
		}
	}
	return 1;
}

uint32_t reactiontime(uint32_t time){
	uint32_t stoptime = HAL_GetTick();
	uint32_t reactiontime = stoptime - time;
	return reactiontime;
}

void setpressed1(){
	pressed = 1;
}
void setpressed0(){
	pressed = 0;
}

void titlescreen(){ // TODO make a real title screen, and level instructions functions
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_MAGENTA);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(110, 70, 'H');
	LCD_DisplayChar(125, 70, 'I');
	LCD_DisplayChar(140, 70, '!');

	LCD_DisplayChar(120, 120, ':');
	LCD_DisplayChar(130, 120, 'D');

	HAL_Delay(3000);
	LCD_Clear(0, LCD_COLOR_BLACK);
}

void lvl1title(){
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	char level[5] = "LEVEL";
	char press[5] = "PRESS";
	char button[6] = "BUTTON";
	char when[4] = "WHEN";
	char circle[6] = "CIRCLE";
	char appears[7] = "APPEARS";

	int xpos = 50;

	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 30, level[i]);
	}
	LCD_DisplayChar(140, 30, '1');
	xpos = 50;
	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 90, press[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 120, button[i]);
	}
	xpos = 50;
	for(int i = 0; i < 4; i++){
		LCD_DisplayChar(xpos+=15, 150, when[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 180, circle[i]);
	}
	xpos = 50;
	for(int i = 0; i < 7; i++){
		LCD_DisplayChar(xpos+=15, 210, appears[i]);
	}

	HAL_Delay(3000);
}

void lvl2title(){
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	char level[5] = "LEVEL";
	char press[5] = "PRESS";
	char button[6] = "BUTTON";
	char when[4] = "WHEN";
	char boxin[6] = "BOX IN";
	char topleft[8] = "TOP LEFT";

	int xpos = 50;

	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 30, level[i]);
	}
	LCD_DisplayChar(140, 30, '2');
	xpos = 50;
	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 90, press[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 120, button[i]);
	}
	xpos = 50;
	for(int i = 0; i < 4; i++){
		LCD_DisplayChar(xpos+=15, 150, when[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 180, boxin[i]);
	}
	xpos = 50;
	for(int i = 0; i < 8; i++){
		LCD_DisplayChar(xpos+=15, 210, topleft[i]);
	}

	HAL_Delay(3000);
}

void lvl3title(){
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	char level[5] = "LEVEL";
	char press[5] = "PRESS";
	char button[6] = "BUTTON";
	char when[4] = "WHEN";
	char circle[6] = "CIRCLE";
	char completely[10] = "COMPLETELY";
	char crosses[7] = "CROSSES";
	char line[4] = "LINE";

	int xpos = 50;

	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 30, level[i]);
	}
	LCD_DisplayChar(140, 30, '3');
	xpos = 50;
	for(int i = 0; i < 5; i++){
		LCD_DisplayChar(xpos+=15, 90, press[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 120, button[i]);
	}
	xpos = 50;
	for(int i = 0; i < 4; i++){
		LCD_DisplayChar(xpos+=15, 150, when[i]);
	}
	xpos = 50;
	for(int i = 0; i < 6; i++){
		LCD_DisplayChar(xpos+=15, 180, circle[i]);
	}
	xpos = 50;
	for(int i = 0; i < 10; i++){
		LCD_DisplayChar(xpos+=15, 210, completely[i]);
	}
	xpos = 50;
	for(int i = 0; i < 7; i++){
		LCD_DisplayChar(xpos+=15, 240, crosses[i]);
	}
	xpos = 50;
	for(int i = 0; i < 4; i++){
		LCD_DisplayChar(xpos+=15, 270, line[i]);
	}

	HAL_Delay(3000);
}

void printnumtest(uint32_t time){

	char buffer[4];
	sprintf(buffer, "%lu", time); // %lu is long unsigned int, in this case uint32_t

	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(0, 0, buffer[0]);
	LCD_DisplayChar(15, 0, buffer[1]);
	LCD_DisplayChar(30, 0, buffer[2]);
	LCD_DisplayChar(45, 0, buffer[3]);
	HAL_Delay(5000);
}

void endscreen(){

//	char level[5] = "LEVEL";

	uint32_t timeavg_num = (timelvl1+timelvl2+timelvl3_best)/3;

	char time1[4];
	char time2[4];
	char time3[4];
	char timeavg[4];
	sprintf(time1, "%lu", timelvl1); // %lu is long unsigned int, in this case uint32_t
	sprintf(time2, "%lu", timelvl2);
	sprintf(time3, "%lu", timelvl3_best);
	sprintf(timeavg, "%lu", timeavg_num);

	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

//	int xpos = 50;
//
//	for(int i = 0; i != '\0'; i++){
//		LCD_DisplayChar(xpos+=15, 30, level[i]);
//	}
	LCD_DisplayChar(50, 30, 'L');
	LCD_DisplayChar(65, 30, 'E');
	LCD_DisplayChar(80, 30, 'V');
	LCD_DisplayChar(95, 30, 'E');
	LCD_DisplayChar(110, 30, 'L');
	LCD_DisplayChar(140, 30, '1');

	if(timelvl1 > 1000){
		LCD_DisplayChar(50, 65, time1[0]);
		LCD_DisplayChar(70, 65, '.');
		LCD_DisplayChar(90, 65, time1[1]);
		LCD_DisplayChar(110, 65, time1[2]);
		LCD_DisplayChar(130, 65, time1[3]);
	}
	else if(timelvl1 > 100){
		LCD_DisplayChar(50, 65, '0');
		LCD_DisplayChar(70, 65, '.');
		LCD_DisplayChar(90, 65, time1[0]);
		LCD_DisplayChar(110, 65, time1[1]);
		LCD_DisplayChar(130, 65, time1[2]);
	}
	else{
		LCD_DisplayChar(50, 65, '0');
		LCD_DisplayChar(70, 65, '.');
		LCD_DisplayChar(90, 65, '0');
		LCD_DisplayChar(110, 65, time1[0]);
		LCD_DisplayChar(130, 65, time1[1]);
	}

//	xpos = 50;
//	for(int i = 0; i != '\0'; i++){
//		LCD_DisplayChar(xpos+=15, 100, level[i]);
//	}
	LCD_DisplayChar(50, 100, 'L');
	LCD_DisplayChar(65, 100, 'E');
	LCD_DisplayChar(80, 100, 'V');
	LCD_DisplayChar(95, 100, 'E');
	LCD_DisplayChar(110, 100, 'L');
	LCD_DisplayChar(140, 100, '2');

	if(timelvl2 > 1000){
		LCD_DisplayChar(50, 135, time2[0]);
		LCD_DisplayChar(70, 135, '.');
		LCD_DisplayChar(90, 135, time2[1]);
		LCD_DisplayChar(110, 135, time2[2]);
		LCD_DisplayChar(130, 135, time2[3]);
	}
	else if(timelvl2 > 100){
		LCD_DisplayChar(50, 135, '0');
		LCD_DisplayChar(70, 135, '.');
		LCD_DisplayChar(90, 135, time2[0]);
		LCD_DisplayChar(110, 135, time2[1]);
		LCD_DisplayChar(130, 135, time2[2]);
	}
	else{
		LCD_DisplayChar(50, 135, '0');
		LCD_DisplayChar(70, 135, '.');
		LCD_DisplayChar(90, 135, '0');
		LCD_DisplayChar(110, 135, time2[0]);
		LCD_DisplayChar(130, 135, time2[1]);
	}

//	xpos = 50;
//	for(int i = 0; i != '\0'; i++){
//		LCD_DisplayChar(xpos+=15, 170, level[i]);
//	}
	LCD_DisplayChar(50, 170, 'L');
	LCD_DisplayChar(65, 170, 'E');
	LCD_DisplayChar(80, 170, 'V');
	LCD_DisplayChar(95, 170, 'E');
	LCD_DisplayChar(110, 170, 'L');
	LCD_DisplayChar(140, 170, '3');

	if(timelvl3_best > 1000){
		LCD_DisplayChar(50, 205, time3[0]);
		LCD_DisplayChar(70, 205, '.');
		LCD_DisplayChar(90, 205, time3[1]);
		LCD_DisplayChar(110, 205, time3[2]);
		LCD_DisplayChar(130, 205, time3[3]);
	}
	else if(timelvl3_best > 100){
		LCD_DisplayChar(50, 205, '0');
		LCD_DisplayChar(70, 205, '.');
		LCD_DisplayChar(90, 205, time3[0]);
		LCD_DisplayChar(110, 205, time3[1]);
		LCD_DisplayChar(130, 205, time3[2]);
	}
	else if(timelvl3_best > 10){
		LCD_DisplayChar(50, 205, '0');
		LCD_DisplayChar(70, 205, '.');
		LCD_DisplayChar(90, 205, '0');
		LCD_DisplayChar(110, 205, time3[0]);
		LCD_DisplayChar(130, 205, time3[1]);
	}
	else{
		LCD_DisplayChar(50, 205, '0');
		LCD_DisplayChar(70, 205, '.');
		LCD_DisplayChar(90, 205, '0');
		LCD_DisplayChar(110, 205, '0');
		LCD_DisplayChar(130, 205, time3[0]);
	}

	LCD_DisplayChar(50, 240, 'A');
	LCD_DisplayChar(65, 240, 'V');
	LCD_DisplayChar(80, 240, 'G');

	if(timeavg_num > 1000){
		LCD_DisplayChar(50, 275, timeavg[0]);
		LCD_DisplayChar(70, 275, '.');
		LCD_DisplayChar(90, 275, timeavg[1]);
		LCD_DisplayChar(110, 275, timeavg[2]);
		LCD_DisplayChar(130, 275, timeavg[3]);
	}
	else if(timeavg_num > 100){
		LCD_DisplayChar(50, 275, '0');
		LCD_DisplayChar(70, 275, '.');
		LCD_DisplayChar(90, 275, timeavg[0]);
		LCD_DisplayChar(110, 275, timeavg[1]);
		LCD_DisplayChar(130, 275, timeavg[2]);
	}
	else{
		LCD_DisplayChar(50, 275, '0');
		LCD_DisplayChar(70, 275, '.');
		LCD_DisplayChar(90, 275, '0');
		LCD_DisplayChar(110, 275, timeavg[0]);
		LCD_DisplayChar(130, 275, timeavg[1]);
	}


	HAL_Delay(7000);
}

void level1(){

	HAL_Delay(10);
	LCD_Clear(0, LCD_COLOR_BLUE);
	HAL_Delay(500);

	uint32_t startTime = 0;

	while(1){

		uint32_t randnum = RNG_getNum();

		uint32_t rand250 = 1+(randnum%250); // roughly 100/100/50 square/triangle/circle

		if(rand250 < 50){
			LCD_Draw_Circle_Fill(120, 120, 40, LCD_COLOR_RED);
			startTime = HAL_GetTick();
			HAL_Delay(10);
			break;
		}
		else if(rand250 >= 50 && rand250 < 150){
			LCD_Draw_Square_Fill(80, 120, 80, LCD_COLOR_RED);
			HAL_Delay(1200);
		}
		else if(rand250 >= 150){
			LCD_Draw_Triangle_Fill(100, 150, 80, 80, LCD_COLOR_RED);
			HAL_Delay(1200);
		}

		HAL_Delay(10);
		LCD_Clear(0, LCD_COLOR_BLUE);
		HAL_Delay(500);
	}
	pressed = 0;
	while(pressed == 0){
		// just stay on the circle until button gets pressed
	}
	timelvl1 = reactiontime(startTime);
	LCD_Clear(0, LCD_COLOR_BLUE);
}

void level2(){

	HAL_Delay(10);
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(120, 0, 320, LCD_COLOR_WHITE);
	LCD_Draw_Horizontal_Line(0, 160, 240, LCD_COLOR_WHITE);
	HAL_Delay(500);

	uint32_t startTime = 0;

	while(1){
		uint32_t randnum = RNG_getNum();

		uint32_t rand350 = 1+(randnum%350);

		if(rand350 < 75){
			LCD_Draw_Square_Fill(40, 100, 40, LCD_COLOR_RED);
			startTime = HAL_GetTick();
			HAL_Delay(10);
			break;
		}
		else if(rand350 >= 75 && rand350 < 166){ // bottom right
			LCD_Draw_Square_Fill(160, 200, 40, LCD_COLOR_RED);
			HAL_Delay(1200);
		}
		else if(rand350 >= 166 && rand350 < 256){ // top right
			LCD_Draw_Square_Fill(160, 100, 40, LCD_COLOR_RED);
			HAL_Delay(1200);
		}
		else if(rand350 >= 256 && rand350 < 350){
			LCD_Draw_Square_Fill(40, 200, 40, LCD_COLOR_RED);
			HAL_Delay(1200);
		}

		HAL_Delay(10);
		LCD_Clear(0, LCD_COLOR_BLACK);
		LCD_Draw_Vertical_Line(120, 0, 320, LCD_COLOR_WHITE);
		LCD_Draw_Horizontal_Line(0, 160, 240, LCD_COLOR_WHITE);
		HAL_Delay(500);
	}

	pressed = 0;
	while(pressed == 0){
		// just stay on the circle until button gets pressed
	}
	timelvl2 = reactiontime(startTime);
	LCD_Clear(0, LCD_COLOR_BLACK);

}

void level3(){

	HAL_Delay(10);
	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_Draw_Vertical_Line(120, 0, 320, LCD_COLOR_WHITE);
	HAL_Delay(500);

	uint32_t startTime = 0;

	uint16_t xpos = 26;

	pressed = 0;

	while(1){

//		startTime = HAL_GetTick();
		pressed = 0;

		for(int i = 0; i < 187; i+=3){
			LCD_Draw_Circle_Fill(i+xpos, 160, 26, LCD_COLOR_BLUE);
			HAL_Delay(20);
			LCD_Clear(0, LCD_COLOR_BLACK);
			LCD_Draw_Vertical_Line(120, 0, 320, LCD_COLOR_WHITE);
			if(i+xpos == 146){ // circle has fully crossed the center line
				startTime = HAL_GetTick();
			}
			if(i+xpos < 146){
				pressed = 0;
			}
			else if(i+xpos >= 146){
				if(pressed == 1){
					break;
				}
			}
		}
		if(pressed == 1){
			break;
		}
		for(int i = 0; i < 187; i+=3){
			if(pressed == 1){
				break;
			}
			LCD_Draw_Circle_Fill(214-i, 160, 26, LCD_COLOR_BLUE);
			HAL_Delay(20);
			LCD_Clear(0, LCD_COLOR_BLACK);
			LCD_Draw_Vertical_Line(120, 0, 320, LCD_COLOR_WHITE);
		}
		if(pressed == 1){
			break;
		}
	}

	if(counter == 0){
		timelvl3_best = reactiontime(startTime);
	}
	else{
		timelvl3_temp = reactiontime(startTime);
		if(timelvl3_temp < timelvl3_best){
			timelvl3_best = timelvl3_temp;
		}
	}
	counter++;
}

void run_game(){
	// switch case with gameState enum and levels

	gameState state = init;

	while(state != done){
		switch(state){
			case init:{
				titlescreen();
				state = lvl1;
				break;
			}
			case lvl1:{
				lvl1title();
				level1();
				state = lvl2;
				break;
			}
			case lvl2:{
				lvl2title();
				level2();
				state = lvl3;
				break;
			}
			case lvl3:{
				lvl3title();
				level3();
				level3();
				level3();
				state = done;
				break;
			}
			case done:{
				break; // handles warning.
			}
		}
	}
	endscreen();
	pressed = 0;
	while(pressed == 0){
		// hold. when pressed, restarts
	}
	LCD_Clear(0, LCD_COLOR_BLACK);
}

void testingtesting(){
	LCD_Draw_Square_Fill(125, 150, 40, LCD_COLOR_BLACK);
	HAL_Delay(3000);
	LCD_Clear(0, LCD_COLOR_BLUE);
	HAL_Delay(1500);
	LCD_Draw_Square_Fill(100, 120, 80, LCD_COLOR_RED);
	HAL_Delay(3000);
	LCD_Clear(0, LCD_COLOR_BLUE);
	HAL_Delay(1500);
	LCD_Draw_Triangle_Fill(125, 150, 40, 40, LCD_COLOR_RED);
	HAL_Delay(3000);
}

// Demo using provided functions
void QuickDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');

}

/*        APPLICATION SPECIFIC FUNCTION DEFINITIONS - PUT YOUR NEWLY CREATED FUNCTIONS HERE       */



/* Lower Level Functions for LTCD. 	MOTIFY ONLY WITH EXTREME CAUTION!!  */
static uint8_t Is_LCD_IO_Initialized = 0;




/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void ili9341_Init(void)
{
  /* Initialize ILI9341 low level bus layer ----------------------------------*/
  LCD_IO_Init();

  /* Configure LCD */
  ili9341_Write_Reg(0xCA);
  ili9341_Send_Data(0xC3);				//param 1
  ili9341_Send_Data(0x08);				//param 2
  ili9341_Send_Data(0x50);				//param 3
  ili9341_Write_Reg(LCD_POWERB); //CF
  ili9341_Send_Data(0x00);				//param 1
  ili9341_Send_Data(0xC1);				//param 2
  ili9341_Send_Data(0x30);				//param 3
  ili9341_Write_Reg(LCD_POWER_SEQ); //ED
  ili9341_Send_Data(0x64);
  ili9341_Send_Data(0x03);
  ili9341_Send_Data(0x12);
  ili9341_Send_Data(0x81);
  ili9341_Write_Reg(LCD_DTCA);
  ili9341_Send_Data(0x85);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x78);
  ili9341_Write_Reg(LCD_POWERA);
  ili9341_Send_Data(0x39);
  ili9341_Send_Data(0x2C);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x34);
  ili9341_Send_Data(0x02);
  ili9341_Write_Reg(LCD_PRC);
  ili9341_Send_Data(0x20);
  ili9341_Write_Reg(LCD_DTCB);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_FRMCTR1);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x1B);
  ili9341_Write_Reg(LCD_DFC);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0xA2);
  ili9341_Write_Reg(LCD_POWER1);
  ili9341_Send_Data(0x10);
  ili9341_Write_Reg(LCD_POWER2);
  ili9341_Send_Data(0x10);
  ili9341_Write_Reg(LCD_VCOM1);
  ili9341_Send_Data(0x45);
  ili9341_Send_Data(0x15);
  ili9341_Write_Reg(LCD_VCOM2);
  ili9341_Send_Data(0x90);
  ili9341_Write_Reg(LCD_MAC);
  ili9341_Send_Data(0xC8);
  ili9341_Write_Reg(LCD_3GAMMA_EN);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_RGB_INTERFACE);
  ili9341_Send_Data(0xC2);
  ili9341_Write_Reg(LCD_DFC);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0xA7);
  ili9341_Send_Data(0x27);
  ili9341_Send_Data(0x04);

  /* Colomn address set */
  ili9341_Write_Reg(LCD_COLUMN_ADDR);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0xEF);

  /* Page address set */
  ili9341_Write_Reg(LCD_PAGE_ADDR);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x01);
  ili9341_Send_Data(0x3F);
  ili9341_Write_Reg(LCD_INTERFACE);
  ili9341_Send_Data(0x01);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x06);

  ili9341_Write_Reg(LCD_GRAM);
  LCD_Delay(200);

  ili9341_Write_Reg(LCD_GAMMA);
  ili9341_Send_Data(0x01);

  ili9341_Write_Reg(LCD_PGAMMA);
  ili9341_Send_Data(0x0F);
  ili9341_Send_Data(0x29);
  ili9341_Send_Data(0x24);
  ili9341_Send_Data(0x0C);
  ili9341_Send_Data(0x0E);
  ili9341_Send_Data(0x09);
  ili9341_Send_Data(0x4E);
  ili9341_Send_Data(0x78);
  ili9341_Send_Data(0x3C);
  ili9341_Send_Data(0x09);
  ili9341_Send_Data(0x13);
  ili9341_Send_Data(0x05);
  ili9341_Send_Data(0x17);
  ili9341_Send_Data(0x11);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_NGAMMA);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x16);
  ili9341_Send_Data(0x1B);
  ili9341_Send_Data(0x04);
  ili9341_Send_Data(0x11);
  ili9341_Send_Data(0x07);
  ili9341_Send_Data(0x31);
  ili9341_Send_Data(0x33);
  ili9341_Send_Data(0x42);
  ili9341_Send_Data(0x05);
  ili9341_Send_Data(0x0C);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0x28);
  ili9341_Send_Data(0x2F);
  ili9341_Send_Data(0x0F);

  ili9341_Write_Reg(LCD_SLEEP_OUT);
  LCD_Delay(200);
  ili9341_Write_Reg(LCD_DISPLAY_ON);
  /* GRAM start writing */
  ili9341_Write_Reg(LCD_GRAM);
}


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOn(void)
{
  /* Display On */
  ili9341_Write_Reg(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOff(void)
{
  /* Display Off */
  ili9341_Write_Reg(LCD_DISPLAY_OFF);
}

/**
  * @brief  Writes  to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void ili9341_Write_Reg(uint8_t LCD_Reg)
{
  LCD_IO_WriteReg(LCD_Reg);
}

/**
  * @brief  Writes data to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void ili9341_Send_Data(uint16_t RegValue)
{
  LCD_IO_WriteData(RegValue);
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  RegValue: Address of the register to read
  * @param  ReadSize: Number of bytes to read
  * @retval LCD Register Value.
  */
uint32_t ili9341_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
  /* Read a max of 4 bytes */
  return (LCD_IO_ReadData(RegValue, ReadSize));
}


/******************************* SPI Routines *********************************/

/**
  * @brief  SPI Bus initialization
  */
static void SPI_Init(void)
{
  if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
  {
    /* SPI configuration -----------------------------------------------------*/
    SpiHandle.Instance = DISCOVERY_SPI;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz)
       to verify these constraints:
       - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
       - l3gd20 SPI interface max baudrate is 10MHz for write/read
       - PCLK2 frequency is set to 90 MHz
    */
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    /* On STM32F429I-Discovery, LCD ID cannot be read then keep a common configuration */
    /* for LCD and GYRO (SPI_DIRECTION_2LINES) */
    /* Note: To read a register a LCD, SPI_DIRECTION_1LINE should be set */
    SpiHandle.Init.Direction      = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase       = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity    = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial  = 7;
    SpiHandle.Init.DataSize       = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS            = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode         = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode           = SPI_MODE_MASTER;

    SPI_MspInit(&SpiHandle);
    HAL_SPI_Init(&SpiHandle);
  }
}

/**
  * @brief  Reads 4 bytes from device.
  * @param  ReadSize: Number of bytes to read (max 4 bytes)
  * @retval Value read on the SPI
  */
static uint32_t SPI_Read(uint8_t ReadSize)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue;

  status = HAL_SPI_Receive(&SpiHandle, (uint8_t*) &readvalue, ReadSize, SpiTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    SPI_Error();
  }

  return readvalue;
}

/**
  * @brief  Writes a byte to device.
  * @param  Value: value to be written
  */
static void SPI_Write(uint16_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&SpiHandle, (uint8_t*) &Value, 1, SpiTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    SPI_Error();
  }
}


/**
  * @brief  SPI error treatment function.
  */
static void SPI_Error(void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&SpiHandle);

  /* Re- Initialize the SPI communication BUS */
  SPI_Init();
}

/**
  * @brief  SPI MSP Init.
  * @param  hspi: SPI handle
  */
static void SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable SPI clock */
  DISCOVERY_SPI_CLK_ENABLE();

  /* Enable DISCOVERY_SPI GPIO clock */
  DISCOVERY_SPI_GPIO_CLK_ENABLE();

  /* configure SPI SCK, MOSI and MISO */
  GPIO_InitStructure.Pin    = (DISCOVERY_SPI_SCK_PIN | DISCOVERY_SPI_MOSI_PIN | DISCOVERY_SPI_MISO_PIN);
  GPIO_InitStructure.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull   = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure.Alternate = DISCOVERY_SPI_AF;
  HAL_GPIO_Init(DISCOVERY_SPI_GPIO_PORT, &GPIO_InitStructure);
}

/********************************* LINK LCD ***********************************/

/**
  * @brief  Configures the LCD_SPI interface.
  */
void LCD_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  if(Is_LCD_IO_Initialized == 0)
  {
    Is_LCD_IO_Initialized = 1;

    /* Configure in Output Push-Pull mode */
    LCD_WRX_GPIO_CLK_ENABLE();
    GPIO_InitStructure.Pin     = LCD_WRX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

    LCD_RDX_GPIO_CLK_ENABLE();
    GPIO_InitStructure.Pin     = LCD_RDX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_RDX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure the LCD Control pins ----------------------------------------*/
    LCD_NCS_GPIO_CLK_ENABLE();

    /* Configure NCS in Output Push-Pull mode */
    GPIO_InitStructure.Pin     = LCD_NCS_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);

    /* Set or Reset the control line */
    LCD_CS_LOW();
    LCD_CS_HIGH();

    SPI_Init();
  }
}

/**
  * @brief  Writes register value.
  */
void LCD_IO_WriteData(uint16_t RegValue)
{
  /* Set WRX to send data */
  LCD_WRX_HIGH();

  /* Reset LCD control line(/CS) and Send data */
  LCD_CS_LOW();
  SPI_Write(RegValue);

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();
}

/**
  * @brief  Writes register address.
  */
void LCD_IO_WriteReg(uint8_t Reg)
{
  /* Reset WRX to send command */
  LCD_WRX_LOW();

  /* Reset LCD control line(/CS) and Send command */
  LCD_CS_LOW();
  SPI_Write(Reg);

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();
}

/**
  * @brief  Reads register value.
  * @param  RegValue Address of the register to read
  * @param  ReadSize Number of bytes to read
  * @retval Content of the register value
  */
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
  uint32_t readvalue = 0;

  /* Select: Chip Select low */
  LCD_CS_LOW();

  /* Reset WRX to send command */
  LCD_WRX_LOW();

  SPI_Write(RegValue);

  readvalue = SPI_Read(ReadSize);

  /* Set WRX to send data */
  LCD_WRX_HIGH();

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();

  return readvalue;
}

/**
  * @brief  Wait for loop in ms.
  * @param  Delay in ms.
  */
void LCD_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}




/*       Generated HAL Stuff      */

//   * @brief LTDC Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_LTDC_Init(void)
// {

//   /* USER CODE BEGIN LTDC_Init 0 */

//   /* USER CODE END LTDC_Init 0 */

//   LTDC_LayerCfgTypeDef pLayerCfg = {0};
//   LTDC_LayerCfgTypeDef pLayerCfg1 = {0};

//   /* USER CODE BEGIN LTDC_Init 1 */

//   /* USER CODE END LTDC_Init 1 */
//   hltdc.Instance = LTDC;
//   hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
//   hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
//   hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
//   hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
//   hltdc.Init.HorizontalSync = 7;
//   hltdc.Init.VerticalSync = 3;
//   hltdc.Init.AccumulatedHBP = 14;
//   hltdc.Init.AccumulatedVBP = 5;
//   hltdc.Init.AccumulatedActiveW = 654;
//   hltdc.Init.AccumulatedActiveH = 485;
//   hltdc.Init.TotalWidth = 660;
//   hltdc.Init.TotalHeigh = 487;
//   hltdc.Init.Backcolor.Blue = 0;
//   hltdc.Init.Backcolor.Green = 0;
//   hltdc.Init.Backcolor.Red = 0;
//   if (HAL_LTDC_Init(&hltdc) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg.WindowX0 = 0;
//   pLayerCfg.WindowX1 = 0;
//   pLayerCfg.WindowY0 = 0;
//   pLayerCfg.WindowY1 = 0;
//   pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg.Alpha = 0;
//   pLayerCfg.Alpha0 = 0;
//   pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg.FBStartAdress = 0;
//   pLayerCfg.ImageWidth = 0;
//   pLayerCfg.ImageHeight = 0;
//   pLayerCfg.Backcolor.Blue = 0;
//   pLayerCfg.Backcolor.Green = 0;
//   pLayerCfg.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg1.WindowX0 = 0;
//   pLayerCfg1.WindowX1 = 0;
//   pLayerCfg1.WindowY0 = 0;
//   pLayerCfg1.WindowY1 = 0;
//   pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg1.Alpha = 0;
//   pLayerCfg1.Alpha0 = 0;
//   pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg1.FBStartAdress = 0;
//   pLayerCfg1.ImageWidth = 0;
//   pLayerCfg1.ImageHeight = 0;
//   pLayerCfg1.Backcolor.Blue = 0;
//   pLayerCfg1.Backcolor.Green = 0;
//   pLayerCfg1.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN LTDC_Init 2 */

//   /* USER CODE END LTDC_Init 2 */

// }

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
// static void MX_SPI5_Init(void)
// {

//   /* USER CODE BEGIN SPI5_Init 0 */

//   /* USER CODE END SPI5_Init 0 */

//   /* USER CODE BEGIN SPI5_Init 1 */

//   /* USER CODE END SPI5_Init 1 */
//   /* SPI5 parameter configuration*/
//   hspi5.Instance = SPI5;
//   hspi5.Init.Mode = SPI_MODE_MASTER;
//   hspi5.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
//   hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi5.Init.NSS = SPI_NSS_SOFT;
//   hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//   hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi5.Init.CRCPolynomial = 10;
//   if (HAL_SPI_Init(&hspi5) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN SPI5_Init 2 */

//   /* USER CODE END SPI5_Init 2 */

// }
