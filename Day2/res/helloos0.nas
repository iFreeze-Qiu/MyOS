; hello-os
; TAB=4

		ORG		0x7c00			; ָ��װ�ص�ַ

; ���±�עFAT12��ʽ����

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		
		DW		512				
		DB		1				
		DW		1				
		DB		2				
		DW		224			
		DW		2880		
		DB		0xf0			
		DW		9				
		DW		18				
		DW		2				
		DD		0				
		DD		2880			
		DB		0,0,0x29		
		DD		0xffffffff		
		DB		"HELLO-OS   "	
		DB		"FAT12   "		
		RESB	18				

; �������

entry:							; ��ǩ
		MOV		AX,0			; ��ʼ���Ĵ��� ע��AX�ۼӼĴ���	BX��ַ�Ĵ���	CX�����Ĵ���	DX���ݼĴ���
		MOV		SS,AX 			; SSջ�μĴ���
		MOV		SP,0x7c00		; SPջָ��Ĵ���
		MOV		DS,AX 			; DS���ݶμĴ���
		MOV		ES,AX 			; ES���ӶμĴ���

		MOV		SI,msg			; Դ��ַ�Ĵ���
putloop:
		MOV		AL,[SI]
		ADD		SI,1			
		CMP		AL,0 			; �Ƚ�AL��0�������ж��Ƿ�ִ��JE�������ִ��JE
		JE		fin 			; ���AL==0,����ת��fin�������������ָ��  ע��finΪ�������
		MOV		AH,0x0e			; ��ʾһ������
		MOV		BX,15			; ָ���ַ���ɫ
		INT		0x10			; �����Կ�BOIS  ע:INT�жϣ���������BIOS����
		JMP		putloop			; ��ת
fin:
		HLT						; ֹͣCPU���ȴ�ָ��
		JMP		fin				; ����ѭ��

msg:
		DB		0x0a, 0x0a		; ����
		DB		"hello, world"
		DB		0x0a			; ����
		DB		"BY Shen Jinsheng"
		DB		0

		RESB	0x7dfe-$		

		DB		0x55, 0xaa

; �������

		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432
