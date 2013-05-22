#include <stdio.h>
void io_hlt(void);					//
void io_cli(void);					//��ֹ�ж�
void io_out8(int port, int data);	//8λ����
int io_load_eflags(void);			//��¼�ж���ɱ�־��ֵ
void io_store_eflags(int eflags);	//�����ж���ɱ�־
/*
 *���Ϻ�����naskfunc.nas��
 */
void init_palette(void);//��ɫ��
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);//�滭����
void init_screen(char *vram, int x, int y);//���ƽ���
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);//��ʾ�ַ�
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);//����ַ���
void init_mouse_cursor8(char *mouse, char bc);//׼�����
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);//��ȡ����ɫ

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};//�ṹ�屣��������Ϣ
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};//GDT��8�ֽ�����

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};//IDT��8�ֽ�����



void init_gdtidt(void);//��ʼ��gdt��idt
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);//��ֵ���������û��
void load_idtr(int limit, int addr);//��ֵ���������û��

void HariMain(void)
{
	
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[40], mcursor[256];
	int mx, my;

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* �������ָ��ĳ�ʼλ�� */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:�� */
		0xff, 0x00, 0x00,	/*  1:���� */
		0x00, 0xff, 0x00,	/*  2:���� */
		0xff, 0xff, 0x00,	/*  3:���� */
		0x00, 0x00, 0xff,	/*  4:���� */
		0xff, 0x00, 0xff,	/*  5:���� */
		0x00, 0xff, 0xff,	/*  6:ǳ���� */
		0xff, 0xff, 0xff,	/*  7:�� */
		0xc6, 0xc6, 0xc6,	/*  8:���� */
		0x84, 0x00, 0x00,	/*  9:���� */
		0x00, 0x84, 0x00,	/* 10:���� */
		0x84, 0x84, 0x00,	/* 11:���� */
		0x00, 0x00, 0x84,	/* 12:���� */
		0x84, 0x00, 0x84,	/* 13:���� */
		0x00, 0x84, 0x84,	/* 14:ǳ���� */
		0x84, 0x84, 0x84	/* 15:���� */
	};
	set_palette(0, 15, table_rgb);//���ú������õ�ɫ��
	return;

}
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* ��¼�ж���ɱ�־��ֵ */
	io_cli(); 					/* ���ж���ɱ�־��Ϊ0����ʾ��ֹ�ж� */
	io_out8(0x03c8, start);		//0x03c8Ϊ�豸����
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);	//0x03c9Ϊ�豸����
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* ��ԭ�ж���ɱ�־ */
	return;
}
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}
void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}
void init_mouse_cursor8(char *mouse, char bc)
/* ׼�����ָ�루16*16�� */
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;//��Ե��ɫ
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;//����м��ɫ
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;//����ǡ�.������ʾ����ɫ
			}
		}
	}
	return;
}
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;//����û�б�ռ�õ��ڴ�ռ�
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;//����û�б�ռ�õ��ڴ�ռ�
	int i;

	/* GDT�ĳ�ʼ�� */
	for (i = 0; i < 8192; i++) {//��ѡ������12λ��ʾ��ֵ�������8192 
		set_segmdesc(gdt + i, 0, 0, 0);//ʵ�����ڲ�����ת����gdt+i*8������ȫ����ʼ��Ϊ0
	}
	//0�Ŷα���NULL Description  
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);//cpu�ܹ����ȫ��4G�ڴ� os���ݶ�
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);//bootpack.hrb����512KB�ڴ� os�����
	load_gdtr(0xffff, 0x00270000);//��GDTR��ֵ����GDT���׵�ַ���ص�GDTR  

	/* IDT�ĳ�ʼ�� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

/* 
* ����GDT 
* sd��GDT����ַ 
* limit�����޳�  
* base:��ָ���������ݶε�ַ 
* ar:�ι�������  00000000(0x00)��δʹ�õļ�¼�� 
*       10010010(0x92)��os�ã��ɶ�д�Σ�����ִ�У�ring0 
*       10011010(0x9a)��os�ã���ִ�жΣ��ɶ�����д��ring0 
*       11110010(0xf2)��app�ã��ɶ�д������ִ�У�ring3 
*       11111010(0xfa)��app�ã���ִ�У��ɶ�����д��ring3 
*/  

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
