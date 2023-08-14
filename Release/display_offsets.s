	.cpu cortex-m4
	.arch armv7e-m
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"display_offsets.c"
	.text
	.align	1
	.global	LCD_GenerateOffsets
	.syntax unified
	.thumb
	.thumb_func
	.type	LCD_GenerateOffsets, %function
LCD_GenerateOffsets:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	push	{r7}
	add	r7, sp, #0
	.syntax unified
@ 45 "../Display/display_offsets.c" 1
	
#define lcd_width #4 
@ 0 "" 2
@ 46 "../Display/display_offsets.c" 1
	
#define lcd_height #6 
@ 0 "" 2
@ 47 "../Display/display_offsets.c" 1
	
#define lcd_x_offs #10 
@ 0 "" 2
@ 48 "../Display/display_offsets.c" 1
	
#define lcd_y_offs #12 
@ 0 "" 2
@ 49 "../Display/display_offsets.c" 1
	
#define lcd_spi #36 
@ 0 "" 2
@ 50 "../Display/display_offsets.c" 1
	
#define lcd_dma_ctrl #40 
@ 0 "" 2
@ 51 "../Display/display_offsets.c" 1
	
#define lcd_dma_strm #44 
@ 0 "" 2
@ 52 "../Display/display_offsets.c" 1
	
#define lcd_dc_port #56 
@ 0 "" 2
@ 53 "../Display/display_offsets.c" 1
	
#define lcd_dc_pin #60 
@ 0 "" 2
@ 54 "../Display/display_offsets.c" 1
	
#define lcd_cs_port #64 
@ 0 "" 2
@ 55 "../Display/display_offsets.c" 1
	
#define lcd_cs_pin #68 
@ 0 "" 2
@ 56 "../Display/display_offsets.c" 1
	
#define lcd_sz_mem #96 
@ 0 "" 2
@ 57 "../Display/display_offsets.c" 1
	
#define lcd_cs_ctrl #101 
@ 0 "" 2
@ 58 "../Display/display_offsets.c" 1
	
#define lcd_dc_ctrl #102 
@ 0 "" 2
@ 59 "../Display/display_offsets.c" 1
	
#define lcd_fill_clr #104 
@ 0 "" 2
	.thumb
	.syntax unified
	nop
	mov	sp, r7
	@ sp needed
	pop	{r7}
	bx	lr
	.size	LCD_GenerateOffsets, .-LCD_GenerateOffsets
	.ident	"GCC: (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1 20210824 (release)"
