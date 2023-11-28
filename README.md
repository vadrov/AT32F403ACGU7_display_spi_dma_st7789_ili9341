# AT32F403ACGU7_display_spi_dma
 Connecting the displays to the AT32F403A MCU via spi with DMA.

AT32F403A high-performance microcontrollers, powered by 32-bit ARM® Cortex®-M4 core, utilize advanced process to achieve 240 MHz computing speed. The embedded single precision floating-point unit (FPU) and digital signal processor (DSP), rich peripherals and flexible clock control mechanism can meet an extensive range of applications. The superior memory design supports up to 1 MB Flash memory and 224 KB SRAM, with the excellent Flash access zero wait far beyond the same level of the chip industry.
![at32f403acgu7 board](https://github.com/vadrov/AT32F403ACGU7_display_spi_dma_st7789_ili9341/assets/111627147/e2e03925-22c7-4f26-88f4-a398f9c42ef4)

Connection:
```
LCD_SCL ---> PB13
LCD_SDA ---> PB15
LCD_DC  ---> PB6
LCD_RES ---> PB7
LCD_CS  ---> PB8
LCD_BLK ---> PB1
```
See the description of the driver, functions and parameters in the project https://github.com/vadrov/stm32-display-spi-dma
All driver files are located in the Display folder.
Some driver functions are written in assembly language. Assembly functions use variable offsets in the LCD_Handler structure. These offsets are generated automatically by a special makefile script (in the Display folder) when building the project (the header file display_offsets.h is generated). In your project using this display driver, in the project properties (AT32 IDE) C/C++ Build -> Settings -> Build Steps in the Pre-Build Steps option, write:
```
make -f ../Display/makefile
```
The assembler version of the driver can be turned on/off using the LCD_USE_ASSEMBLER macro definition in the header file display_config.h

Смотрите описание драйвера, функций и параметров в проекте https://github.com/vadrov/stm32-display-spi-dma

Все файлы драйвера расположены в папке Display.\
Некоторые функции драйвера написаны на ассемблере. В ассемблерных функциях используются смещения переменных в "Сишной" структуре LCD_Handler. Генерация этих смещений происходит автоматически специальным скриптом makefile (в папке Display) при сборке проекта (формируется заголовочный файл display_offsets.h). Поэтому в своем проекте с использованием этого драйвера дисплея в свойствах С/С++ Build -> Settings -> Build Steps в опции Pre-Build Steps пропишите:
```
make -f ../Display/makefile
```
Ассемблерный вариант драйвера можно включать/выключать с помощью макроопределения LCD_USE_ASSEMBLER в заголовочном файле display_config.h

Контакты: [Youtube](https://www.youtube.com/@VadRov) [Дзен](https://dzen.ru/vadrov) [VK](https://vk.com/vadrov) [Telegram](https://t.me/vadrov_channel)\
Поддержать автора: [donate.qiwi](https://donate.qiwi.com/payin/VadRov)  [donate.yoomoney](https://yoomoney.ru/to/4100117522443917)
