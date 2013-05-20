; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; ����Ŀ���ļ���ģʽ	
[INSTRSET "i486p"]				; ��ʾ������486����ʹ��32λ�Ĵ���
[BITS 32]						; ����32λģʽ�õĻ�������


; ����Ŀ���ļ�����Ϣ

[FILE "naskfunc.nas"]			; Դ�ļ�����Ϣ

		GLOBAL	_io_hlt,_write_mem8			; �����а����ĺ���


; ʵ�ʳ���

[SECTION .text]		; Ŀ���ļ���д����Щ����д����

_io_hlt:	; void io_hlt(void);
		HLT
		RET


_write_mem8:				; ��ͬ��C���Ե�void write_mem8(int addr,int data)
	MOV		ECX,[ESP+4]		; ��ַaddr��ֵ��ECX
	MOV		AL,[ESP+8]		; ����data����AL
	MOV		[ECX],AL		; AL�����ݷŵ�addr��
	RET
