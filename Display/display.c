/*
 *	Драйвер управления дисплеями по SPI
 *  Author: VadRov
 *  Copyright (C) 2023, VadRov, all right reserved.
 *
 *  Допускается свободное распространение.
 *  При любом способе распространения указание автора ОБЯЗАТЕЛЬНО.
 *  В случае внесения изменений и распространения модификаций указание первоначального автора ОБЯЗАТЕЛЬНО.
 *  Распространяется по типу "как есть", то есть использование осуществляется на свой страх и риск.
 *  Автор не предоставляет никаких гарантий.
 *
 *  Версия: 1.4 для м/к ARTERY AT32F403A
 *
 *  https://www.youtube.com/@VadRov
 *  https://dzen.ru/vadrov
 *  https://vk.com/vadrov
 *  https://t.me/vadrov_channel
 *
 */

#include "display.h"
#include "fonts.h"
#include <string.h>
#include "main.h"
#ifdef LCD_DYNAMIC_MEM
#include "stdlib.h"
#endif

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define min(x1,x2)	(x1 < x2 ? x1 : x2)
#define max(x1,x2)	(x1 > x2 ? x1 : x2)

#define min3(x1,x2,x3)	min(min(x1,x2),x3)
#define max3(x1,x2,x3)	max(max(x1,x2),x3)

#define LCD_CS_LOW		if (lcd->spi_data.cs_port) { lcd->spi_data.cs_port->clr = lcd->spi_data.cs_pin; }
#define LCD_CS_HI		if (lcd->spi_data.cs_port) { lcd->spi_data.cs_port->scr = lcd->spi_data.cs_pin; }
#define LCD_DC_LOW		{lcd->spi_data.dc_port->clr = lcd->spi_data.dc_pin;}
#define LCD_DC_HI		{lcd->spi_data.dc_port->scr = lcd->spi_data.dc_pin;}

#define DISABLE_IRQ		__disable_irq(); __DSB(); __ISB();
#define ENABLE_IRQ		__enable_irq();

LCD_Handler *LCD = 0; //список дисплеев

inline void LCD_SetCS(LCD_Handler *lcd)
{
	LCD_CS_HI
}

inline void LCD_ResCS(LCD_Handler *lcd)
{
	LCD_CS_LOW
}

inline void LCD_SetDC(LCD_Handler *lcd)
{
	LCD_DC_HI
}

inline void LCD_ResDC(LCD_Handler *lcd)
{
	LCD_DC_LOW
}

typedef enum {
	lcd_write_command = 0,
	lcd_write_data
} lcd_dc_select;

inline static void LCD_WRITE_DC(LCD_Handler* lcd, uint8_t data, lcd_dc_select lcd_dc)
{
	spi_type *spi = lcd->spi_data.spi;
	if (lcd_dc == lcd_write_command)  {
		LCD_DC_LOW
	}
	else {
		LCD_DC_HI
	}
	*((volatile uint8_t *)&spi->dt) = data;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится
}

void LCD_HardWareReset (LCD_Handler* lcd)
{
	if (lcd->spi_data.reset_port) {
		lcd->spi_data.reset_port->clr = lcd->spi_data.reset_pin;
		delay_ms(25);
		lcd->spi_data.reset_port->scr = lcd->spi_data.reset_pin;
		delay_ms(25);
	}
}

//интерпретатор строк с управлящими кодами: "команда", "данные", "пауза", "завершение пакета"
void LCD_String_Interpretator(LCD_Handler* lcd, uint8_t *str)
{
	spi_type *spi = lcd->spi_data.spi;
	int i;
	while (spi->ctrl1_bit.spien) ; //ждем когда дисплей освободится
	spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //8-битная передача
	spi->ctrl1_bit.spien = TRUE; //SPI включаем
	LCD_CS_LOW
	while (1) {
		switch (*str++) {
			//управляющий код "команда"
			case LCD_UPR_COMMAND:
				//отправляем код команды контроллеру дисплея
				LCD_WRITE_DC(lcd, *str++, lcd_write_command);
				//количество параметров команды
				i = *str++;
				//отправляем контроллеру дисплея параметры команды
				while(i--) {
					LCD_WRITE_DC(lcd, *str++, lcd_write_data);
				}
				break;
			//управляющий код "данные"
			case LCD_UPR_DATA:
				//количество данных
				i = *str++;
				//отправляем контроллеру дисплея данные
				while(i--) {
					LCD_WRITE_DC(lcd, *str++, lcd_write_data);
				}
				break;
			//управляющий код "пауза"
			case LCD_UPR_PAUSE:
				//ожидание в соответствии с параметром (0...255)
				delay_ms(*str++);
				break;
			//управляющий код "завершение пакета"
			case LCD_UPR_END:
			default:
				LCD_CS_HI
				spi->ctrl1_bit.spien = FALSE; //выключаем spi
				return;
		}
	}
}

//создание обработчика дисплея и добавление его в список дисплеев lcds
//возвращает указатель на созданный обработчик либо 0 при неудаче
LCD_Handler* LCD_DisplayAdd(LCD_Handler *lcds,     /* указатель на список дисплеев
													  (первый дисплей в списке) */
#ifndef LCD_DYNAMIC_MEM
							LCD_Handler *lcd,	   /* указатель на создаваемый обработчик дисплея
													  в случае статического выделения памяти */
#endif
							uint16_t resolution1,
							uint16_t resolution2,
							uint16_t width_controller,
							uint16_t height_controller,
							int16_t w_offs,
							int16_t h_offs,
							LCD_PageOrientation orientation,
							DisplayInitCallback init,
							DisplaySetWindowCallback set_win,
							DisplaySleepInCallback sleep_in,
							DisplaySleepOutCallback sleep_out,
							void *connection_data,
							LCD_DATA_BUS data_bus,
							LCD_BackLight_data bkl_data
					   )
{

#ifdef LCD_DYNAMIC_MEM
	LCD_Handler* lcd = (LCD_Handler*)malloc(sizeof(LCD_Handler));
#endif
	if (!lcd) return 0;
	memset(lcd, 0, sizeof(LCD_Handler));
	LCD_DMA_TypeDef *hdma = 0;
	lcd->data_bus = data_bus;
	//инициализация данных подключения
	lcd->spi_data = *((LCD_SPI_Connected_data*)connection_data);
	hdma = &lcd->spi_data.dma_tx;
	//настройка DMA
	if (hdma->dma) {
		dma_channel_type *dma_x = ((dma_channel_type *)((uint32_t)((uint32_t)hdma->dma + 8 + 20 * (hdma->stream - 1))));
		dma_x->ctrl_bit.chen = FALSE; //отключаем канал DMA
		while(dma_x->ctrl_bit.chen) ; //ждем отключения канала
		if (lcd->data_bus == LCD_DATA_8BIT_BUS) {
			dma_x->ctrl_bit.pwidth = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
			dma_x->ctrl_bit.mwidth = DMA_MEMORY_DATA_WIDTH_BYTE;
		}
		else if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
			dma_x->ctrl_bit.pwidth = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
			dma_x->ctrl_bit.mwidth = DMA_MEMORY_DATA_WIDTH_HALFWORD;
		}
		//запрещаем прерывания по ошибке и передачи половины буфера
		dma_x->ctrl_bit.dterrien = FALSE;
		dma_x->ctrl_bit.hdtien = FALSE;
		//разрешаем прерывание по окончанию передачи
		dma_x->ctrl_bit.fdtien = TRUE;
		dma_x->ctrl_bit.pincm = DMA_PERIPHERAL_INC_DISABLE;	//инкремент адреса периферии отключен
		dma_x->ctrl_bit.mincm = DMA_MEMORY_INC_ENABLE;		//инкремент адреса памяти включен
	}
	//настройка ориентации дисплея и смещения начала координат
	uint16_t max_res = max(resolution1, resolution2);
	uint16_t min_res = min(resolution1, resolution2);
	if (orientation==PAGE_ORIENTATION_PORTRAIT || orientation==PAGE_ORIENTATION_PORTRAIT_MIRROR) {
		lcd->Width = min_res;
		lcd->Height = max_res;
		lcd->Width_Controller = width_controller;
		lcd->Height_Controller = height_controller;
		if (orientation==PAGE_ORIENTATION_PORTRAIT) {
			lcd->x_offs = w_offs;
			lcd->y_offs = h_offs;
		}
		else {
			lcd->x_offs = lcd->Width_Controller - lcd->Width - w_offs;
			lcd->y_offs = lcd->Height_Controller - lcd->Height - h_offs;
		}
	}
	else if (orientation==PAGE_ORIENTATION_LANDSCAPE || orientation==PAGE_ORIENTATION_LANDSCAPE_MIRROR)	{
		lcd->Width = max_res;
		lcd->Height = min_res;
		lcd->Width_Controller = height_controller;
		lcd->Height_Controller = width_controller;
		if (orientation==PAGE_ORIENTATION_LANDSCAPE) {
			lcd->x_offs = h_offs;
			lcd->y_offs = lcd->Height_Controller - lcd->Height - w_offs;
		}
		else {
			lcd->x_offs = lcd->Width_Controller - lcd->Width - h_offs;
			lcd->y_offs = w_offs;
		}
	}
	else {
		LCD_Delete(lcd);
		return 0;
	}

	if (lcd->Width_Controller < lcd->Width ||
		lcd->Height_Controller < lcd->Height ||
		init==NULL ||
		set_win==NULL )	{
		LCD_Delete(lcd);
		return 0;
	}
	lcd->Orientation = orientation;
	lcd->Init_callback = init;
	lcd->SetActiveWindow_callback = set_win;
	lcd->SleepIn_callback = sleep_in;
	lcd->SleepOut_callback = sleep_out;
	lcd->bkl_data = bkl_data;
	lcd->display_number = 0;
	lcd->next = 0;
	lcd->prev = 0;
	if (!lcds) {
		return lcd;
	}
	LCD_Handler *prev = lcds;
	while (prev->next) {
		prev = (LCD_Handler *)prev->next;
		lcd->display_number++;
	}
	lcd->prev = (void*)prev;
	prev->next = (void*)lcd;
	return lcd;
}

//удаляет дисплей
void LCD_Delete(LCD_Handler* lcd)
{
	if (lcd) {
#ifdef LCD_DYNAMIC_MEM
		if (lcd->tmp_buf) {
			free(lcd->tmp_buf);
		}
#endif
		memset(lcd, 0, sizeof(LCD_Handler));
#ifdef LCD_DYNAMIC_MEM
		free(lcd);
#endif
	}
}

//инициализирует дисплей
void LCD_Init(LCD_Handler* lcd)
{
	LCD_HardWareReset(lcd);
	LCD_String_Interpretator(lcd, lcd->Init_callback(lcd->Orientation));
	LCD_SetBackLight(lcd, lcd->bkl_data.bk_percent);
}

//возвращает яркость подсветки, %
inline uint8_t LCD_GetBackLight(LCD_Handler* lcd)
{
	return lcd->bkl_data.bk_percent;
}

//возвращает ширину дисплея, пиксели
inline uint16_t LCD_GetWidth(LCD_Handler* lcd)
{
	return lcd->Width;
}

//возвращает высоту дисплея, пиксели
inline uint16_t LCD_GetHeight(LCD_Handler* lcd)
{
	return lcd->Height;
}

//возвращает статус дисплея: занят либо свободен (требуется для отправки новых данных на дисплей)
//дисплей занят, если занято spi, к которому он подключен
inline LCD_State LCD_GetState(LCD_Handler* lcd)
{
	if (lcd->spi_data.spi->ctrl1_bit.spien) {
		return LCD_STATE_BUSY;
	}
	return LCD_STATE_READY;
}

//управление подсветкой
void LCD_SetBackLight(LCD_Handler* lcd, uint8_t bk_percent)
{
	if (bk_percent > 100) {
		bk_percent = 100;
	}
	lcd->bkl_data.bk_percent = bk_percent;
	//подсветка с использованием PWM
	if (lcd->bkl_data.htim_bk) {
		//вычисляем % яркости, как часть от периода счетчика
		uint32_t bk_value = lcd->bkl_data.htim_bk->pr * bk_percent / 100;
		//задаем скважность PWM конкретного канала
		switch(lcd->bkl_data.channel_htim_bk) {
			case TMR_SELECT_CHANNEL_1:
				lcd->bkl_data.htim_bk->c1dt = bk_value;
				break;
			case TMR_SELECT_CHANNEL_2:
				lcd->bkl_data.htim_bk->c2dt = bk_value;
				break;
			case TMR_SELECT_CHANNEL_3:
				lcd->bkl_data.htim_bk->c3dt = bk_value;
				break;
			case TMR_SELECT_CHANNEL_4:
				lcd->bkl_data.htim_bk->c4dt = bk_value;
				break;
			default:
				break;
		}
	}
	//подсветка без PWM (просто вкл./выкл.), если таймер с PWM недоступен
	else if (lcd->bkl_data.blk_port) {
		if (bk_percent) {
			lcd->bkl_data.blk_port->scr = lcd->bkl_data.blk_pin;
		}
		else {
			lcd->bkl_data.blk_port->clr = lcd->bkl_data.blk_pin;
		}
	}
}

//перевод дисплея в "спящий режим" (выключение отображения дисплея и подсветки)
void LCD_SleepIn(LCD_Handler* lcd)
{
	//подсветка с использованием PWM
	if (lcd->bkl_data.htim_bk) {
		//выключаем подсветку, установив нулевую скважность
		switch(lcd->bkl_data.channel_htim_bk) {
			case TMR_SELECT_CHANNEL_1:
				lcd->bkl_data.htim_bk->c1dt = 0;
				break;
			case TMR_SELECT_CHANNEL_2:
				lcd->bkl_data.htim_bk->c2dt = 0;
				break;
			case TMR_SELECT_CHANNEL_3:
				lcd->bkl_data.htim_bk->c3dt = 0;
				break;
			case TMR_SELECT_CHANNEL_4:
				lcd->bkl_data.htim_bk->c4dt = 0;
				break;
			default:
				break;
		}
	}
	//подсветка без PWM (просто вкл./выкл.), если таймер с PWM недоступен
	else if (lcd->bkl_data.blk_port) {
		lcd->bkl_data.blk_port->clr = lcd->bkl_data.blk_pin;
	}
	if (lcd->SleepIn_callback) {
		LCD_String_Interpretator(lcd, lcd->SleepIn_callback());
	}
}

//вывод дисплея из "спящего режима" (включение отображения дисплея и подсветки)
void LCD_SleepOut(LCD_Handler* lcd)
{
	if (lcd->SleepOut_callback) {
		LCD_String_Interpretator(lcd, lcd->SleepOut_callback());
	}
	//включение подсветки
	LCD_SetBackLight(lcd, lcd->bkl_data.bk_percent);
}

#if LCD_USE_ASSEMBLER == 0
//коллбэк по прерыванию потока передачи
//этот обработчик необходимо прописать в функциях обработки прерываний в потоках DMA,
//которые используются дисплеями - at32f403a_407_int.c
void Display_TC_Callback(dma_type *dma_x, uint32_t stream)
{
	//сбрасываем флаги прерываний
	dma_x->clr = 1 << ((stream - 1) * 4);
	LCD_Handler *lcd = LCD; //указатель на первый дисплей в списке
	uint32_t stream_ct = 0;
	dma_type *dma_ct = 0;
	//проходим по списку дисплеев (пока есть следующий в списке)
	while (lcd) {
		//получаем параметры DMA потока дисплея
		dma_ct = lcd->spi_data.dma_tx.dma;
		stream_ct = lcd->spi_data.dma_tx.stream;
		//проверка на соответствие текущего потока DMA потоку, к которому привязан i-тый дисплей
		if (dma_ct == dma_x && stream_ct == stream) {
			if (lcd->spi_data.cs_port) {//управление по cs поддерживается?
				//на выводе cs дисплея высокий уровень?
				if (lcd->spi_data.cs_port->odt & lcd->spi_data.cs_pin) { //проверяем состояние пина выходного регистра порта
					lcd = (LCD_Handler *)lcd->next;		   //если высокий уровень cs, то не этот дисплей активен
					continue;							   //и переходим к следующему
				}
			}
			//указатель на поток: aдрес контроллера + смещение
			dma_channel_type *dma_TX = ((dma_channel_type *)((uint32_t)((uint32_t)dma_x + 8 + 20 * (stream - 1))));
			//выключаем поток DMA
			dma_TX->ctrl_bit.chen = FALSE;
			while (dma_TX->ctrl_bit.chen) ; //ждем отключения потока
			if (lcd->size_mem) { //если переданы не все данные из памяти, то перезапускаем DMA и выходим из прерывания
				if (lcd->size_mem > 65535) {
					dma_TX->dtcnt = 65535;
					lcd->size_mem -= 65535;
				}
				else {
					dma_TX->dtcnt = lcd->size_mem;
					lcd->size_mem = 0;
				}
				//включаем поток DMA
				dma_TX->ctrl_bit.chen = TRUE;
				return;
			}
#ifdef LCD_DYNAMIC_MEM
			//очищаем буфер дисплея
			if (lcd->tmp_buf) {
		    	//так как память выделяется динамически, то на всякий случай,
				//чтобы не было коллизий, запретим прерывания перед освобождением памяти
				DISABLE_IRQ
				free(lcd->tmp_buf);
				lcd->tmp_buf = 0;
				ENABLE_IRQ
			}
#endif
			//запрещаем SPI отправлять запросы к DMA
			lcd->spi_data.spi->ctrl2_bit.dmaten = FALSE;
			while(lcd->spi_data.spi->sts_bit.bf) ; //ждем когда SPI освободится
			//отключаем дисплей от MK (притягиваем вывод CS дисплея к высокому уровню)
			LCD_CS_HI
			//выключаем spi
			lcd->spi_data.spi->ctrl1_bit.spien = FALSE; // SPI выключаем
			return;
		}
		//переходим к следующему дисплею в списке
		lcd = (LCD_Handler *)lcd->next;
	}
}

//установка на дисплее окна вывода
//------------------- 397 тактов ----------------------------
//если интерпретатор использовать, то 616 тактов
//ассемблерный вариант - 371 такт
//для подключения ассемблерного закомментировать функцию
void LCD_SetActiveWindow(LCD_Handler* lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	//этот вариант 616 тактов
	//LCD_String_Interpretator(lcd, lcd->SetActiveWindow_callback(x1 + lcd->x_offs, y1 + lcd->y_offs, x2 + lcd->x_offs, y2 + lcd->y_offs));

	x1 += lcd->x_offs;
	x2 += lcd->x_offs;
	y1 += lcd->y_offs;
	y2 += lcd->y_offs;
	spi_type *spi = lcd->spi_data.spi;
	while (spi->ctrl1_bit.spien) ; //ожидание освобождения spi
	spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //8-битная передача
	spi->ctrl1_bit.spien = TRUE; //SPI включаем
	LCD_CS_LOW
	LCD_DC_LOW
	*((volatile uint8_t *)&spi->dt) = 0x2a;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится

	LCD_DC_HI
	*((volatile uint8_t *)&spi->dt) = x1 >> 8;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = x1 & 0xff;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = x2 >> 8;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = x2 & 0xff;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится

	LCD_DC_LOW
	*((volatile uint8_t *)&spi->dt) = 0x2b;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится

	LCD_DC_HI
	*((volatile uint8_t *)&spi->dt) = y1 >> 8;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = y1 & 0xff;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = y2 >> 8;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	*((volatile uint8_t *)&spi->dt) = y2 & 0xff;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится

	LCD_DC_LOW
	*((volatile uint8_t *)&spi->dt) = 0x2c;
	while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
	while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится

	LCD_CS_HI
	spi->ctrl1_bit.spien = FALSE; // SPI выключаем
}

//вывод блока данных на дисплей
void LCD_WriteData(LCD_Handler *lcd, uint16_t *data, uint32_t len)
{
	spi_type *spi = lcd->spi_data.spi;
	while (spi->ctrl1_bit.spien) ; //ожидание освобождения spi
	spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //8-битная передача
	if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
		spi->ctrl1_bit.fbn = SPI_FRAME_16BIT; //16-битная передача
	}
	spi->ctrl1_bit.spien = TRUE; //SPI включаем
	LCD_CS_LOW
	LCD_DC_HI
	if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
		while (len--) {
			spi->dt = *data++; //записываем данные в регистр
			while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
		}
	}
	else {
		len *= 2;
		uint8_t *data1 = (uint8_t*)data;
		while (len--)	{
			*((volatile uint8_t *)&spi->dt) = *data1++; //записываем данные в регистр
			while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
		}
	}
	while(spi->sts_bit.bf) ; //ждем когда SPI освободится
	LCD_CS_HI
	spi->ctrl1_bit.spien = FALSE; //выключаем spi
}

//вывод блока данных на дисплей с DMA
void LCD_WriteDataDMA(LCD_Handler *lcd, uint16_t *data, uint32_t len)
{
	if (lcd->spi_data.dma_tx.dma) {
		if (lcd->data_bus == LCD_DATA_8BIT_BUS) {
			len *= 2;
		}
		spi_type *spi = lcd->spi_data.spi;
		while (spi->ctrl1_bit.spien) ; //ожидание освобождения spi
		lcd->size_mem = len;
		spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //8-битная передача
		if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
			spi->ctrl1_bit.fbn = SPI_FRAME_16BIT; //16-битная передача
		}
		spi->ctrl1_bit.spien = TRUE; //SPI включаем
		//разрешаем spi отправлять запросы к DMA
		spi->ctrl2_bit.dmaten = TRUE;
		LCD_CS_LOW
		LCD_DC_HI
		dma_type *dma_x = lcd->spi_data.dma_tx.dma;
		uint32_t stream = lcd->spi_data.dma_tx.stream;
		dma_channel_type *dma_TX = ((dma_channel_type *)((uint32_t)((uint32_t)dma_x + 8 + 20 * (stream - 1))));
		//сбрасываем флаги прерываний tx
		dma_x->clr = 1 << ((stream - 1) * 4);
		//настраиваем адреса, длину, инкременты
		dma_TX->paddr = (uint32_t)(&spi->dt); 	//приемник периферия - адрес регистра DR spi
		dma_TX->maddr = (uint32_t)data;			//источник память - адрес буфера исходящих данных
		dma_TX->ctrl_bit.pincm = DMA_PERIPHERAL_INC_DISABLE;	//инкремент адреса периферии отключен
		dma_TX->ctrl_bit.mincm = DMA_MEMORY_INC_ENABLE;			//инкремент адреса памяти включен
		if (len <= 65535) {
			dma_TX->dtcnt = (uint32_t)len;	//размер передаваемых данных
			lcd->size_mem = 0;
		}
		else {
			dma_TX->dtcnt = 65535;
			lcd->size_mem = len - 65535;
		}
		dma_TX->ctrl_bit.chen = TRUE;	//включение потока передачи (старт DMA передачи)
		return;
	}
	LCD_WriteData(lcd, data, len);
}

void LCD_FillWindow(LCD_Handler* lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color)
{
	uint16_t tmp;
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
	if (x1 > lcd->Width - 1 || y1 > lcd->Height - 1) return;
	if (x2 > lcd->Width - 1)  x2 = lcd->Width - 1;
	if (y2 > lcd->Height - 1) y2 = lcd->Height - 1;
	uint32_t len = (x2 - x1 + 1) * (y2 - y1 + 1); //количество закрашиваемых пикселей
	LCD_SetActiveWindow(lcd, x1, y1, x2, y2);
	uint16_t color16 = lcd->fill_color = LCD_Color_24b_to_16b(lcd, color);
	spi_type *spi = lcd->spi_data.spi;
	spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //8-битная передача
	if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
		spi->ctrl1_bit.fbn = SPI_FRAME_16BIT; //16-битная передача
	}
	spi->ctrl1_bit.spien = TRUE; //SPI включаем
	LCD_CS_LOW
	LCD_DC_HI
	if (lcd->spi_data.dma_tx.dma)
	{
		if (lcd->data_bus == LCD_DATA_8BIT_BUS)	{
			len *= 2;
		}
		dma_type *dma_x = lcd->spi_data.dma_tx.dma;
		uint32_t stream = lcd->spi_data.dma_tx.stream;
		dma_channel_type *dma_TX = ((dma_channel_type *)((uint32_t)((uint32_t)dma_x + 8 + 20 * (stream - 1))));
		//сбрасываем флаги прерываний tx
		dma_x->clr = 1 << ((stream - 1) * 4);
		//разрешаем spi отправлять запросы к DMA
		spi->ctrl2_bit.dmaten = TRUE;
		//настраиваем адреса, длину, инкременты
		dma_TX->paddr = (uint32_t)(&spi->dt); 	//приемник периферия - адрес регистра DR spi
		dma_TX->maddr = (uint32_t)&lcd->fill_color;	//источник память - адрес буфера исходящих данных
		dma_TX->ctrl_bit.pincm = DMA_PERIPHERAL_INC_DISABLE;	//инкремент адреса периферии отключен
		dma_TX->ctrl_bit.mincm = DMA_MEMORY_INC_DISABLE;		//инкремент адреса памяти отключен
		if (len <= 65535) {
			dma_TX->dtcnt = (uint32_t)len; //размер передаваемых данных
			lcd->size_mem = 0;
		}
		else {
			dma_TX->dtcnt = 65535;
			lcd->size_mem = len - 65535;
		}
		dma_TX->ctrl_bit.chen = TRUE;	//включение потока передачи (старт DMA передачи)
		return;
	}
	if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
		while(len--) {
			spi->dt = color16; //записываем данные в регистр
			while(!spi->sts_bit.tdbe) ;	//ждем окончания передачи
		}
	}
	else {
		uint8_t color1 = color16 & 0xFF, color2 = color16 >> 8;
		while(len--) {
			*((volatile uint8_t *)&spi->dt) = color1;
			while(!spi->sts_bit.tdbe) ;	//ждем окончания передачи
			*((volatile uint8_t *)&spi->dt) = color2;
			while(!spi->sts_bit.tdbe) ;	//ждем окончания передачи
		}
	}
	while(spi->sts_bit.bf) ; //ждем когда SPI освободится
	LCD_CS_HI
	//выключаем spi
	spi->ctrl1_bit.spien = FALSE;
}


/*
 * Выводит в заданную область дисплея блок памяти (изображение) по адресу в data:
 * x, y - координата левого верхнего угла области дисплея;
 * w, h - ширина и высота области дисплея;
 * data - указатель на блок памяти (изображение) для вывода на дисплей;
 * dma_use_flag - флаг, определяющий задействование DMA (0 - без DMA, !=0 - с DMA)
 */
void LCD_DrawImage(LCD_Handler* lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data, uint8_t dma_use_flag)
{
	if ((x > lcd->Width - 1) || (y > lcd->Height - 1) || x + w > lcd->Width || y + h > lcd->Height) return;
	LCD_SetActiveWindow(lcd, x, y, x + w - 1, y + h - 1);
	if (dma_use_flag) {
		LCD_WriteDataDMA(lcd, data, w * h);
	}
	else {
		LCD_WriteData(lcd, data, w * h);
	}
}

#endif

/* Закрашивает весь дисплей заданным цветом */
void LCD_Fill(LCD_Handler* lcd, uint32_t color)
{
	LCD_FillWindow(lcd, 0, 0, lcd->Width - 1, lcd->Height - 1, color);
}

/* Рисует точку в заданных координатах */
void LCD_DrawPixel(LCD_Handler* lcd, int16_t x, int16_t y, uint32_t color)
{
	if (x > lcd->Width - 1 || y > lcd->Height - 1 || x < 0 || y < 0)	return;
	LCD_SetActiveWindow(lcd, x, y, x, y);
	uint16_t color1 = LCD_Color_24b_to_16b(lcd, color);
	LCD_WriteData(lcd, &color1, 1);
}

/*
 * Рисует линию по координатам двух точек
 * Горизонтальные и вертикальные линии рисуются очень быстро
 */
void LCD_DrawLine(LCD_Handler* lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
{
	if(x0 == x1 || y0 == y1) {
		int16_t tmp;
		if (x0 > x1) { tmp = x0; x0 = x1; x1 = tmp; }
		if (y0 > y1) { tmp = y0; y0 = y1; y1 = tmp; }
		if (x1 < 0 || x0 > lcd->Width - 1)  return;
		if (y1 < 0 || y0 > lcd->Height - 1) return;
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		LCD_FillWindow(lcd, x0, y0, x1, y1, color);
		return;
	}
	int16_t swap;
    uint16_t steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep) {
		swap = x0;
		x0 = y0;
		y0 = swap;

		swap = x1;
		x1 = y1;
		y1 = swap;
    }

    if (x0 > x1) {
		swap = x0;
		x0 = x1;
		x1 = swap;

		swap = y0;
		y0 = y1;
		y1 = swap;
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            LCD_DrawPixel(lcd, y0, x0, color);
        } else {
            LCD_DrawPixel(lcd, x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

/* Рисует прямоугольник по координатам левого верхнего и правого нижнего углов */
void LCD_DrawRectangle(LCD_Handler* lcd, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
	LCD_DrawLine(lcd, x1, y1, x2, y1, color);
	LCD_DrawLine(lcd, x1, y1, x1, y2, color);
	LCD_DrawLine(lcd, x1, y2, x2, y2, color);
	LCD_DrawLine(lcd, x2, y1, x2, y2, color);
}

/* Рисует закрашенный прямоугольник по координатам левого верхнего и правого нижнего углов */
void LCD_DrawFilledRectangle(LCD_Handler* lcd, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
	int16_t tmp;
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
	if (x2 < 0 || x1 > lcd->Width - 1)  return;
	if (y2 < 0 || y1 > lcd->Height - 1) return;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	LCD_FillWindow(lcd, x1, y1, x2, y2, color);
}

/* Рисует треугольник по координатам трех точек */
void LCD_DrawTriangle(LCD_Handler* lcd, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint32_t color)
{
	LCD_DrawLine(lcd, x1, y1, x2, y2, color);
	LCD_DrawLine(lcd, x2, y2, x3, y3, color);
	LCD_DrawLine(lcd, x3, y3, x1, y1, color);
}

/* Виды пересечений отрезков */
typedef enum {
	LINES_NO_INTERSECT = 0, //не пересекаются
	LINES_INTERSECT,		//пересекаются
	LINES_MATCH				//совпадают (накладываются)
} INTERSECTION_TYPES;

/*
 * Определение вида пересечения и координат (по оси х) пересечения отрезка с координатами (x1,y1)-(x2,y2)
 * с горизонтальной прямой y = y0
 * Возвращает один из видов пересечения типа INTERSECTION_TYPES, а в переменных x_min, x_max - координату
 * либо диапазон пересечения (если накладываются).
 * В match инкрементирует количество накладываний (считаем результаты со всех нужных вызовов)
 */
static INTERSECTION_TYPES LinesIntersection(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t y0, int16_t *x_min, int16_t *x_max, uint8_t *match)
{
	if (y1 == y2) { //Частный случай - отрезок параллелен оси х
		if (y0 == y1) { //Проверка на совпадение
			*x_min = min(x1, x2);
			*x_max = max(x1, x2);
			(*match)++;
			return LINES_MATCH;
		}
		return LINES_NO_INTERSECT;
	}
	if (x1 == x2) { //Частный случай - отрезок параллелен оси y
		if (min(y1, y2) <= y0 && y0 <= max(y1, y2)) {
			*x_min = *x_max = x1;
			return LINES_INTERSECT;
		}
		return LINES_NO_INTERSECT;
	}
	//Определяем точку пересечения прямых (уравнение прямой получаем из координат точек, задающих отрезок)
	*x_min = *x_max = (x2 - x1) * (y0 - y1) / (y2 - y1) + x1;
	if (min(x1, x2) <= *x_min && *x_min <= max(x1, x2)) { //Если координата x точки пересечения принадлежит отрезку,
		return LINES_INTERSECT;							  //то есть пересечение
	}
	return LINES_NO_INTERSECT;
}

/* Рисует закрашенный треугольник по координатам трех точек */
void LCD_DrawFilledTriangle(LCD_Handler* lcd, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint32_t color)
{
	//Сортируем координаты в порядке возрастания y
	int16_t tmp;
	if (y1 > y2) {
		tmp = y1; y1 = y2; y2 = tmp;
		tmp = x1; x1 = x2; x2 = tmp;
	}
	if (y1 > y3) {
		tmp = y1; y1 = y3; y3 = tmp;
		tmp = x1; x1 = x3; x3 = tmp;
	}
	if (y2 > y3) {
		tmp = y2; y2 = y3; y3 = tmp;
		tmp = x2; x2 = x3; x3 = tmp;
	}
	//Проверяем, попадает ли треугольник в область вывода
	if (y1 > lcd->Height - 1 ||	y3 < 0) return;
	int16_t xmin = min3(x1, x2, x3);
	int16_t xmax = max3(x1, x2, x3);
	if (xmax < 0 || xmin > lcd->Width - 1) return;
	uint8_t c_mas, match;
	int16_t x_mas[8], x_min, x_max;
	//"Обрезаем" координаты, выходящие за рабочую область дисплея
	int16_t y_start = y1 < 0 ? 0: y1;
	int16_t y_end = y3 > lcd->Height - 1 ? lcd->Height - 1: y3;
	//Проходим в цикле по точкам диапазона координаты y и ищем пересечение отрезка y = y[i] (где y[i]=y1...y3, 1)
	//со сторонами треугольника
	for (int16_t y = y_start; y < y_end; y++) {
		c_mas = match = 0;
		if (LinesIntersection(x1, y1, x2, y2, y, &x_mas[c_mas], &x_mas[c_mas + 1], &match)) {
			c_mas += 2;
		}
		if (LinesIntersection(x2, y2, x3, y3, y, &x_mas[c_mas], &x_mas[c_mas + 1], &match)) {
			c_mas += 2;
		}
		if (LinesIntersection(x3, y3, x1, y1, y, &x_mas[c_mas], &x_mas[c_mas + 1], &match)) {
			c_mas += 2;
		}
		if (!c_mas) continue;
		x_min = x_max = x_mas[0];
		while (c_mas) {
			x_min = min(x_min, x_mas[c_mas - 2]);
			x_max = max(x_max, x_mas[c_mas - 1]);
			c_mas -= 2;
		}
		LCD_DrawLine(lcd, x_min, y, x_max, y, color);
	}
}

/* Рисует окружность с заданным центром и радиусом */
void LCD_DrawCircle(LCD_Handler* lcd, int16_t x0, int16_t y0, int16_t r, uint32_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DrawPixel(lcd, x0, y0 + r, color);
	LCD_DrawPixel(lcd, x0, y0 - r, color);
	LCD_DrawPixel(lcd, x0 + r, y0, color);
	LCD_DrawPixel(lcd, x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DrawPixel(lcd, x0 + x, y0 + y, color);
		LCD_DrawPixel(lcd, x0 - x, y0 + y, color);
		LCD_DrawPixel(lcd, x0 + x, y0 - y, color);
		LCD_DrawPixel(lcd, x0 - x, y0 - y, color);

		LCD_DrawPixel(lcd, x0 + y, y0 + x, color);
		LCD_DrawPixel(lcd, x0 - y, y0 + x, color);
		LCD_DrawPixel(lcd, x0 + y, y0 - x, color);
		LCD_DrawPixel(lcd, x0 - y, y0 - x, color);
	}
}

/* Рисует закрашенную окружность с заданным центром и радиусом */
void LCD_DrawFilledCircle(LCD_Handler* lcd, int16_t x0, int16_t y0, int16_t r, uint32_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DrawLine(lcd, x0 - r, y0, x0 + r, y0, color);

	while (x < y) {
		if (f >= 0)	{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DrawLine(lcd, x0 - x, y0 + y, x0 + x, y0 + y, color);
		LCD_DrawLine(lcd, x0 + x, y0 - y, x0 - x, y0 - y, color);

		LCD_DrawLine(lcd, x0 + y, y0 + x, x0 - y, y0 + x, color);
		LCD_DrawLine(lcd, x0 + y, y0 - x, x0 - y, y0 - x, color);
	}
}

/*
 * Вывод на дисплей символа с кодом в ch, с начальными координатами координатам (x, y), шрифтом font, цветом color,
 * цветом окружения bgcolor.
 * modesym - определяет, как выводить символ:
 *    LCD_SYMBOL_PRINT_FAST - быстрый вывод с полным затиранием знакоместа;
 *    LCD_SYMBOL_PRINT_PSETBYPSET - вывод символа по точкам, при этом цвет окружения bgcolor игнорируется (режим наложения).
 * Ширина символа до 32 пикселей (4 байта на строку). Высота символа библиотекой не ограничивается.
 */
void LCD_WriteChar(LCD_Handler* lcd, uint16_t x, uint16_t y, char ch, FontDef *font, uint32_t txcolor, uint32_t bgcolor, LCD_PrintSymbolMode modesym)
{
	int i, j, k;
	uint32_t tmp = 0;
	const uint8_t *b = font->data;
	uint16_t color;
	uint16_t txcolor16 = LCD_Color_24b_to_16b(lcd, txcolor);
	uint16_t bgcolor16 = LCD_Color_24b_to_16b(lcd, bgcolor);
	ch = ch < font->firstcode || ch > font->lastcode ? 0: ch - font->firstcode;
	int bytes_per_line = ((font->width - 1) >> 3) + 1;
	if (bytes_per_line > 4) { //Поддержка ширины символов до 32 пикселей (4 байта на строку)
		return;
	}
	k = 1 << ((bytes_per_line << 3) - 1);
	b += ch * bytes_per_line * font->height;
	spi_type *spi = lcd->spi_data.spi;
	if (modesym == LCD_SYMBOL_PRINT_FAST) {
		LCD_SetActiveWindow(lcd, x, y, x + font->width - 1, y + font->height - 1);
		LCD_CS_LOW
		LCD_DC_HI
		spi->ctrl1_bit.fbn = SPI_FRAME_8BIT; //установим 8-битную передачу
		if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
			spi->ctrl1_bit.fbn = SPI_FRAME_16BIT; //16-битная передача
		}
		spi->ctrl1_bit.spien = TRUE; // SPI включаем
		for (i = 0; i < font->height; i++) {
			if (bytes_per_line == 1)      { tmp = *((uint8_t*)b);  }
			else if (bytes_per_line == 2) { tmp = *((uint16_t*)b); }
			else if (bytes_per_line == 3) { tmp = (*((uint8_t*)b)) | ((*((uint8_t*)(b + 1))) << 8) |  ((*((uint8_t*)(b + 2))) << 16); }
			else { tmp = *((uint32_t*)b); }
			b += bytes_per_line;
			for (j = 0; j < font->width; j++)
			{
				color = (tmp << j) & k ? txcolor16: bgcolor16;
				while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
				if (lcd->data_bus == LCD_DATA_16BIT_BUS) {
					spi->dt = color;
				}
				else {
					uint8_t color1 = color & 0xFF, color2 = color >> 8;
					*((volatile uint8_t *)&spi->dt) = color1;
					while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
					*((volatile uint8_t *)&spi->dt) = color2;
				}
			}
		}
		while(!spi->sts_bit.tdbe) ; //ждем окончания передачи
		while(spi->sts_bit.bf) ; 	//ждем когда SPI освободится
		//выключаем spi
		spi->ctrl1_bit.spien = FALSE;
		LCD_CS_HI
	}
	else {
		for (i = 0; i < font->height; i++) {
			if (bytes_per_line == 1) { tmp = *((uint8_t*)b); }
			else if (bytes_per_line == 2) { tmp = *((uint16_t*)b); }
			else if (bytes_per_line == 3) { tmp = (*((uint8_t*)b)) | ((*((uint8_t*)(b + 1))) << 8) |  ((*((uint8_t*)(b + 2))) << 16); }
			else if (bytes_per_line == 4) { tmp = *((uint32_t*)b); }
			b += bytes_per_line;
			for (j = 0; j < font->width; j++) {
				if ((tmp << j) & k) {
					LCD_DrawPixel(lcd, x + j, y + i, txcolor);
				}
			}
		}
	}
}

//вывод строки str текста с позиции x, y, шрифтом font, цветом букв color, цветом окружения bgcolor
//modesym - определяет, как выводить текст:
//LCD_SYMBOL_PRINT_FAST - быстрый вывод с полным затиранием знакоместа
//LCD_SYMBOL_PRINT_PSETBYPSET - вывод по точкам, при этом цвет окружения bgcolor игнорируется (позволяет накладывать надписи на картинки)
void LCD_WriteString(LCD_Handler* lcd, uint16_t x, uint16_t y, const char *str, FontDef *font, uint32_t color, uint32_t bgcolor, LCD_PrintSymbolMode modesym)
{
	while (*str) {
		if (x + font->width > lcd->Width) {
			x = 0;
			y += font->height;
			if (y + font->height > lcd->Height) {
				break;
			}
		}
		LCD_WriteChar(lcd, x, y, *str, font, color, bgcolor, modesym);
		x += font->width;
		str++;
	}
	lcd->AtPos.x = x;
	lcd->AtPos.y = y;
}

inline uint16_t LCD_Color (LCD_Handler *lcd, uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t color = ((r & 0xF8) << 8) | ((g & 0xF8) << 3) | (b >> 3);
	if (lcd->data_bus == LCD_DATA_8BIT_BUS) {//8-битная передача
		color = (color >> 8) | ((color & 0xFF) << 8);
	}
	return color;
}

inline uint16_t LCD_Color_24b_to_16b(LCD_Handler *lcd, uint32_t color)
{
	uint8_t r = (color >> 16) & 0xff;
	uint8_t g = (color >> 8) & 0xff;
	uint8_t b = color & 0xff;
	return LCD_Color(lcd, r, g, b);
}
