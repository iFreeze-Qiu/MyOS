; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; ����Ŀ���ļ���ģʽ	
[INSTRSET "i486p"]				; ��ʾ������486����ʹ��32λ�Ĵ���
[BITS 32]						; ����32λģʽ�õĻ�������


; ����Ŀ���ļ�����Ϣ

[FILE "naskfunc.nas"]			; Դ�ļ�����Ϣ

		GLOBAL	_io_hlt			; �����а����ĺ���


; ʵ�ʳ���

[SECTION .text]		; Ŀ���ļ���д����Щ����д����

_io_hlt:	; void io_hlt(void);
		HLT
		RET


