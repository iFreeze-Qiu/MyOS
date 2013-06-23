/* dsctbl.c
 * ����GDT,IDT��
 * descriptor table�Ĵ���
 * 
 *
 *����GDT 
 * sd��GDT����ַ 
 * limit�����޳�  
 * base:��ָ���������ݶε�ַ 
 * ar:�ι�������  00000000(0x00)��δʹ�õļ�¼�� 
 *       10010010(0x92)��os�ã��ɶ�д�Σ�����ִ�У�ring0 
 *       10011010(0x9a)��os�ã���ִ�жΣ��ɶ�����д��ring0 
 *       11110010(0xf2)��app�ã��ɶ�д������ִ�У�ring3 
 *       11111010(0xfa)��app�ã���ִ�У��ɶ�����д��ring3 
 */

#include "bootpack.h"

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
