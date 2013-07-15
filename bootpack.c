/*
 *bootpack.c
 *��������
 */
#include "bootpack.h"
#include <stdio.h>

extern struct KEYBUF keybuf;
 
void HariMain(void)
{
	
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[40], mcursor[256];
	int mx, my, i;
	
	init_gdtidt();
	init_pic();
	io_sti(); 

	io_out8(PIC0_IMR, 0xf9); /* PIC1����ȫ����ֹ(11111001) */
	io_out8(PIC1_IMR, 0xef); 

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* �������ָ��ĳ�ʼλ�� */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 

	for (;;) {
		io_cli();	//�����ж�
		if (/*keybuf.flag == 0*/keybuf.len == 0) {
			io_stihlt();	//ȡ�������жϲ�ִ��HLT
		} else {
			//���������Ϣ
			/*i = keybuf.data;
			keybuf.flag = 0;*/
			i = keybuf.data[keybuf.next_r];
			keybuf.len--;
			keybuf.next_r++;
			if (keybuf.next_r==32)
			{
				keybuf.next_r = 0;
			}
			io_sti();
			sprintf(s, "%02X", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
}
