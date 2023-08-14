# AT32F403ACGU7_display_spi_dma
 Connecting the displays to the AT32F403A MCU via spi with DMA.\
Connection:
```
LCD_SCL ---> PB13
LCD_SDA ---> PB15
LCD_DC  ---> PB6
LCD_RES ---> PB7
LCD_CS  ---> PB8
LCD_BLK ---> PB1
```
See the description of the driver, functions and parameters in the project https://github.com/vadrov/stm32-display-spi-dma\
Смотрите описание драйвера, функций и параметров в проекте https://github.com/vadrov/stm32-display-spi-dma\
Все файлы драйвера расположены в папке Display.\
Некоторые функции драйвера написаны на ассемблере. В ассемблерных функциях используются смещения переменных в "Сишной" структуре LCD_Handler. Генерация этих смещений происходит автоматически специальным скриптом makefile (в папке Display) при сборке проекта (формируется заголовочный файл display_offsets.h). Поэтому в своем проекте с использованием этого драйвера дисплея в свойствах С/С++ Build -> Settings -> Build Steps в опции Pre-Build Steps пропишите:
```
make -f ../Display/makefile
```
Ассемблерный вариант драйвера можно включать/выключать с помощью макроопределения LCD_USE_ASSEMBLER в заголовочном файле display_config.h\

Контакты: [Youtube](https://www.youtube.com/@VadRov) [Дзен](https://dzen.ru/vadrov) [VK](https://vk.com/vadrov) [Telegram](https://t.me/vadrov_channel)\
Поддержать автора: [donate.qiwi](https://donate.qiwi.com/payin/VadRov)  [donate.yoomoney](https://yoomoney.ru/to/4100117522443917)
