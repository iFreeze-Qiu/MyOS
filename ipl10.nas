; dogged-os
; TAB=4
		CYLS	EQU		10		; ����CYLS=10

		ORG		0x7c00			; ָ��װ�ص�ַ

; ���±�עFAT12��ʽ����

		JMP		entry
		DB		0x90
		DB		"DOGGED  "		;������������ ,8�ֽ�		
		DW		512				;ÿ��������С��������512�ֽ�		
		DB		1				;�صĴ�С
		DW		1				;FAT����ʼλ��		
		DB		2				;FAT�ĸ���	
		DW		224				;��Ŀ¼��С
		DW		2880			;�ô��̴�С
		DB		0xf0			;��������
		DW		9				;FAT����
		DW		18				;1���ŵ��м�������
		DW		2				;��ͷ��������Ϊ2
		DD		0				;��ʹ�÷���
		DD		2880			;��дһ�δ��̴�С
		DB		0,0,0x29		
		DD		0xffffffff		;������
		DB		"HELLO-OS   "	;��������
		DB		"FAT12   "		;���̸�ʽ����
		RESB	18		;�ճ�18���ֽڣ�����Ϊ0

; �������

entry:							; ��ǩ
		MOV		AX,0			; ��ʼ���Ĵ��� ע��AX�ۼӼĴ���	BX��ַ�Ĵ���	CX�����Ĵ���	DX���ݼĴ���
		MOV		SS,AX 			; SSջ�μĴ���
		MOV		SP,0x7c00		; SPջָ��Ĵ���
		MOV		DS,AX 			; DS���ݶμĴ���

; ����
		MOV		AX,0x0820 		; װ�ص��ڴ�0x0820����Ϊ0x0800�Ժ��ڴ�Ϊ����0x0800-0x08ff�Ѿ���iplռ��
		MOV		ES,AX
		MOV		CH,0			; ����0
		MOV		DH,0			; ��ͷ0
		MOV		CL,2			; ����2
readloop:
		MOV 	SI,0 			; SI�Ĵ�����¼ʧ�ܴ���

retry:


		MOV		AH,0x02			; AH=0x02 : ����
		MOV		AL,1			; 1������
		MOV		BX,0
		MOV		DL,0x00			; A������
		INT		0x13			; ���ô���BIOS
		JNC 	next 			; û�д�������ת��next
		ADD 	SI,1 			; SI��һ
		CMP 	SI,5 			; �Ƚ�SI��5�����ڵ���5����ת��error
		JAE 	error 			
		MOV		AH,0x00
		MOV 	DL,0x00 		; A������
		INT 	0x13 			; ��������
		JMP 	retry
next:
		MOV		AX,ES			; ���ڴ��ַ����0x200
		ADD		AX,0x0020
		MOV		ES,AX
		ADD 	CL,1 			; CL��һ�����бȽ�
		CMP		CL,18
		JBE		readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2 			; DHС��2����ת
		JB 		readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB 		readloop




; ���ô���

		MOV		[0x0ff0],CH		; IPLװ�ص�ַ����dogged.sys
		JMP		0xc200

error:
		MOV		SI,msg		
;success:
;		MOV		SI,msg	
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


������Ϊ�����Ϣ


msg:
		DB		0x0a,0x0a			; ����
		DB 		"Load error"
		DB 		0x0a
		DB		"BY Shen Jinsheng"
		DB		0

		RESB	0x7dfe-$		

		DB		0x55, 0xaa
