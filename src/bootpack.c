/*
 *bootpack.c
 *��������
 */
#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED		0xed		/* ��Ҫ���͵�LED���� */


void HariMain(void)
{
	
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;	//�������л����������ṹ
	struct FIFO32 keycmd;	/* �����������̷��͵����� */
	char s[40];	//���������
	int fifobuf[128];	//���л�����
	int keycmd_buf[32];
	int mx, my, i;
	int task_b_esp;	//Ϊ����B�����ջ
	unsigned int memtotal;	//��¼�ڴ��С
	int count;	//����
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;	//ͼ������ṹָ��
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];
	struct SHEET *sht_win, *sht_cons;	//���Ӵ���ͼ��
	unsigned char *buf_win, *buf_cons;		//����ͼ��Ļ�����
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
	int cursor_x;	//���λ�ã�ÿ������һ���ַ�����ƶ�8
	int cursor_c;	//���״̬��֪ͨ�����˸
	struct TIMER *timer;
	struct TASK *task_a, *task_cons;
	int key_to = 0, key_shift = 0;
	int key_leds = (binfo->leds >> 4) & 7;
	int keycmd_wait = -1;


	
	init_gdtidt();	//��ʼ��GDT��IDT
	init_pic();	//��ʼ��PIC
	io_sti(); //����IDT/PIC��ʼ�������CPU�жϽ���
	fifo32_init(&fifo, 128 , fifobuf,0);	//���л�������ʼ��
	init_pit();	//��ʼ��PIT
	init_keyboard(&fifo, 256);	//��ʼ�����̿��Ƶ�·
	enable_mouse(&fifo, 512, &mdec);	//�������
	io_out8(PIC0_IMR, 0xf8); /* PIT��PIC1����ȫ����ֹ(11111000) */
	io_out8(PIC1_IMR, 0xef); //�������Ϊ����
	fifo32_init(&keycmd, 32, keycmd_buf, 0);	/* ��ʼ������������� */
	
	memtotal = memtest(0x00400000, 0xbfffffff);	//���4M~3G-1���ڴ�  memtotal���ڴ�ʵ�ʴ�С
	memman_init(memman);	//��ʼ���ڴ�����ṹ
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff ������ڴ��ǡ���ȫ����*/
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	//4M��ʵ���ڴ��С

	init_palette();	//��ʼ����ɫ��
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);	//��ʼ��ͼ������ṹ
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	
	//sht_back
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* û��͸��ɫ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* û��͸��ɫ */
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
	sht_win = sheet_alloc(shtctl);		//������ͼ�����ӵ����ڹ�����
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160*52);		//���䴰���ڴ�
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);	//û��͸��ɫ
	make_window8(buf_win, 144, 52, "task_a",1);	//��ʾ���ں�������
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);


	//sht_mouse
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);	//͸��ɫ��99
	init_mouse_cursor8(buf_mouse, 99);	//����ɫ��99
	mx = (binfo->scrnx - 16) / 2; /* ����ʾ�ڻ��������������������� */
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

	/* ���жԼ���LED�����ö��ǶԼ����е�һ��оƬ8048������ */
	/* ������ǰ���õ�i8042,����ͨ��i8042��ӵĶ�8048�������� */

	for (;;) 
	{
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) 
		{
			/* �������Ҫ����̷��������ݣ������� */
			keycmd_wait = fifo32_get(&keycmd);	/* ȡ������ */
			wait_KBC_sendready();	/* ���i8042�����뻺���� */
			io_out8(PORT_KEYDAT, keycmd_wait);	/* ��0x60�˿ڷ���0xed */
		/* �����˸�����֮����Ҫ������0x60�˿ڷ���LED�����ֽ� */
		/* ������ȴ�ȷ���Ƿ��а��¿���LED�Ƶļ���������LED�����ֽڵ�ֵ��Ȼ���ٷ��� */
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			
			if (i <= 511 && i >=256) //��������
			{	
				/*sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);*/
				if (i < 0x80 + 256) //���̱���ת��Ϊ�ַ�����
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
						s[0] += 0x20;//����д��ĸת��ΪСд
					}
				}
				if (s[0] != 0)
				{
					if (key_to == 0)	//���͸�����A
					{
						if (cursor_x < 128)
						{
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
						
					}
					else 	//���͵�������
					{
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}

					

				if (i == 256 + 0x0e )	//�˸�
				{
					if (key_to ==0)//����A
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
						cursor_c = -1;//����ʾ���
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);//�����д��ڹ��ON
					}
				 	else 
				 	{
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;//��ʾ���
						fifo32_put(&task_cons->fifo, 3);//�����д��ڹ��OFF
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x1c)//�س���
				{
					if (key_to != 0)
					{
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x2a) //��shift����
				{
					key_shift |= 1;
				}
				if (i == 256 + 0x36) //��shift����
				{	
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) //��shift�ɿ�
				{	
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) //��shift�ɿ� 
				{	
					key_shift &= ~2;
				}

				if (i == 256 + 0x3a) 
				{	/* CapsLock */
					key_leds ^= 4;		/* key_leds�б�ʶCapsLock��bitλȡ�� */
					fifo32_put(&keycmd, KEYCMD_LED);	/* ��i8042�����������޸�8048 */
					fifo32_put(&keycmd, key_leds);		/* �ı�CapsLock�ȵ�״̬ */
				}
				if (i == 256 + 0x45) 
				{	/* NumLock */
					key_leds ^= 2;		/* ��CapsLock���ƵĴ��� */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) 
				{	/* ScrollLock */
					key_leds ^= 1;		/* ��CapsLock���ƵĴ��� */
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				/* 0xfa��ACK��Ϣ */
				if (i == 256 + 0xfa) 
				{	/* ���̳ɹ����յ����� */
					keycmd_wait = -1;	/* ����-1��ʾ���Է���ָ�� */
				}
				if (i == 256 + 0xfe) 
				{	/* ����û�гɹ����յ����� */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);	/* ���·����ϴε�ָ�� */
				}
				
				//������ʾ���
				if (cursor_c >= 0 )
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				
			} 
			else if (i <= 767 && i >=512) //�������
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
					/* �ƶ���� */
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
					sheet_slide(sht_mouse, mx, my); /* ����sheet_refresh */
					if ((mdec.btn & 0x01) != 0)
					//����������ƶ�����
					{
						sheet_slide(sht_win, mx-80, my - 8);
					}
				}
			}
			else if (i <= 1) //����ü�ʱ��
			{
				if (i != 0)
				{
					timer_init(timer, &fifo, 0);	//Ȼ������0
					if (cursor_c >= 0)	
					{
						cursor_c = COL8_000000;
					}	
				}
				else
				{
					timer_init(timer, &fifo, 1);	//Ȼ������1
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

