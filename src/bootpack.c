/*
 *bootpack.c
 *其他处理
 */
#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED		0xed		/* 需要发送的LED数据 */


void HariMain(void)
{
	
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;	//公共队列缓冲区管理结构
	struct FIFO32 keycmd;	/* 用来存放向键盘发送的命令 */
	char s[40];	//输出缓冲区
	int fifobuf[128];	//队列缓冲区
	int keycmd_buf[32];
	int mx, my, i;
	int task_b_esp;	//为任务B定义的栈
	unsigned int memtotal;	//记录内存大小
	int count;	//计数
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;	//图层管理结构指针
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];
	struct SHEET *sht_win, *sht_cons;	//添加窗口图层
	unsigned char *buf_win, *buf_cons;		//窗口图层的缓冲区
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0,   '\\', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
		0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
		0,    0,   0,   0x5c,0,   0,   0,   0,    0,   0,   0,   0,   0,   0x5c,   0,   0,
	};
	static char keytable1[0x80] = {
		0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '=', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0,   '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
		0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
		0,    0,   0,   0x5c,0,   0,   0,   0,    0,   0,   0,   0,   0,   0x5c,   0,   0,
	};
	int cursor_x;	//光标位置，每次输入一个字符向后移动8
	int cursor_c;	//光标状态，通知光标闪烁
	struct TIMER *timer;
	struct TASK *task_a, *task_cons;
	int key_to = 0, key_shift = 0;
	int key_leds = (binfo->leds >> 4) & 7;
	int keycmd_wait = -1;


	
	init_gdtidt();	//初始化GDT、IDT
	init_pic();	//初始化PIC
	io_sti(); //结束IDT/PIC初始化，解除CPU中断禁用
	fifo32_init(&fifo, 128 , fifobuf,0);	//队列缓冲区初始化
	init_pit();	//初始化PIT
	init_keyboard(&fifo, 256);	//初始化键盘控制电路
	enable_mouse(&fifo, 512, &mdec);	//激活鼠标
	io_out8(PIC0_IMR, 0xf8); /* PIT和PIC1以外全部禁止(11111000) */
	io_out8(PIC1_IMR, 0xef); //鼠标设置为许可
	fifo32_init(&keycmd, 32, keycmd_buf, 0);	/* 初始化键盘命令缓冲区 */
	
	memtotal = memtest(0x00400000, 0xbfffffff);	//检测4M~3G-1的内存  memtotal是内存实际大小
	memman_init(memman);	//初始化内存管理结构
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff ，这段内存是“安全”的*/
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	//4M到实际内存大小

	init_palette();	//初始化调色板
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);	//初始化图层管理结构
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	
	//sht_back
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 没有透明色 */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level=2, priority=2 */


	//sht_win
	sht_win = sheet_alloc(shtctl);		//将窗口图层添加到窗口管理中
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160*52);		//分配窗口内存
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);	//没有透明色
	make_window8(buf_win, 144, 52, "task_a",1);	//显示窗口函数调用
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);


	//sht_mouse
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);	//透明色号99
	init_mouse_cursor8(buf_mouse, 99);	//背景色号99
	mx = (binfo->scrnx - 16) / 2; /* 按显示在画面中央来计算最终坐标 */
	my = (binfo->scrny - 28 - 16) / 2;


	sheet_slide(sht_back,  0,  0);
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win,  64, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_cons,  1);
	sheet_updown(sht_win,   2);
	sheet_updown(sht_mouse, 3);
	/*sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);*/
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	/* 所有对键盘LED的设置都是对键盘中的一块芯片8048的设置 */
	/* 不是以前设置的i8042,但是通过i8042间接的对8048进行设置 */

	for (;;) 
	{
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) 
		{
			/* 如果存在要向键盘发生的数据，则发送它 */
			keycmd_wait = fifo32_get(&keycmd);	/* 取出数据 */
			wait_KBC_sendready();	/* 清空i8042的输入缓冲区 */
			io_out8(PORT_KEYDAT, keycmd_wait);	/* 向0x60端口发生0xed */
		/* 发生了该命令之后，需要继续向0x60端口发生LED设置字节 */
		/* 在下面等待确认是否有按下控制LED灯的键后来决定LED设置字节的值，然后再发生 */
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			
			if (i <= 511 && i >=256) //键盘数据
			{	
				/*sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);*/
				if (i < 0x80 + 256) //键盘编码转换为字符编码
				{
					if (key_shift ==0)
					{
						s[0] = keytable0[i - 256];
					}
					else
						s[0] = keytable1[i - 256];
				}
				else
					s[0] = 0;
				if ('A' <= s[0] && s[0] <= 'Z')
				{
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0))
					{
						s[0] += 0x20;//将大写字母转换为小写
					}
				}
				if (s[0] != 0)
				{
					if (key_to == 0)	//发送给任务A
					{
						if (cursor_x < 128)
						{
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
						
					}
					else 	//发送到命令行
					{
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}

					

				if (i == 256 + 0x0e )	//退格
				{
					if (key_to ==0)//任务A
					{
						if (cursor_x > 8)
						{
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}
					else
						fifo32_put(&task_cons->fifo, 8 + 256);
					
				}
				if (i == 256 + 0x0f) /* Tab */
				{ 
					if (key_to == 0) 
					{
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;//不显示光标
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);//命令行窗口光标ON
					}
				 	else 
				 	{
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;//显示光标
						fifo32_put(&task_cons->fifo, 3);//命令行窗口光标OFF
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x1c)//回车键
				{
					if (key_to != 0)
					{
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x2a) //左shift按下
				{
					key_shift |= 1;
				}
				if (i == 256 + 0x36) //右shift按下
				{	
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) //左shift松开
				{	
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) //右shift松开 
				{	
					key_shift &= ~2;
				}

				if (i == 256 + 0x3a) 
				{	/* CapsLock */
					key_leds ^= 4;		/* key_leds中标识CapsLock的bit位取反 */
					fifo32_put(&keycmd, KEYCMD_LED);	/* 向i8042发生命令来修改8048 */
					fifo32_put(&keycmd, key_leds);		/* 改变CapsLock等的状态 */
				}
				if (i == 256 + 0x45) 
				{	/* NumLock */
					key_leds ^= 2;		/* 和CapsLock类似的处理 */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) 
				{	/* ScrollLock */
					key_leds ^= 1;		/* 和CapsLock类似的处理 */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* 0xfa是ACK信息 */
				if (i == 256 + 0xfa) 
				{	/* 键盘成功接收到数据 */
					keycmd_wait = -1;	/* 等于-1表示可以发送指令 */
				}
				if (i == 256 + 0xfe) 
				{	/* 键盘没有成功接收到数据 */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);	/* 重新发送上次的指令 */
				}
				
				//重新显示光标
				if (cursor_c >= 0 )
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				
			} 
			else if (i <= 767 && i >=512) //鼠标数据
			{	
				if (mouse_decode(&mdec, i - 512) != 0) 
				{
					
					/*sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) 
					{
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) 
					{
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) 
					{
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);*/
					/* 移动光标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) 
					{
						mx = 0;
					}
					if (my < 0) 
					{
						my = 0;
					}
					if(mx > binfo->scrnx - 1)
    					mx = binfo->scrnx - 1;
					if(my > binfo->scrny - 1)
   	 					my = binfo->scrny - 1;
					/*sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);*/
					sheet_slide(sht_mouse, mx, my); /* 包含sheet_refresh */
					if ((mdec.btn & 0x01) != 0)
					//按下左键，移动窗口
					{
						sheet_slide(sht_win, mx-80, my - 8);
					}
				}
			}
			else if (i <= 1) //光标用计时器
			{
				if (i != 0)
				{
					timer_init(timer, &fifo, 0);	//然后设置0
					if (cursor_c >= 0)	
					{
						cursor_c = COL8_000000;
					}	
				}
				else
				{
					timer_init(timer, &fifo, 1);	//然后设置1
					if (cursor_c >= 0)	
					{
						cursor_c = COL8_FFFFFF;
					}

				}
				timer_settime(timer, 50);
				if (cursor_c >= 0)
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}


