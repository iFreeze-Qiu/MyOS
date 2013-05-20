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

void HariMain(void)
{
	int i; 
	char *p; 
	init_palette();//�趨��ɫ��

	for (i = 0xa0000; i <= 0xaffff; i++) {

		p = (char *)i;//��������ת�� 
		*p = i & 0x0f;

	}

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
