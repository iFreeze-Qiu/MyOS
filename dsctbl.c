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
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;//����û�б�ռ�õ��ڴ�ռ�
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;//����û�б�ռ�õ��ڴ�ռ�
	int i;

	/* GDT�ĳ�ʼ�� */
	for (i = 0; i < 8192; i++) {//��ѡ������12λ��ʾ��ֵ�������8192 
		set_segmdesc(gdt + i, 0, 0, 0);//ʵ�����ڲ�����ת����gdt+i*8������ȫ����ʼ��Ϊ0
	}
	//0�Ŷα���NULL Description  
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);//cpu�ܹ����ȫ��4G�ڴ� os���ݶ�
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);//bootpack.hrb����512KB�ڴ� os�����
	load_gdtr(LIMIT_GDT, ADR_GDT);//��GDTR��ֵ����GDT���׵�ַ���ص�GDTR  

	/* IDT�ĳ�ʼ�� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	/* IDT���趨 */
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);

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
