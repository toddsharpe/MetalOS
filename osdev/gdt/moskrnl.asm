Microsoft (R) COFF/PE Dumper Version 14.22.27905.0
Copyright (C) Microsoft Corporation.  All rights reserved.


Dump of file moskrnl.exe

File Type: EXECUTABLE IMAGE

`dynamic initializer for 'IDT'':
  0000000000101000: 40 55              push        rbp
  0000000000101002: 48 83 EC 60        sub         rsp,60h
  0000000000101006: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  000000000010100B: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101011: 45 33 C0           xor         r8d,r8d
  0000000000101014: 48 8D 15 C5 42 00  lea         rdx,[DE_ISR_HANDLER]
                    00
  000000000010101B: 48 8D 0D DE 60 10  lea         rcx,[207100h]
                    00
  0000000000101022: E8 B9 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101027: 41 B9 0E 00 00 00  mov         r9d,0Eh
  000000000010102D: 45 33 C0           xor         r8d,r8d
  0000000000101030: 48 8D 15 B7 42 00  lea         rdx,[DB_ISR_HANDLER]
                    00
  0000000000101037: 48 8D 0D D2 60 10  lea         rcx,[207110h]
                    00
  000000000010103E: E8 9D 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101043: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101049: 66 41 B8 02 00     mov         r8w,2
  000000000010104E: 48 8D 15 A7 42 00  lea         rdx,[NMI_ISR_HANDLER]
                    00
  0000000000101055: 48 8D 0D C4 60 10  lea         rcx,[207120h]
                    00
  000000000010105C: E8 7F 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101061: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101067: 66 41 B8 03 00     mov         r8w,3
  000000000010106C: 48 8D 15 7B 42 00  lea         rdx,[DB_ISR_HANDLER]
                    00
  0000000000101073: 48 8D 0D B6 60 10  lea         rcx,[207130h]
                    00
  000000000010107A: E8 61 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010107F: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101085: 45 33 C0           xor         r8d,r8d
  0000000000101088: 48 8D 15 89 42 00  lea         rdx,[OF_ISR_HANDLER]
                    00
  000000000010108F: 48 8D 0D AA 60 10  lea         rcx,[207140h]
                    00
  0000000000101096: E8 45 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010109B: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001010A1: 45 33 C0           xor         r8d,r8d
  00000000001010A4: 48 8D 15 7B 42 00  lea         rdx,[BR_ISR_HANDLER]
                    00
  00000000001010AB: 48 8D 0D 9E 60 10  lea         rcx,[207150h]
                    00
  00000000001010B2: E8 29 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001010B7: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001010BD: 45 33 C0           xor         r8d,r8d
  00000000001010C0: 48 8D 15 6D 42 00  lea         rdx,[UD_ISR_HANDLER]
                    00
  00000000001010C7: 48 8D 0D 92 60 10  lea         rcx,[207160h]
                    00
  00000000001010CE: E8 0D 09 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001010D3: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001010D9: 45 33 C0           xor         r8d,r8d
  00000000001010DC: 48 8D 15 5F 42 00  lea         rdx,[NM_ISR_HANDLER]
                    00
  00000000001010E3: 48 8D 0D 86 60 10  lea         rcx,[207170h]
                    00
  00000000001010EA: E8 F1 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001010EF: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001010F5: 66 41 B8 01 00     mov         r8w,1
  00000000001010FA: 48 8D 15 4F 42 00  lea         rdx,[DF_EXC_HANDLER]
                    00
  0000000000101101: 48 8D 0D 78 60 10  lea         rcx,[207180h]
                    00
  0000000000101108: E8 D3 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010110D: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101113: 45 33 C0           xor         r8d,r8d
  0000000000101116: 48 8D 15 45 42 00  lea         rdx,[CSO_ISR_HANDLER]
                    00
  000000000010111D: 48 8D 0D 6C 60 10  lea         rcx,[207190h]
                    00
  0000000000101124: E8 B7 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101129: 41 B9 0E 00 00 00  mov         r9d,0Eh
  000000000010112F: 45 33 C0           xor         r8d,r8d
  0000000000101132: 48 8D 15 37 42 00  lea         rdx,[TS_EXC_HANDLER]
                    00
  0000000000101139: 48 8D 0D 60 60 10  lea         rcx,[2071A0h]
                    00
  0000000000101140: E8 9B 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101145: 41 B9 0E 00 00 00  mov         r9d,0Eh
  000000000010114B: 45 33 C0           xor         r8d,r8d
  000000000010114E: 48 8D 15 2D 42 00  lea         rdx,[NP_EXC_HANDLER]
                    00
  0000000000101155: 48 8D 0D 54 60 10  lea         rcx,[2071B0h]
                    00
  000000000010115C: E8 7F 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101161: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101167: 45 33 C0           xor         r8d,r8d
  000000000010116A: 48 8D 15 23 42 00  lea         rdx,[SS_EXC_HANDLER]
                    00
  0000000000101171: 48 8D 0D 48 60 10  lea         rcx,[2071C0h]
                    00
  0000000000101178: E8 63 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010117D: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101183: 45 33 C0           xor         r8d,r8d
  0000000000101186: 48 8D 15 19 42 00  lea         rdx,[GP_EXC_HANDLER]
                    00
  000000000010118D: 48 8D 0D 3C 60 10  lea         rcx,[2071D0h]
                    00
  0000000000101194: E8 47 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101199: 41 B9 0E 00 00 00  mov         r9d,0Eh
  000000000010119F: 45 33 C0           xor         r8d,r8d
  00000000001011A2: 48 8D 15 0F 42 00  lea         rdx,[PF_EXC_HANDLER]
                    00
  00000000001011A9: 48 8D 0D 30 60 10  lea         rcx,[2071E0h]
                    00
  00000000001011B0: E8 2B 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001011B5: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001011BB: 45 33 C0           xor         r8d,r8d
  00000000001011BE: 48 8D 15 05 42 00  lea         rdx,[MF_ISR_HANDLER]
                    00
  00000000001011C5: 48 8D 0D 24 60 10  lea         rcx,[2071F0h]
                    00
  00000000001011CC: E8 0F 08 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001011D1: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001011D7: 45 33 C0           xor         r8d,r8d
  00000000001011DA: 48 8D 15 F7 41 00  lea         rdx,[AC_EXC_HANDLER]
                    00
  00000000001011E1: 48 8D 0D 18 60 10  lea         rcx,[207200h]
                    00
  00000000001011E8: E8 F3 07 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  00000000001011ED: 41 B9 0E 00 00 00  mov         r9d,0Eh
  00000000001011F3: 66 41 B8 04 00     mov         r8w,4
  00000000001011F8: 48 8D 15 EB 41 00  lea         rdx,[MC_ISR_HANDLER]
                    00
  00000000001011FF: 48 8D 0D 0A 60 10  lea         rcx,[207210h]
                    00
  0000000000101206: E8 D5 07 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010120B: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101211: 45 33 C0           xor         r8d,r8d
  0000000000101214: 48 8D 15 DD 41 00  lea         rdx,[XM_ISR_HANDLER]
                    00
  000000000010121B: 48 8D 0D FE 5F 10  lea         rcx,[207220h]
                    00
  0000000000101222: E8 B9 07 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101227: 41 B9 0E 00 00 00  mov         r9d,0Eh
  000000000010122D: 45 33 C0           xor         r8d,r8d
  0000000000101230: 48 8D 15 CF 41 00  lea         rdx,[VE_ISR_HANDLER]
                    00
  0000000000101237: 48 8D 0D F2 5F 10  lea         rcx,[207230h]
                    00
  000000000010123E: E8 9D 07 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  0000000000101243: 41 B9 0E 00 00 00  mov         r9d,0Eh
  0000000000101249: 45 33 C0           xor         r8d,r8d
  000000000010124C: 48 8D 15 C1 41 00  lea         rdx,[SX_EXC_HANDLER]
                    00
  0000000000101253: 48 8D 0D E6 5F 10  lea         rcx,[207240h]
                    00
  000000000010125A: E8 81 07 00 00     call        ??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z
  000000000010125F: 4C 8D 0D 5A 07 00  lea         r9,[??0_IDT_GATE@@QEAA@XZ]
                    00
  0000000000101266: 41 B8 EB 00 00 00  mov         r8d,0EBh
  000000000010126C: BA 10 00 00 00     mov         edx,10h
  0000000000101271: 48 8D 0D D8 5F 10  lea         rcx,[207250h]
                    00
  0000000000101278: E8 A3 08 00 00     call        ??_H@YAXPEAX_K1P6APEAX0@Z@Z
  000000000010127D: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101281: 5D                 pop         rbp
  0000000000101282: C3                 ret
  0000000000101283: CC CC CC CC CC CC CC CC CC CC CC CC CC           ллллллллллллл
`dynamic initializer for 'KernelGDT'':
  0000000000101290: 40 55              push        rbp
  0000000000101292: 48 83 EC 50        sub         rsp,50h
  0000000000101296: 48 8B EC           mov         rbp,rsp
  0000000000101299: 48 8D 05 60 AE 10  lea         rax,[20C100h]
                    00
  00000000001012A0: 66 89 05 C3 5D 00  mov         word ptr [10706Ah],ax
                    00
  00000000001012A7: 48 C7 45 00 00 00  mov         qword ptr [rbp],0
                    00 00
  00000000001012AF: 48 8D 05 4A AE 10  lea         rax,[20C100h]
                    00
  00000000001012B6: 48 C1 E8 10        shr         rax,10h
  00000000001012BA: 0F B6 C0           movzx       eax,al
  00000000001012BD: 48 25 FF 00 00 00  and         rax,0FFh
  00000000001012C3: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001012C7: 48 0B C8           or          rcx,rax
  00000000001012CA: 48 8B C1           mov         rax,rcx
  00000000001012CD: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001012D1: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001012D5: 48 0D 00 89 00 00  or          rax,8900h
  00000000001012DB: 66 89 05 8A 5D 00  mov         word ptr [10706Ch],ax
                    00
  00000000001012E2: 48 C7 45 00 00 00  mov         qword ptr [rbp],0
                    00 00
  00000000001012EA: 48 8D 05 0F AE 10  lea         rax,[20C100h]
                    00
  00000000001012F1: 48 C1 E8 18        shr         rax,18h
  00000000001012F5: 0F B6 C0           movzx       eax,al
  00000000001012F8: 48 25 FF 00 00 00  and         rax,0FFh
  00000000001012FE: 48 C1 E0 08        shl         rax,8
  0000000000101302: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000101306: 48 0B C8           or          rcx,rax
  0000000000101309: 48 8B C1           mov         rax,rcx
  000000000010130C: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000101310: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000101314: 48 0F BA E8 07     bts         rax,7
  0000000000101319: 66 89 05 4E 5D 00  mov         word ptr [10706Eh],ax
                    00
  0000000000101320: 48 8D 05 D9 AD 10  lea         rax,[20C100h]
                    00
  0000000000101327: 48 C1 E8 20        shr         rax,20h
  000000000010132B: 89 05 3F 5D 00 00  mov         dword ptr [107070h],eax
  0000000000101331: C7 05 39 5D 00 00  mov         dword ptr [107074h],0
                    00 00 00 00
  000000000010133B: 48 8D 65 50        lea         rsp,[rbp+50h]
  000000000010133F: 5D                 pop         rbp
  0000000000101340: C3                 ret
  0000000000101341: CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC     ллллллллллллллл
`dynamic initializer for 'display'':
  0000000000101350: 40 55              push        rbp
  0000000000101352: 48 83 EC 60        sub         rsp,60h
  0000000000101356: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  000000000010135B: 48 8D 0D 5E 5D 10  lea         rcx,[?display@@3VDisplay@@A]
                    00
  0000000000101362: E8 29 06 00 00     call        ??0Display@@QEAA@XZ
  0000000000101367: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010136B: 5D                 pop         rbp
  000000000010136C: C3                 ret
  000000000010136D: CC CC CC                                         ллл
??0Display@@QEAA@PEAUGRAPHICS_DEVICE@@@Z:
  0000000000101370: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101375: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010137A: 55                 push        rbp
  000000000010137B: 48 83 EC 40        sub         rsp,40h
  000000000010137F: 48 8B EC           mov         rbp,rsp
  0000000000101382: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101386: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  000000000010138A: 48 89 08           mov         qword ptr [rax],rcx
  000000000010138D: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101391: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101395: 5D                 pop         rbp
  0000000000101396: C3                 ret
  0000000000101397: CC CC CC CC CC CC CC CC CC                       ллллллллл
?ColorPixel@Display@@QEAAXUBGRRPixel@@UPoint2D@@@Z:
  00000000001013A0: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  00000000001013A5: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  00000000001013A9: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001013AE: 55                 push        rbp
  00000000001013AF: 48 83 EC 70        sub         rsp,70h
  00000000001013B3: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  00000000001013B8: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001013BC: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001013BF: 8B 40 0C           mov         eax,dword ptr [rax+0Ch]
  00000000001013C2: 39 45 70           cmp         dword ptr [rbp+70h],eax
  00000000001013C5: 72 0C              jb          00000000001013D3
  00000000001013C7: 48 8D 0D 32 4C 00  lea         rcx,[??_C@_0CN@IGHBCMID@position?4X?5?$DM?5m_pDevice?9?$DOHorizon@]
                    00
  00000000001013CE: E8 3D 08 00 00     call        ?KernelBugcheck@@YAXPEBD@Z
  00000000001013D3: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001013D7: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001013DA: 8B 40 10           mov         eax,dword ptr [rax+10h]
  00000000001013DD: 39 45 74           cmp         dword ptr [rbp+74h],eax
  00000000001013E0: 72 0C              jb          00000000001013EE
  00000000001013E2: 48 8D 0D 47 4C 00  lea         rcx,[??_C@_0CL@CHJBJOFN@position?4Y?5?$DM?5m_pDevice?9?$DOVertica@]
                    00
  00000000001013E9: E8 22 08 00 00     call        ?KernelBugcheck@@YAXPEBD@Z
  00000000001013EE: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001013F2: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001013F5: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001013F8: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001013FC: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000101400: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000101403: 8B 4D 74           mov         ecx,dword ptr [rbp+74h]
  0000000000101406: 0F AF 48 14        imul        ecx,dword ptr [rax+14h]
  000000000010140A: 8B C1              mov         eax,ecx
  000000000010140C: 8B C0              mov         eax,eax
  000000000010140E: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000101412: 48 8D 04 81        lea         rax,[rcx+rax*4]
  0000000000101416: 8B 4D 70           mov         ecx,dword ptr [rbp+70h]
  0000000000101419: 48 8D 04 88        lea         rax,[rax+rcx*4]
  000000000010141D: 48 89 45 08        mov         qword ptr [rbp+8],rax
  0000000000101421: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000101425: 8B 4D 68           mov         ecx,dword ptr [rbp+68h]
  0000000000101428: 89 08              mov         dword ptr [rax],ecx
  000000000010142A: 48 8D 65 50        lea         rsp,[rbp+50h]
  000000000010142E: 5D                 pop         rbp
  000000000010142F: C3                 ret
?ColorRectangle@Display@@QEAAXUBGRRPixel@@PEAURectangle@@@Z:
  0000000000101430: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000101435: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  0000000000101439: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010143E: 55                 push        rbp
  000000000010143F: 48 83 EC 60        sub         rsp,60h
  0000000000101443: 48 8B EC           mov         rbp,rsp
  0000000000101446: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010144A: 48 8B 00           mov         rax,qword ptr [rax]
  000000000010144D: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000101450: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000101454: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  000000000010145B: 8B 00              mov         eax,dword ptr [rax]
  000000000010145D: 89 45 08           mov         dword ptr [rbp+8],eax
  0000000000101460: EB 08              jmp         000000000010146A
  0000000000101462: 8B 45 08           mov         eax,dword ptr [rbp+8]
  0000000000101465: FF C0              inc         eax
  0000000000101467: 89 45 08           mov         dword ptr [rbp+8],eax
  000000000010146A: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  0000000000101471: 8B 40 08           mov         eax,dword ptr [rax+8]
  0000000000101474: 39 45 08           cmp         dword ptr [rbp+8],eax
  0000000000101477: 73 58              jae         00000000001014D1
  0000000000101479: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  0000000000101480: 8B 40 04           mov         eax,dword ptr [rax+4]
  0000000000101483: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  0000000000101486: EB 08              jmp         0000000000101490
  0000000000101488: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  000000000010148B: FF C0              inc         eax
  000000000010148D: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  0000000000101490: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  0000000000101497: 8B 40 0C           mov         eax,dword ptr [rax+0Ch]
  000000000010149A: 39 45 0C           cmp         dword ptr [rbp+0Ch],eax
  000000000010149D: 73 30              jae         00000000001014CF
  000000000010149F: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  00000000001014A3: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001014A6: 8B 4D 0C           mov         ecx,dword ptr [rbp+0Ch]
  00000000001014A9: 0F AF 48 14        imul        ecx,dword ptr [rax+14h]
  00000000001014AD: 8B C1              mov         eax,ecx
  00000000001014AF: 8B C0              mov         eax,eax
  00000000001014B1: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001014B5: 48 8D 04 81        lea         rax,[rcx+rax*4]
  00000000001014B9: 8B 4D 08           mov         ecx,dword ptr [rbp+8]
  00000000001014BC: 48 8D 04 88        lea         rax,[rax+rcx*4]
  00000000001014C0: 48 89 45 10        mov         qword ptr [rbp+10h],rax
  00000000001014C4: 48 8B 45 10        mov         rax,qword ptr [rbp+10h]
  00000000001014C8: 8B 4D 78           mov         ecx,dword ptr [rbp+78h]
  00000000001014CB: 89 08              mov         dword ptr [rax],ecx
  00000000001014CD: EB B9              jmp         0000000000101488
  00000000001014CF: EB 91              jmp         0000000000101462
  00000000001014D1: 48 8D 65 60        lea         rsp,[rbp+60h]
  00000000001014D5: 5D                 pop         rbp
  00000000001014D6: C3                 ret
  00000000001014D7: CC CC CC CC CC CC CC CC CC                       ллллллллл
?ColorScreen@Display@@QEAAXUBGRRPixel@@@Z:
  00000000001014E0: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  00000000001014E4: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001014E9: 55                 push        rbp
  00000000001014EA: 48 81 EC 80 00 00  sub         rsp,80h
                    00
  00000000001014F1: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  00000000001014F6: C7 45 50 00 00 00  mov         dword ptr [rbp+50h],0
                    00
  00000000001014FD: C7 45 54 00 00 00  mov         dword ptr [rbp+54h],0
                    00
  0000000000101504: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101508: 48 89 45 00        mov         qword ptr [rbp],rax
  000000000010150C: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000101510: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000101513: 8B 40 0C           mov         eax,dword ptr [rax+0Ch]
  0000000000101516: 89 45 58           mov         dword ptr [rbp+58h],eax
  0000000000101519: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010151D: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000101520: 8B 40 10           mov         eax,dword ptr [rax+10h]
  0000000000101523: 89 45 5C           mov         dword ptr [rbp+5Ch],eax
  0000000000101526: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  000000000010152A: 48 89 45 08        mov         qword ptr [rbp+8],rax
  000000000010152E: 4C 8D 45 00        lea         r8,[rbp]
  0000000000101532: 8B 55 78           mov         edx,dword ptr [rbp+78h]
  0000000000101535: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000101539: E8 F2 FE FF FF     call        ?ColorRectangle@Display@@QEAAXUBGRRPixel@@PEAURectangle@@@Z
  000000000010153E: 48 8D 65 60        lea         rsp,[rbp+60h]
  0000000000101542: 5D                 pop         rbp
  0000000000101543: C3                 ret
  0000000000101544: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
?GetCharacterMap@Font@@QEAAPEBDD@Z:
  0000000000101550: 88 54 24 10        mov         byte ptr [rsp+10h],dl
  0000000000101554: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101559: 55                 push        rbp
  000000000010155A: 48 83 EC 60        sub         rsp,60h
  000000000010155E: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101563: 0F BE 45 58        movsx       eax,byte ptr [rbp+58h]
  0000000000101567: 83 F8 7F           cmp         eax,7Fh
  000000000010156A: 7E 0C              jle         0000000000101578
  000000000010156C: 48 8D 0D ED 4A 00  lea         rcx,[??_C@_09DAIDAGKM@c?5?$DM?$DN?50x7F@]
                    00
  0000000000101573: E8 98 06 00 00     call        ?KernelBugcheck@@YAXPEBD@Z
  0000000000101578: 48 0F BE 45 58     movsx       rax,byte ptr [rbp+58h]
  000000000010157D: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101581: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000101584: 48 8D 04 C1        lea         rax,[rcx+rax*8]
  0000000000101588: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010158C: 5D                 pop         rbp
  000000000010158D: C3                 ret
  000000000010158E: CC CC                                            лл
?GetImageSize@Loader@@SAIPEAX@Z:
  0000000000101590: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101595: 55                 push        rbp
  0000000000101596: 48 83 EC 70        sub         rsp,70h
  000000000010159A: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  000000000010159F: 48 83 7D 60 00     cmp         qword ptr [rbp+60h],0
  00000000001015A4: 75 0C              jne         00000000001015B2
  00000000001015A6: 48 8D 0D C3 4A 00  lea         rcx,[??_C@_0BH@JGLNJLPD@baseAddress?5?$CB?$DN?5nullptr@]
                    00
  00000000001015AD: E8 5E 06 00 00     call        ?KernelBugcheck@@YAXPEBD@Z
  00000000001015B2: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001015B6: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001015BA: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001015BE: 8B 40 3C           mov         eax,dword ptr [rax+3Ch]
  00000000001015C1: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001015C5: 48 03 C8           add         rcx,rax
  00000000001015C8: 48 8B C1           mov         rax,rcx
  00000000001015CB: 48 89 45 08        mov         qword ptr [rbp+8],rax
  00000000001015CF: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  00000000001015D3: 8B 40 50           mov         eax,dword ptr [rax+50h]
  00000000001015D6: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001015DA: 5D                 pop         rbp
  00000000001015DB: C3                 ret
  00000000001015DC: CC CC CC CC                                      лллл
??0Font@@QEAA@XZ:
  00000000001015E0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001015E5: 55                 push        rbp
  00000000001015E6: 48 83 EC 40        sub         rsp,40h
  00000000001015EA: 48 8B EC           mov         rbp,rsp
  00000000001015ED: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001015F1: 48 8D 0D 98 4A 00  lea         rcx,[106090h]
                    00
  00000000001015F8: 48 89 08           mov         qword ptr [rax],rcx
  00000000001015FB: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001015FF: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101603: 5D                 pop         rbp
  0000000000101604: C3                 ret
  0000000000101605: CC CC CC CC CC CC CC CC CC CC CC                 ллллллллллл
??0LoadingScreen@@QEAA@AEAVDisplay@@@Z:
  0000000000101610: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101615: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010161A: 55                 push        rbp
  000000000010161B: 57                 push        rdi
  000000000010161C: 48 83 EC 78        sub         rsp,78h
  0000000000101620: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101625: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000101629: C7 00 01 00 00 00  mov         dword ptr [rax],1
  000000000010162F: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000101633: 48 8B 4D 78        mov         rcx,qword ptr [rbp+78h]
  0000000000101637: 48 89 48 10        mov         qword ptr [rax+10h],rcx
  000000000010163B: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010163F: 48 83 C0 18        add         rax,18h
  0000000000101643: 48 8B C8           mov         rcx,rax
  0000000000101646: E8 95 FF FF FF     call        ??0Font@@QEAA@XZ
  000000000010164B: 48 8D 45 40        lea         rax,[rbp+40h]
  000000000010164F: 48 8B F8           mov         rdi,rax
  0000000000101652: 33 C0              xor         eax,eax
  0000000000101654: B9 08 00 00 00     mov         ecx,8
  0000000000101659: F3 AA              rep stos    byte ptr [rdi]
  000000000010165B: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010165F: 48 8B 4D 40        mov         rcx,qword ptr [rbp+40h]
  0000000000101663: 48 89 48 04        mov         qword ptr [rax+4],rcx
  0000000000101667: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010166B: 48 8D 65 58        lea         rsp,[rbp+58h]
  000000000010166F: 5F                 pop         rdi
  0000000000101670: 5D                 pop         rbp
  0000000000101671: C3                 ret
  0000000000101672: CC CC CC CC CC CC CC CC CC CC CC CC CC CC        лллллллллллллл
?AdvanceX@LoadingScreen@@QEAAXXZ:
  0000000000101680: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101685: 55                 push        rbp
  0000000000101686: 48 83 EC 50        sub         rsp,50h
  000000000010168A: 48 8B EC           mov         rbp,rsp
  000000000010168D: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000101691: 8B 40 04           mov         eax,dword ptr [rax+4]
  0000000000101694: 83 C0 0A           add         eax,0Ah
  0000000000101697: 89 45 40           mov         dword ptr [rbp+40h],eax
  000000000010169A: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  000000000010169E: 8B 4D 40           mov         ecx,dword ptr [rbp+40h]
  00000000001016A1: 89 48 04           mov         dword ptr [rax+4],ecx
  00000000001016A4: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001016A8: 8B 4D 40           mov         ecx,dword ptr [rbp+40h]
  00000000001016AB: 89 48 04           mov         dword ptr [rax+4],ecx
  00000000001016AE: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001016B2: 5D                 pop         rbp
  00000000001016B3: C3                 ret
  00000000001016B4: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
?AdvanceY@LoadingScreen@@QEAAXXZ:
  00000000001016C0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001016C5: 55                 push        rbp
  00000000001016C6: 48 83 EC 50        sub         rsp,50h
  00000000001016CA: 48 8B EC           mov         rbp,rsp
  00000000001016CD: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001016D1: 8B 40 08           mov         eax,dword ptr [rax+8]
  00000000001016D4: 83 C0 0A           add         eax,0Ah
  00000000001016D7: 89 45 40           mov         dword ptr [rbp+40h],eax
  00000000001016DA: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001016DE: 8B 4D 40           mov         ecx,dword ptr [rbp+40h]
  00000000001016E1: 89 48 08           mov         dword ptr [rax+8],ecx
  00000000001016E4: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001016E8: 8B 4D 40           mov         ecx,dword ptr [rbp+40h]
  00000000001016EB: 89 48 08           mov         dword ptr [rax+8],ecx
  00000000001016EE: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001016F2: 5D                 pop         rbp
  00000000001016F3: C3                 ret
  00000000001016F4: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
?ResetX@LoadingScreen@@QEAAXXZ:
  0000000000101700: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101705: 55                 push        rbp
  0000000000101706: 48 83 EC 40        sub         rsp,40h
  000000000010170A: 48 8B EC           mov         rbp,rsp
  000000000010170D: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101711: C7 40 04 00 00 00  mov         dword ptr [rax+4],0
                    00
  0000000000101718: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010171C: 5D                 pop         rbp
  000000000010171D: C3                 ret
  000000000010171E: CC CC                                            лл
?ResetY@LoadingScreen@@QEAAXXZ:
  0000000000101720: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101725: 55                 push        rbp
  0000000000101726: 48 83 EC 40        sub         rsp,40h
  000000000010172A: 48 8B EC           mov         rbp,rsp
  000000000010172D: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101731: C7 40 08 00 00 00  mov         dword ptr [rax+8],0
                    00
  0000000000101738: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010173C: 5D                 pop         rbp
  000000000010173D: C3                 ret
  000000000010173E: CC CC                                            лл
?WriteCharacter@LoadingScreen@@QEAAXD@Z:
  0000000000101740: 88 54 24 10        mov         byte ptr [rsp+10h],dl
  0000000000101744: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101749: 55                 push        rbp
  000000000010174A: 48 81 EC A0 00 00  sub         rsp,0A0h
                    00
  0000000000101751: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101756: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010175D: 48 83 C0 18        add         rax,18h
  0000000000101761: 0F B6 95 98 00 00  movzx       edx,byte ptr [rbp+98h]
                    00
  0000000000101768: 48 8B C8           mov         rcx,rax
  000000000010176B: E8 E0 FD FF FF     call        ?GetCharacterMap@Font@@QEAAPEBDD@Z
  0000000000101770: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000101774: C7 45 08 08 00 00  mov         dword ptr [rbp+8],8
                    00
  000000000010177B: C7 45 0C 00 00 00  mov         dword ptr [rbp+0Ch],0
                    00
  0000000000101782: EB 08              jmp         000000000010178C
  0000000000101784: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  0000000000101787: FF C0              inc         eax
  0000000000101789: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  000000000010178C: 8B 45 08           mov         eax,dword ptr [rbp+8]
  000000000010178F: 39 45 0C           cmp         dword ptr [rbp+0Ch],eax
  0000000000101792: 0F 83 D7 00 00 00  jae         000000000010186F
  0000000000101798: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  000000000010179B: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  000000000010179F: 0F B6 04 01        movzx       eax,byte ptr [rcx+rax]
  00000000001017A3: 88 45 10           mov         byte ptr [rbp+10h],al
  00000000001017A6: C7 45 14 00 00 00  mov         dword ptr [rbp+14h],0
                    00
  00000000001017AD: C6 45 18 80        mov         byte ptr [rbp+18h],80h
  00000000001017B1: 0F B6 45 18        movzx       eax,byte ptr [rbp+18h]
  00000000001017B5: 85 C0              test        eax,eax
  00000000001017B7: 0F 8E AD 00 00 00  jle         000000000010186A
  00000000001017BD: 0F BE 45 10        movsx       eax,byte ptr [rbp+10h]
  00000000001017C1: 0F B6 4D 18        movzx       ecx,byte ptr [rbp+18h]
  00000000001017C5: 23 C1              and         eax,ecx
  00000000001017C7: 85 C0              test        eax,eax
  00000000001017C9: 0F 84 85 00 00 00  je          0000000000101854
  00000000001017CF: B8 08 00 00 00     mov         eax,8
  00000000001017D4: 2B 45 14           sub         eax,dword ptr [rbp+14h]
  00000000001017D7: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  00000000001017DE: 8B 49 04           mov         ecx,dword ptr [rcx+4]
  00000000001017E1: 8D 44 01 FF        lea         eax,[rcx+rax-1]
  00000000001017E5: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  00000000001017EC: 0F AF 01           imul        eax,dword ptr [rcx]
  00000000001017EF: 89 45 20           mov         dword ptr [rbp+20h],eax
  00000000001017F2: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  00000000001017F9: 8B 40 08           mov         eax,dword ptr [rax+8]
  00000000001017FC: 03 45 0C           add         eax,dword ptr [rbp+0Ch]
  00000000001017FF: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  0000000000101806: 0F AF 01           imul        eax,dword ptr [rcx]
  0000000000101809: 89 45 24           mov         dword ptr [rbp+24h],eax
  000000000010180C: 48 8B 45 20        mov         rax,qword ptr [rbp+20h]
  0000000000101810: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000101814: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010181B: 8B 00              mov         eax,dword ptr [rax]
  000000000010181D: 8B 4D 20           mov         ecx,dword ptr [rbp+20h]
  0000000000101820: 03 C8              add         ecx,eax
  0000000000101822: 8B C1              mov         eax,ecx
  0000000000101824: 89 45 30           mov         dword ptr [rbp+30h],eax
  0000000000101827: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010182E: 8B 00              mov         eax,dword ptr [rax]
  0000000000101830: 8B 4D 24           mov         ecx,dword ptr [rbp+24h]
  0000000000101833: 03 C8              add         ecx,eax
  0000000000101835: 8B C1              mov         eax,ecx
  0000000000101837: 89 45 34           mov         dword ptr [rbp+34h],eax
  000000000010183A: 4C 8D 45 28        lea         r8,[rbp+28h]
  000000000010183E: 8B 15 4C 4C 00 00  mov         edx,dword ptr [?m_white@LoadingScreen@@0UBGRRPixel@@B]
  0000000000101844: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010184B: 48 8B 48 10        mov         rcx,qword ptr [rax+10h]
  000000000010184F: E8 DC FB FF FF     call        ?ColorRectangle@Display@@QEAAXUBGRRPixel@@PEAURectangle@@@Z
  0000000000101854: 0F B6 45 18        movzx       eax,byte ptr [rbp+18h]
  0000000000101858: D1 F8              sar         eax,1
  000000000010185A: 88 45 18           mov         byte ptr [rbp+18h],al
  000000000010185D: 8B 45 14           mov         eax,dword ptr [rbp+14h]
  0000000000101860: FF C0              inc         eax
  0000000000101862: 89 45 14           mov         dword ptr [rbp+14h],eax
  0000000000101865: E9 47 FF FF FF     jmp         00000000001017B1
  000000000010186A: E9 15 FF FF FF     jmp         0000000000101784
  000000000010186F: 48 8D A5 80 00 00  lea         rsp,[rbp+80h]
                    00
  0000000000101876: 5D                 pop         rbp
  0000000000101877: C3                 ret
  0000000000101878: CC CC CC CC CC CC CC CC                          лллллллл
?WriteLine@LoadingScreen@@QEAAXPEBD@Z:
  0000000000101880: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101885: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010188A: 55                 push        rbp
  000000000010188B: 48 83 EC 60        sub         rsp,60h
  000000000010188F: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101894: 48 8B 55 58        mov         rdx,qword ptr [rbp+58h]
  0000000000101898: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  000000000010189C: E8 8F 00 00 00     call        ?WriteText@LoadingScreen@@QEAAXPEBD@Z
  00000000001018A1: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  00000000001018A5: E8 16 FE FF FF     call        ?AdvanceY@LoadingScreen@@QEAAXXZ
  00000000001018AA: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  00000000001018AE: E8 4D FE FF FF     call        ?ResetX@LoadingScreen@@QEAAXXZ
  00000000001018B3: 48 8D 65 40        lea         rsp,[rbp+40h]
  00000000001018B7: 5D                 pop         rbp
  00000000001018B8: C3                 ret
  00000000001018B9: CC CC CC CC CC CC CC                             ллллллл
?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ:
  00000000001018C0: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001018C5: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001018CA: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  00000000001018CF: 4C 89 4C 24 20     mov         qword ptr [rsp+20h],r9
  00000000001018D4: 55                 push        rbp
  00000000001018D5: 48 81 EC C0 00 00  sub         rsp,0C0h
                    00
  00000000001018DC: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  00000000001018E1: 48 8D 85 C0 00 00  lea         rax,[rbp+0C0h]
                    00
  00000000001018E8: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001018EC: 4C 8B 45 50        mov         r8,qword ptr [rbp+50h]
  00000000001018F0: 48 8B 95 B8 00 00  mov         rdx,qword ptr [rbp+0B8h]
                    00
  00000000001018F7: 48 8D 4D 00        lea         rcx,[rbp]
  00000000001018FB: E8 D0 31 00 00     call        ?vsprintf@String@@SAHPEADPEBD0@Z
  0000000000101900: 89 45 58           mov         dword ptr [rbp+58h],eax
  0000000000101903: 48 63 45 58        movsxd      rax,dword ptr [rbp+58h]
  0000000000101907: C6 44 05 00 00     mov         byte ptr [rbp+rax],0
  000000000010190C: 48 C7 45 50 00 00  mov         qword ptr [rbp+50h],0
                    00 00
  0000000000101914: 48 8D 55 00        lea         rdx,[rbp]
  0000000000101918: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  000000000010191F: E8 5C FF FF FF     call        ?WriteLine@LoadingScreen@@QEAAXPEBD@Z
  0000000000101924: 48 8D A5 A0 00 00  lea         rsp,[rbp+0A0h]
                    00
  000000000010192B: 5D                 pop         rbp
  000000000010192C: C3                 ret
  000000000010192D: CC CC CC                                         ллл
?WriteText@LoadingScreen@@QEAAXPEBD@Z:
  0000000000101930: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101935: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010193A: 55                 push        rbp
  000000000010193B: 48 83 EC 60        sub         rsp,60h
  000000000010193F: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101944: 48 83 7D 58 00     cmp         qword ptr [rbp+58h],0
  0000000000101949: 75 0C              jne         0000000000101957
  000000000010194B: 48 8D 0D 46 4B 00  lea         rcx,[??_C@_0BA@PHAFGIPF@text?5?$CB?$DN?5nullptr@]
                    00
  0000000000101952: E8 B9 02 00 00     call        ?KernelBugcheck@@YAXPEBD@Z
  0000000000101957: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  000000000010195B: 0F BE 00           movsx       eax,byte ptr [rax]
  000000000010195E: 85 C0              test        eax,eax
  0000000000101960: 74 26              je          0000000000101988
  0000000000101962: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  0000000000101966: 0F B6 10           movzx       edx,byte ptr [rax]
  0000000000101969: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  000000000010196D: E8 CE FD FF FF     call        ?WriteCharacter@LoadingScreen@@QEAAXD@Z
  0000000000101972: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101976: E8 05 FD FF FF     call        ?AdvanceX@LoadingScreen@@QEAAXXZ
  000000000010197B: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  000000000010197F: 48 FF C0           inc         rax
  0000000000101982: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  0000000000101986: EB CF              jmp         0000000000101957
  0000000000101988: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010198C: 5D                 pop         rbp
  000000000010198D: C3                 ret
  000000000010198E: CC CC                                            лл
??0Display@@QEAA@XZ:
  0000000000101990: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101995: 55                 push        rbp
  0000000000101996: 48 83 EC 40        sub         rsp,40h
  000000000010199A: 48 8B EC           mov         rbp,rsp
  000000000010199D: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001019A1: 48 C7 00 00 00 00  mov         qword ptr [rax],0
                    00
  00000000001019A8: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001019AC: 48 8D 65 40        lea         rsp,[rbp+40h]
  00000000001019B0: 5D                 pop         rbp
  00000000001019B1: C3                 ret
  00000000001019B2: CC CC CC CC CC CC CC CC CC CC CC CC CC CC        лллллллллллллл
??0_IDT_GATE@@QEAA@XZ:
  00000000001019C0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001019C5: 55                 push        rbp
  00000000001019C6: 48 83 EC 40        sub         rsp,40h
  00000000001019CA: 48 8B EC           mov         rbp,rsp
  00000000001019CD: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001019D1: 48 8D 65 40        lea         rsp,[rbp+40h]
  00000000001019D5: 5D                 pop         rbp
  00000000001019D6: C3                 ret
  00000000001019D7: CC CC CC CC CC CC CC CC CC                       ллллллллл
??0_IDT_GATE@@QEAA@_KGW4IDT_GATE_TYPE@@@Z:
  00000000001019E0: 44 89 4C 24 20     mov         dword ptr [rsp+20h],r9d
  00000000001019E5: 66 44 89 44 24 18  mov         word ptr [rsp+18h],r8w
  00000000001019EB: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001019F0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001019F5: 55                 push        rbp
  00000000001019F6: 48 83 EC 40        sub         rsp,40h
  00000000001019FA: 48 8B EC           mov         rbp,rsp
  00000000001019FD: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101A01: 0F B7 4D 58        movzx       ecx,word ptr [rbp+58h]
  0000000000101A05: 66 89 08           mov         word ptr [rax],cx
  0000000000101A08: 33 C0              xor         eax,eax
  0000000000101A0A: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A0E: 66 89 41 02        mov         word ptr [rcx+2],ax
  0000000000101A12: B8 FC FF 00 00     mov         eax,0FFFCh
  0000000000101A17: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A1B: 0F B7 49 02        movzx       ecx,word ptr [rcx+2]
  0000000000101A1F: 66 23 C8           and         cx,ax
  0000000000101A22: 0F B7 C1           movzx       eax,cx
  0000000000101A25: 66 83 C8 01        or          ax,1
  0000000000101A29: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A2D: 66 89 41 02        mov         word ptr [rcx+2],ax
  0000000000101A31: 0F B7 45 60        movzx       eax,word ptr [rbp+60h]
  0000000000101A35: 66 83 E0 07        and         ax,7
  0000000000101A39: B9 F8 FF 00 00     mov         ecx,0FFF8h
  0000000000101A3E: 48 8B 55 50        mov         rdx,qword ptr [rbp+50h]
  0000000000101A42: 0F B7 52 04        movzx       edx,word ptr [rdx+4]
  0000000000101A46: 66 23 D1           and         dx,cx
  0000000000101A49: 0F B7 CA           movzx       ecx,dx
  0000000000101A4C: 66 0B C8           or          cx,ax
  0000000000101A4F: 0F B7 C1           movzx       eax,cx
  0000000000101A52: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A56: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101A5A: B8 07 FF 00 00     mov         eax,0FF07h
  0000000000101A5F: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A63: 0F B7 49 04        movzx       ecx,word ptr [rcx+4]
  0000000000101A67: 66 23 C8           and         cx,ax
  0000000000101A6A: 0F B7 C1           movzx       eax,cx
  0000000000101A6D: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A71: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101A75: 0F B7 45 68        movzx       eax,word ptr [rbp+68h]
  0000000000101A79: 66 83 E0 0F        and         ax,0Fh
  0000000000101A7D: 66 C1 E0 08        shl         ax,8
  0000000000101A81: B9 FF F0 00 00     mov         ecx,0F0FFh
  0000000000101A86: 48 8B 55 50        mov         rdx,qword ptr [rbp+50h]
  0000000000101A8A: 0F B7 52 04        movzx       edx,word ptr [rdx+4]
  0000000000101A8E: 66 23 D1           and         dx,cx
  0000000000101A91: 0F B7 CA           movzx       ecx,dx
  0000000000101A94: 66 0B C8           or          cx,ax
  0000000000101A97: 0F B7 C1           movzx       eax,cx
  0000000000101A9A: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101A9E: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101AA2: B8 FF EF 00 00     mov         eax,0EFFFh
  0000000000101AA7: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AAB: 0F B7 49 04        movzx       ecx,word ptr [rcx+4]
  0000000000101AAF: 66 23 C8           and         cx,ax
  0000000000101AB2: 0F B7 C1           movzx       eax,cx
  0000000000101AB5: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AB9: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101ABD: B8 FF 9F 00 00     mov         eax,9FFFh
  0000000000101AC2: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AC6: 0F B7 49 04        movzx       ecx,word ptr [rcx+4]
  0000000000101ACA: 66 23 C8           and         cx,ax
  0000000000101ACD: 0F B7 C1           movzx       eax,cx
  0000000000101AD0: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AD4: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101AD8: B8 00 80 00 00     mov         eax,8000h
  0000000000101ADD: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AE1: 0F B7 49 04        movzx       ecx,word ptr [rcx+4]
  0000000000101AE5: 66 0B C8           or          cx,ax
  0000000000101AE8: 0F B7 C1           movzx       eax,cx
  0000000000101AEB: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AEF: 66 89 41 04        mov         word ptr [rcx+4],ax
  0000000000101AF3: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  0000000000101AF7: 48 C1 E8 10        shr         rax,10h
  0000000000101AFB: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101AFF: 66 89 41 06        mov         word ptr [rcx+6],ax
  0000000000101B03: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  0000000000101B07: 48 C1 E8 20        shr         rax,20h
  0000000000101B0B: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000101B0F: 89 41 08           mov         dword ptr [rcx+8],eax
  0000000000101B12: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101B16: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101B1A: 5D                 pop         rbp
  0000000000101B1B: C3                 ret
  0000000000101B1C: CC CC CC CC                                      лллл
??_H@YAXPEAX_K1P6APEAX0@Z@Z:
  0000000000101B20: 4C 89 4C 24 20     mov         qword ptr [rsp+20h],r9
  0000000000101B25: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000101B2A: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101B2F: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101B34: 55                 push        rbp
  0000000000101B35: 48 83 EC 70        sub         rsp,70h
  0000000000101B39: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101B3E: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000101B42: 48 89 45 40        mov         qword ptr [rbp+40h],rax
  0000000000101B46: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000101B4A: 48 FF C8           dec         rax
  0000000000101B4D: 48 89 45 70        mov         qword ptr [rbp+70h],rax
  0000000000101B51: 48 83 7D 40 00     cmp         qword ptr [rbp+40h],0
  0000000000101B56: 76 1B              jbe         0000000000101B73
  0000000000101B58: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000101B5C: FF 55 78           call        qword ptr [rbp+78h]
  0000000000101B5F: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000101B63: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000101B67: 48 03 C8           add         rcx,rax
  0000000000101B6A: 48 8B C1           mov         rax,rcx
  0000000000101B6D: 48 89 45 60        mov         qword ptr [rbp+60h],rax
  0000000000101B71: EB CB              jmp         0000000000101B3E
  0000000000101B73: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000101B77: 5D                 pop         rbp
  0000000000101B78: C3                 ret
  0000000000101B79: CC CC CC CC CC CC CC                             ллллллл
?InitializeGlobals@@YAXXZ:
  0000000000101B80: 40 55              push        rbp
  0000000000101B82: 48 83 EC 40        sub         rsp,40h
  0000000000101B86: 48 8B EC           mov         rbp,rsp
  0000000000101B89: 48 8D 05 70 65 10  lea         rax,[208100h]
                    00
  0000000000101B90: 89 05 8E A5 10 00  mov         dword ptr [20C124h],eax
  0000000000101B96: 48 8D 05 63 65 10  lea         rax,[208100h]
                    00
  0000000000101B9D: 48 C1 E8 20        shr         rax,20h
  0000000000101BA1: 89 05 81 A5 10 00  mov         dword ptr [20C128h],eax
  0000000000101BA7: 48 8D 05 52 75 10  lea         rax,[209100h]
                    00
  0000000000101BAE: 89 05 78 A5 10 00  mov         dword ptr [20C12Ch],eax
  0000000000101BB4: 48 8D 05 45 75 10  lea         rax,[209100h]
                    00
  0000000000101BBB: 48 C1 E8 20        shr         rax,20h
  0000000000101BBF: 89 05 6B A5 10 00  mov         dword ptr [20C130h],eax
  0000000000101BC5: 48 8D 05 34 85 10  lea         rax,[20A100h]
                    00
  0000000000101BCC: 89 05 62 A5 10 00  mov         dword ptr [20C134h],eax
  0000000000101BD2: 48 8D 05 27 85 10  lea         rax,[20A100h]
                    00
  0000000000101BD9: 48 C1 E8 20        shr         rax,20h
  0000000000101BDD: 89 05 55 A5 10 00  mov         dword ptr [20C138h],eax
  0000000000101BE3: 48 8D 05 16 95 10  lea         rax,[20B100h]
                    00
  0000000000101BEA: 89 05 4C A5 10 00  mov         dword ptr [20C13Ch],eax
  0000000000101BF0: 48 8D 05 09 95 10  lea         rax,[20B100h]
                    00
  0000000000101BF7: 48 C1 E8 20        shr         rax,20h
  0000000000101BFB: 89 05 3F A5 10 00  mov         dword ptr [20C140h],eax
  0000000000101C01: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101C05: 5D                 pop         rbp
  0000000000101C06: C3                 ret
  0000000000101C07: CC CC CC CC CC CC CC CC CC                       ллллллллл
?KernelBugcheck@@YAXPEBD@Z:
  0000000000101C10: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101C15: 55                 push        rbp
  0000000000101C16: 48 83 EC 60        sub         rsp,60h
  0000000000101C1A: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101C1F: 8B 15 83 48 00 00  mov         edx,dword ptr [1064A8h]
  0000000000101C25: 48 8D 0D 94 54 10  lea         rcx,[?display@@3VDisplay@@A]
                    00
  0000000000101C2C: E8 AF F8 FF FF     call        ?ColorScreen@Display@@QEAAXUBGRRPixel@@@Z
  0000000000101C31: 48 8B 0D 48 54 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101C38: E8 C3 FA FF FF     call        ?ResetX@LoadingScreen@@QEAAXXZ
  0000000000101C3D: 48 8B 0D 3C 54 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101C44: E8 D7 FA FF FF     call        ?ResetY@LoadingScreen@@QEAAXXZ
  0000000000101C49: 48 8B 55 50        mov         rdx,qword ptr [rbp+50h]
  0000000000101C4D: 48 8B 0D 2C 54 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101C54: E8 D7 FC FF FF     call        ?WriteText@LoadingScreen@@QEAAXPEBD@Z
  0000000000101C59: F4                 hlt
  0000000000101C5A: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101C5E: 5D                 pop         rbp
  0000000000101C5F: C3                 ret
?SetDisplay@Display@@QEAAXPEAUGRAPHICS_DEVICE@@@Z:
  0000000000101C60: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000101C65: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101C6A: 55                 push        rbp
  0000000000101C6B: 48 83 EC 40        sub         rsp,40h
  0000000000101C6F: 48 8B EC           mov         rbp,rsp
  0000000000101C72: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101C76: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  0000000000101C7A: 48 89 08           mov         qword ptr [rax],rcx
  0000000000101C7D: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101C81: 5D                 pop         rbp
  0000000000101C82: C3                 ret
  0000000000101C83: CC CC CC CC CC CC CC CC CC CC CC CC CC           ллллллллллллл
?test@@YAXXZ:
  0000000000101C90: 40 55              push        rbp
  0000000000101C92: 48 83 EC 60        sub         rsp,60h
  0000000000101C96: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101C9B: 33 C9              xor         ecx,ecx
  0000000000101C9D: E8 8E 00 00 00     call        main
  0000000000101CA2: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101CA6: 5D                 pop         rbp
  0000000000101CA7: C3                 ret
  0000000000101CA8: CC CC CC CC CC CC CC CC                          лллллллл
EXCEPTION_HANDLER:
  0000000000101CB0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101CB5: 55                 push        rbp
  0000000000101CB6: 48 83 EC 60        sub         rsp,60h
  0000000000101CBA: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101CBF: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101CC3: 4C 8B 80 88 00 00  mov         r8,qword ptr [rax+88h]
                    00
  0000000000101CCA: 48 8D 15 EF 47 00  lea         rdx,[??_C@_0BA@BOEBCBBF@EXC?3?5RIP?50x?$CF16x@]
                    00
  0000000000101CD1: 48 8B 0D A8 53 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101CD8: E8 E3 FB FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101CDD: F4                 hlt
  0000000000101CDE: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101CE2: 5D                 pop         rbp
  0000000000101CE3: C3                 ret
  0000000000101CE4: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
INTERRUPT_HANDLER:
  0000000000101CF0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101CF5: 55                 push        rbp
  0000000000101CF6: 48 83 EC 60        sub         rsp,60h
  0000000000101CFA: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000101CFF: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000101D03: 4C 8B 80 80 00 00  mov         r8,qword ptr [rax+80h]
                    00
  0000000000101D0A: 48 8D 15 9F 47 00  lea         rdx,[??_C@_0BA@JMCKIJDJ@ISR?3?5RIP?50x?$CF16x@]
                    00
  0000000000101D11: 48 8B 0D 68 53 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101D18: E8 A3 FB FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101D1D: F4                 hlt
  0000000000101D1E: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000101D22: 5D                 pop         rbp
  0000000000101D23: C3                 ret
  0000000000101D24: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
main:
  0000000000101D30: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000101D35: 55                 push        rbp
  0000000000101D36: 57                 push        rdi
  0000000000101D37: 48 81 EC F8 00 00  sub         rsp,0F8h
                    00
  0000000000101D3E: 48 8D 6C 24 30     lea         rbp,[rsp+30h]
  0000000000101D43: E8 38 FE FF FF     call        ?InitializeGlobals@@YAXXZ
  0000000000101D48: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000101D4F: 48 83 C0 48        add         rax,48h
  0000000000101D53: 48 8B D0           mov         rdx,rax
  0000000000101D56: 48 8D 0D 63 53 10  lea         rcx,[?display@@3VDisplay@@A]
                    00
  0000000000101D5D: E8 FE FE FF FF     call        ?SetDisplay@Display@@QEAAXPEAUGRAPHICS_DEVICE@@@Z
  0000000000101D62: 8B 15 44 47 00 00  mov         edx,dword ptr [1064ACh]
  0000000000101D68: 48 8D 0D 51 53 10  lea         rcx,[?display@@3VDisplay@@A]
                    00
  0000000000101D6F: E8 6C F7 FF FF     call        ?ColorScreen@Display@@QEAAXUBGRRPixel@@@Z
  0000000000101D74: 48 8D 15 45 53 10  lea         rdx,[?display@@3VDisplay@@A]
                    00
  0000000000101D7B: 48 8D 4D 00        lea         rcx,[rbp]
  0000000000101D7F: E8 8C F8 FF FF     call        ??0LoadingScreen@@QEAA@AEAVDisplay@@@Z
  0000000000101D84: 48 8D 45 00        lea         rax,[rbp]
  0000000000101D88: 48 89 05 F1 52 00  mov         qword ptr [?loading@@3PEAVLoadingScreen@@EA],rax
                    00
  0000000000101D8F: C7 45 20 00 00 00  mov         dword ptr [rbp+20h],0
                    00
  0000000000101D96: EB 08              jmp         0000000000101DA0
  0000000000101D98: 8B 45 20           mov         eax,dword ptr [rbp+20h]
  0000000000101D9B: FF C0              inc         eax
  0000000000101D9D: 89 45 20           mov         dword ptr [rbp+20h],eax
  0000000000101DA0: 0F B7 05 69 52 00  movzx       eax,word ptr [107010h]
                    00
  0000000000101DA7: FF C0              inc         eax
  0000000000101DA9: 99                 cdq
  0000000000101DAA: 83 E2 07           and         edx,7
  0000000000101DAD: 03 C2              add         eax,edx
  0000000000101DAF: C1 F8 03           sar         eax,3
  0000000000101DB2: 39 45 20           cmp         dword ptr [rbp+20h],eax
  0000000000101DB5: 7D 39              jge         0000000000101DF0
  0000000000101DB7: 8B 45 20           mov         eax,dword ptr [rbp+20h]
  0000000000101DBA: C1 E0 03           shl         eax,3
  0000000000101DBD: 48 98              cdqe
  0000000000101DBF: 48 8B 0D 4C 52 00  mov         rcx,qword ptr [107012h]
                    00
  0000000000101DC6: 48 03 C8           add         rcx,rax
  0000000000101DC9: 48 8B C1           mov         rax,rcx
  0000000000101DCC: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000101DD0: 48 8B 45 28        mov         rax,qword ptr [rbp+28h]
  0000000000101DD4: 4C 8B 08           mov         r9,qword ptr [rax]
  0000000000101DD7: 4C 8B 45 28        mov         r8,qword ptr [rbp+28h]
  0000000000101DDB: 48 8D 15 EE 46 00  lea         rdx,[??_C@_0P@LCOMGBIM@0x?$CF16x?3?50x?$CF16x@]
                    00
  0000000000101DE2: 48 8B 0D 97 52 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101DE9: E8 D2 FA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101DEE: EB A8              jmp         0000000000101D98
  0000000000101DF0: 41 B9 38 00 00 00  mov         r9d,38h
  0000000000101DF6: 4C 8D 05 43 52 00  lea         r8,[107040h]
                    00
  0000000000101DFD: 48 8D 15 DC 46 00  lea         rdx,[??_C@_0BP@EIALBIAB@KernelGDT?5?9?50x?$CF16x?0?5Size?3?50x?$CFx@]
                    00
  0000000000101E04: 48 8B 0D 75 52 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101E0B: E8 B0 FA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101E10: 0F B7 05 F9 51 00  movzx       eax,word ptr [107010h]
                    00
  0000000000101E17: 48 8B 0D F4 51 00  mov         rcx,qword ptr [107012h]
                    00
  0000000000101E1E: 48 89 4C 24 20     mov         qword ptr [rsp+20h],rcx
  0000000000101E23: 44 8B C8           mov         r9d,eax
  0000000000101E26: 4C 8D 05 E3 51 00  lea         r8,[107010h]
                    00
  0000000000101E2D: 48 8D 15 CC 46 00  lea         rdx,[??_C@_0CN@LKCENBAD@GDTR?5?$CG?30x?$CF16x?0?5Limit?30x?$CF08x?0?5Ad@]
                    00
  0000000000101E34: 48 8B 0D 45 52 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101E3B: E8 80 FA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101E40: E8 34 36 00 00     call        x64_ReadCS
  0000000000101E45: 66 89 45 30        mov         word ptr [rbp+30h],ax
  0000000000101E49: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000101E4D: 0F B7 4D 30        movzx       ecx,word ptr [rbp+30h]
  0000000000101E51: 66 83 E1 03        and         cx,3
  0000000000101E55: 0F B7 C9           movzx       ecx,cx
  0000000000101E58: 44 8B C8           mov         r9d,eax
  0000000000101E5B: 44 8B C1           mov         r8d,ecx
  0000000000101E5E: 48 8D 15 CB 46 00  lea         rdx,[??_C@_0BH@JDPADLC@CS?5RPL?5?$CFd?5Value?30x?$CF16x@]
                    00
  0000000000101E65: 48 8B 0D 14 52 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101E6C: E8 4F FA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101E71: E8 07 36 00 00     call        x64_ReadDS
  0000000000101E76: 0F B7 C0           movzx       eax,ax
  0000000000101E79: 89 85 B0 00 00 00  mov         dword ptr [rbp+0B0h],eax
  0000000000101E7F: E8 F5 35 00 00     call        x64_ReadCS
  0000000000101E84: 0F B7 C0           movzx       eax,ax
  0000000000101E87: 8B 8D B0 00 00 00  mov         ecx,dword ptr [rbp+0B0h]
  0000000000101E8D: 44 8B C9           mov         r9d,ecx
  0000000000101E90: 44 8B C0           mov         r8d,eax
  0000000000101E93: 48 8D 15 AE 46 00  lea         rdx,[??_C@_0BH@HEDENMBC@CS?3?50x?$CF16x?0?5DS?3?50x?$CF16x@]
                    00
  0000000000101E9A: 48 8B 0D DF 51 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101EA1: E8 1A FA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101EA6: E8 AC 35 00 00     call        x64_sti
  0000000000101EAB: E8 A4 35 00 00     call        x64_rflags
  0000000000101EB0: 4C 8B C0           mov         r8,rax
  0000000000101EB3: 48 8D 15 A6 46 00  lea         rdx,[??_C@_0P@BNCEKAJM@RFLAGS?3?50x?$CF16x@]
                    00
  0000000000101EBA: 48 8B 0D BF 51 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101EC1: E8 FA F9 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101EC6: 0F 01 15 43 51 00  lgdt        tbyte ptr [107010h]
                    00
  0000000000101ECD: 48 8D 45 34        lea         rax,[rbp+34h]
  0000000000101ED1: 48 8B F8           mov         rdi,rax
  0000000000101ED4: 33 C0              xor         eax,eax
  0000000000101ED6: B9 02 00 00 00     mov         ecx,2
  0000000000101EDB: F3 AA              rep stos    byte ptr [rdi]
  0000000000101EDD: 0F B7 45 34        movzx       eax,word ptr [rbp+34h]
  0000000000101EE1: 66 83 E0 07        and         ax,7
  0000000000101EE5: 66 83 C8 10        or          ax,10h
  0000000000101EE9: 66 89 45 34        mov         word ptr [rbp+34h],ax
  0000000000101EED: 48 8D 45 38        lea         rax,[rbp+38h]
  0000000000101EF1: 48 8B F8           mov         rdi,rax
  0000000000101EF4: 33 C0              xor         eax,eax
  0000000000101EF6: B9 02 00 00 00     mov         ecx,2
  0000000000101EFB: F3 AA              rep stos    byte ptr [rdi]
  0000000000101EFD: 0F B7 45 38        movzx       eax,word ptr [rbp+38h]
  0000000000101F01: 66 83 E0 07        and         ax,7
  0000000000101F05: 66 83 C8 08        or          ax,8
  0000000000101F09: 66 89 45 38        mov         word ptr [rbp+38h],ax
  0000000000101F0D: 48 8D 45 3C        lea         rax,[rbp+3Ch]
  0000000000101F11: 48 8B F8           mov         rdi,rax
  0000000000101F14: 33 C0              xor         eax,eax
  0000000000101F16: B9 02 00 00 00     mov         ecx,2
  0000000000101F1B: F3 AA              rep stos    byte ptr [rdi]
  0000000000101F1D: 0F B7 45 3C        movzx       eax,word ptr [rbp+3Ch]
  0000000000101F21: 66 83 E0 07        and         ax,7
  0000000000101F25: 66 83 C8 28        or          ax,28h
  0000000000101F29: 66 89 45 3C        mov         word ptr [rbp+3Ch],ax
  0000000000101F2D: 0F B7 45 3C        movzx       eax,word ptr [rbp+3Ch]
  0000000000101F31: 0F B7 4D 34        movzx       ecx,word ptr [rbp+34h]
  0000000000101F35: 0F B7 55 38        movzx       edx,word ptr [rbp+38h]
  0000000000101F39: 89 44 24 20        mov         dword ptr [rsp+20h],eax
  0000000000101F3D: 44 8B C9           mov         r9d,ecx
  0000000000101F40: 44 8B C2           mov         r8d,edx
  0000000000101F43: 48 8D 15 26 46 00  lea         rdx,[??_C@_0CF@CAFEFHHM@Code?3?50x?$CF4x?0?5Data?3?50x?$CF4x?0?5TSS?3?5@]
                    00
  0000000000101F4A: 48 8B 0D 2F 51 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101F51: E8 6A F9 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101F56: 0F B7 55 38        movzx       edx,word ptr [rbp+38h]
  0000000000101F5A: 0F B7 4D 34        movzx       ecx,word ptr [rbp+34h]
  0000000000101F5E: E8 F8 34 00 00     call        x64_update_segments
  0000000000101F63: E8 15 35 00 00     call        x64_ReadDS
  0000000000101F68: 0F B7 C0           movzx       eax,ax
  0000000000101F6B: 89 85 B0 00 00 00  mov         dword ptr [rbp+0B0h],eax
  0000000000101F71: E8 03 35 00 00     call        x64_ReadCS
  0000000000101F76: 0F B7 C0           movzx       eax,ax
  0000000000101F79: 8B 8D B0 00 00 00  mov         ecx,dword ptr [rbp+0B0h]
  0000000000101F7F: 44 8B C9           mov         r9d,ecx
  0000000000101F82: 44 8B C0           mov         r8d,eax
  0000000000101F85: 48 8D 15 BC 45 00  lea         rdx,[??_C@_0BH@HEDENMBC@CS?3?50x?$CF16x?0?5DS?3?50x?$CF16x@]
                    00
  0000000000101F8C: 48 8B 0D ED 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101F93: E8 28 F9 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101F98: F4                 hlt
  0000000000101F99: 0F B7 4D 3C        movzx       ecx,word ptr [rbp+3Ch]
  0000000000101F9D: E8 AE 34 00 00     call        x64_ltr
  0000000000101FA2: 0F 01 1D 71 50 00  lidt        tbyte ptr [10701Ah]
                    00
  0000000000101FA9: F4                 hlt
  0000000000101FAA: CC                 int         3
  0000000000101FAB: E8 F9 34 00 00     call        x64_ReadRCX
  0000000000101FB0: 48 89 85 B0 00 00  mov         qword ptr [rbp+0B0h],rax
                    00
  0000000000101FB7: E8 F1 34 00 00     call        x64_ReadRDX
  0000000000101FBC: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  0000000000101FC3: 4C 8B C9           mov         r9,rcx
  0000000000101FC6: 4C 8B C0           mov         r8,rax
  0000000000101FC9: 48 8D 15 C8 45 00  lea         rdx,[??_C@_0BJ@COOINDL@RDX?3?50x?$CF16x?0?5RCX?3?50x?$CF16x@]
                    00
  0000000000101FD0: 48 8B 0D A9 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101FD7: E8 E4 F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000101FDC: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000101FE3: 4C 8B 48 08        mov         r9,qword ptr [rax+8]
  0000000000101FE7: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000101FEE: 4C 8B 00           mov         r8,qword ptr [rax]
  0000000000101FF1: 48 8D 15 C0 45 00  lea         rdx,[??_C@_0CI@PDPNGPGI@MetalOS?4Kernel?5?9?5Base?30x?$CF16x?5Si@]
                    00
  0000000000101FF8: 48 8B 0D 81 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000101FFF: E8 BC F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000102004: 4C 8B 85 E0 00 00  mov         r8,qword ptr [rbp+0E0h]
                    00
  000000000010200B: 48 8D 15 CE 45 00  lea         rdx,[??_C@_0BG@OGJDAHDB@LOADER_PARAMS?3?50x?$CF16x@]
                    00
  0000000000102012: 48 8B 0D 67 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102019: E8 A2 F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010201E: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000102025: 4C 8B 40 68        mov         r8,qword ptr [rax+68h]
  0000000000102029: 48 8D 15 C8 45 00  lea         rdx,[??_C@_0BF@GKIOAOJM@ConfigTableSizes?3?5?$CFd@]
                    00
  0000000000102030: 48 8B 0D 49 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102037: E8 84 F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010203C: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000102043: 4C 8B 40 18        mov         r8,qword ptr [rax+18h]
  0000000000102047: 48 8D 15 C2 45 00  lea         rdx,[??_C@_0BC@EEGFBGJ@MemoryMap?3?50x?$CF16x@]
                    00
  000000000010204E: 48 8B 0D 2B 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102055: E8 66 F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010205A: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000102061: 4C 8B 40 48        mov         r8,qword ptr [rax+48h]
  0000000000102065: 48 8D 15 BC 45 00  lea         rdx,[??_C@_0CA@HOKFNFGI@Display?4FrameBufferBase?3?50x?$CF16x@]
                    00
  000000000010206C: 48 8B 0D 0D 50 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102073: E8 48 F8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000102078: 48 C7 44 24 28 00  mov         qword ptr [rsp+28h],1000h
                    10 00 00
  0000000000102081: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000102088: 48 8B 40 18        mov         rax,qword ptr [rax+18h]
  000000000010208C: 48 89 44 24 20     mov         qword ptr [rsp+20h],rax
  0000000000102091: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  0000000000102098: 44 8B 48 30        mov         r9d,dword ptr [rax+30h]
  000000000010209C: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  00000000001020A3: 4C 8B 40 28        mov         r8,qword ptr [rax+28h]
  00000000001020A7: 48 8B 85 E0 00 00  mov         rax,qword ptr [rbp+0E0h]
                    00
  00000000001020AE: 48 8B 50 20        mov         rdx,qword ptr [rax+20h]
  00000000001020B2: 48 8D 4D 40        lea         rcx,[rbp+40h]
  00000000001020B6: E8 25 00 00 00     call        ??0MemoryMap@@QEAA@_K0IPEAUEFI_MEMORY_DESCRIPTOR@@0@Z
  00000000001020BB: 48 8D 4D 40        lea         rcx,[rbp+40h]
  00000000001020BF: E8 BC 02 00 00     call        ?DumpMemoryMap@MemoryMap@@QEAAXXZ
  00000000001020C4: F4                 hlt
  00000000001020C5: 33 C0              xor         eax,eax
  00000000001020C7: 48 8D A5 C8 00 00  lea         rsp,[rbp+0C8h]
                    00
  00000000001020CE: 5F                 pop         rdi
  00000000001020CF: 5D                 pop         rbp
  00000000001020D0: C3                 ret
  00000000001020D1: CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC     ллллллллллллллл
??0MemoryMap@@QEAA@_K0IPEAUEFI_MEMORY_DESCRIPTOR@@0@Z:
  00000000001020E0: 44 89 4C 24 20     mov         dword ptr [rsp+20h],r9d
  00000000001020E5: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  00000000001020EA: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001020EF: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001020F4: 55                 push        rbp
  00000000001020F5: 48 83 EC 40        sub         rsp,40h
  00000000001020F9: 48 8B EC           mov         rbp,rsp
  00000000001020FC: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102100: 48 8D 0D 49 45 00  lea         rcx,[106650h]
                    00
  0000000000102107: 48 89 08           mov         qword ptr [rax],rcx
  000000000010210A: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010210E: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  0000000000102112: 48 89 48 08        mov         qword ptr [rax+8],rcx
  0000000000102116: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010211A: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  000000000010211E: 48 89 48 10        mov         qword ptr [rax+10h],rcx
  0000000000102122: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102126: 8B 4D 68           mov         ecx,dword ptr [rbp+68h]
  0000000000102129: 89 48 18           mov         dword ptr [rax+18h],ecx
  000000000010212C: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102130: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000102134: 48 89 48 20        mov         qword ptr [rax+20h],rcx
  0000000000102138: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010213C: 48 8B 4D 78        mov         rcx,qword ptr [rbp+78h]
  0000000000102140: 48 89 48 28        mov         qword ptr [rax+28h],rcx
  0000000000102144: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102148: 48 8D 65 40        lea         rsp,[rbp+40h]
  000000000010214C: 5D                 pop         rbp
  000000000010214D: C3                 ret
  000000000010214E: CC CC                                            лл
?AddressFree@MemoryMap@@QEAA_N_K@Z:
  0000000000102150: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000102155: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010215A: 55                 push        rbp
  000000000010215B: 48 83 EC 70        sub         rsp,70h
  000000000010215F: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000102164: 48 8B 55 68        mov         rdx,qword ptr [rbp+68h]
  0000000000102168: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  000000000010216C: E8 CF 06 00 00     call        ?ResolveAddress@MemoryMap@@AEAAPEAUEFI_MEMORY_DESCRIPTOR@@_K@Z
  0000000000102171: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102175: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102179: 83 38 07           cmp         dword ptr [rax],7
  000000000010217C: 75 09              jne         0000000000102187
  000000000010217E: C7 45 48 01 00 00  mov         dword ptr [rbp+48h],1
                    00
  0000000000102185: EB 07              jmp         000000000010218E
  0000000000102187: C7 45 48 00 00 00  mov         dword ptr [rbp+48h],0
                    00
  000000000010218E: 0F B6 45 48        movzx       eax,byte ptr [rbp+48h]
  0000000000102192: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000102196: 5D                 pop         rbp
  0000000000102197: C3                 ret
  0000000000102198: CC CC CC CC CC CC CC CC                          лллллллл
?AllocatePages@MemoryMap@@QEAA_KI@Z:
  00000000001021A0: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  00000000001021A4: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001021A9: 55                 push        rbp
  00000000001021AA: 48 81 EC 80 00 00  sub         rsp,80h
                    00
  00000000001021B1: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  00000000001021B6: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  00000000001021BA: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001021BE: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001021C2: EB 16              jmp         00000000001021DA
  00000000001021C4: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  00000000001021C8: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  00000000001021CC: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001021D0: 48 03 C8           add         rcx,rax
  00000000001021D3: 48 8B C1           mov         rax,rcx
  00000000001021D6: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001021DA: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  00000000001021DE: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001021E2: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  00000000001021E6: 48 03 41 08        add         rax,qword ptr [rcx+8]
  00000000001021EA: 48 39 45 00        cmp         qword ptr [rbp],rax
  00000000001021EE: 73 13              jae         0000000000102203
  00000000001021F0: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  00000000001021F3: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001021F7: 48 39 41 18        cmp         qword ptr [rcx+18h],rax
  00000000001021FB: 73 02              jae         00000000001021FF
  00000000001021FD: EB C5              jmp         00000000001021C4
  00000000001021FF: EB 02              jmp         0000000000102203
  0000000000102201: EB C1              jmp         00000000001021C4
  0000000000102203: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102207: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  000000000010220B: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  000000000010220F: 48 03 41 08        add         rax,qword ptr [rcx+8]
  0000000000102213: 48 39 45 00        cmp         qword ptr [rbp],rax
  0000000000102217: 72 0C              jb          0000000000102225
  0000000000102219: 48 8D 0D 60 46 00  lea         rcx,[??_C@_0DN@CAKMEMJD@current?5?$DM?5NextMemoryDescriptor?$CI@]
                    00
  0000000000102220: E8 EB F9 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  0000000000102225: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102228: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  000000000010222C: 48 39 41 18        cmp         qword ptr [rcx+18h],rax
  0000000000102230: 0F 86 32 01 00 00  jbe         0000000000102368
  0000000000102236: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010223A: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  000000000010223E: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000102242: 48 03 41 10        add         rax,qword ptr [rcx+10h]
  0000000000102246: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  000000000010224A: 48 3B 41 28        cmp         rax,qword ptr [rcx+28h]
  000000000010224E: 76 0C              jbe         000000000010225C
  0000000000102250: 48 8D 0D 69 46 00  lea         rcx,[??_C@_0DJ@KPPDOMCG@m_memoryMapSize?5?$CL?5m_memoryMapDe@]
                    00
  0000000000102257: E8 B4 F9 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  000000000010225C: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102260: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102264: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  0000000000102268: 48 2B C1           sub         rax,rcx
  000000000010226B: 48 99              cqo
  000000000010226D: B9 28 00 00 00     mov         ecx,28h
  0000000000102272: 48 F7 F9           idiv        rax,rcx
  0000000000102275: 33 D2              xor         edx,edx
  0000000000102277: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  000000000010227B: 48 F7 71 10        div         rax,qword ptr [rcx+10h]
  000000000010227F: 89 45 08           mov         dword ptr [rbp+8],eax
  0000000000102282: 33 D2              xor         edx,edx
  0000000000102284: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102288: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  000000000010228C: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000102290: 48 F7 71 10        div         rax,qword ptr [rcx+10h]
  0000000000102294: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  0000000000102297: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  000000000010229A: FF C8              dec         eax
  000000000010229C: 89 45 10           mov         dword ptr [rbp+10h],eax
  000000000010229F: EB 08              jmp         00000000001022A9
  00000000001022A1: 8B 45 10           mov         eax,dword ptr [rbp+10h]
  00000000001022A4: FF C8              dec         eax
  00000000001022A6: 89 45 10           mov         dword ptr [rbp+10h],eax
  00000000001022A9: 8B 45 08           mov         eax,dword ptr [rbp+8]
  00000000001022AC: 39 45 10           cmp         dword ptr [rbp+10h],eax
  00000000001022AF: 7C 4C              jl          00000000001022FD
  00000000001022B1: 48 63 45 10        movsxd      rax,dword ptr [rbp+10h]
  00000000001022B5: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  00000000001022B9: 48 8B 49 10        mov         rcx,qword ptr [rcx+10h]
  00000000001022BD: 48 0F AF C8        imul        rcx,rax
  00000000001022C1: 48 8B C1           mov         rax,rcx
  00000000001022C4: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  00000000001022C8: 48 03 41 20        add         rax,qword ptr [rcx+20h]
  00000000001022CC: 8B 4D 10           mov         ecx,dword ptr [rbp+10h]
  00000000001022CF: FF C1              inc         ecx
  00000000001022D1: 48 63 C9           movsxd      rcx,ecx
  00000000001022D4: 48 8B 55 70        mov         rdx,qword ptr [rbp+70h]
  00000000001022D8: 48 8B 52 10        mov         rdx,qword ptr [rdx+10h]
  00000000001022DC: 48 0F AF D1        imul        rdx,rcx
  00000000001022E0: 48 8B CA           mov         rcx,rdx
  00000000001022E3: 48 8B 55 70        mov         rdx,qword ptr [rbp+70h]
  00000000001022E7: 48 03 4A 20        add         rcx,qword ptr [rdx+20h]
  00000000001022EB: 48 8B 55 70        mov         rdx,qword ptr [rbp+70h]
  00000000001022EF: 44 8B 42 10        mov         r8d,dword ptr [rdx+10h]
  00000000001022F3: 48 8B D0           mov         rdx,rax
  00000000001022F6: E8 E5 05 00 00     call        ?memcpy@CRT@@SAXPEAX0I@Z
  00000000001022FB: EB A4              jmp         00000000001022A1
  00000000001022FD: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102301: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  0000000000102305: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000102309: 48 03 41 10        add         rax,qword ptr [rcx+10h]
  000000000010230D: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000102311: 48 89 41 08        mov         qword ptr [rcx+8],rax
  0000000000102315: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102319: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  000000000010231D: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102321: 48 03 C8           add         rcx,rax
  0000000000102324: 48 8B C1           mov         rax,rcx
  0000000000102327: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  000000000010232B: 69 45 78 00 10 00  imul        eax,dword ptr [rbp+78h],1000h
                    00
  0000000000102332: 8B C0              mov         eax,eax
  0000000000102334: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102338: 48 03 41 08        add         rax,qword ptr [rcx+8]
  000000000010233C: 48 8B 4D 18        mov         rcx,qword ptr [rbp+18h]
  0000000000102340: 48 89 41 08        mov         qword ptr [rcx+8],rax
  0000000000102344: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102347: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  000000000010234B: 48 8B 49 18        mov         rcx,qword ptr [rcx+18h]
  000000000010234F: 48 2B C8           sub         rcx,rax
  0000000000102352: 48 8B C1           mov         rax,rcx
  0000000000102355: 48 8B 4D 18        mov         rcx,qword ptr [rbp+18h]
  0000000000102359: 48 89 41 18        mov         qword ptr [rcx+18h],rax
  000000000010235D: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102360: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102364: 48 89 41 18        mov         qword ptr [rcx+18h],rax
  0000000000102368: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010236C: C7 00 02 00 00 00  mov         dword ptr [rax],2
  0000000000102372: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102376: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  000000000010237A: 48 8D 65 60        lea         rsp,[rbp+60h]
  000000000010237E: 5D                 pop         rbp
  000000000010237F: C3                 ret
?DumpMemoryMap@MemoryMap@@QEAAXXZ:
  0000000000102380: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000102385: 55                 push        rbp
  0000000000102386: 48 81 EC 90 00 00  sub         rsp,90h
                    00
  000000000010238D: 48 8D 6C 24 40     lea         rbp,[rsp+40h]
  0000000000102392: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000102396: 4C 8B 48 10        mov         r9,qword ptr [rax+10h]
  000000000010239A: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  000000000010239E: 4C 8B 40 08        mov         r8,qword ptr [rax+8]
  00000000001023A2: 48 8D 15 57 45 00  lea         rdx,[??_C@_0BO@IGLJAJBN@MapSize?3?50x?$CFx?0?5DescSize?3?50x?$CFx@]
                    00
  00000000001023A9: 48 8B 0D D0 4C 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  00000000001023B0: E8 0B F5 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  00000000001023B5: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001023B9: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001023BD: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001023C1: EB 16              jmp         00000000001023D9
  00000000001023C3: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001023C7: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  00000000001023CB: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001023CF: 48 03 C8           add         rcx,rax
  00000000001023D2: 48 8B C1           mov         rax,rcx
  00000000001023D5: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001023D9: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001023DD: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001023E1: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001023E5: 48 03 41 08        add         rax,qword ptr [rcx+8]
  00000000001023E9: 48 39 45 00        cmp         qword ptr [rbp],rax
  00000000001023ED: 73 51              jae         0000000000102440
  00000000001023EF: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001023F3: 8B 00              mov         eax,dword ptr [rax]
  00000000001023F5: 48 6B C0 1B        imul        rax,rax,1Bh
  00000000001023F9: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001023FD: 48 03 01           add         rax,qword ptr [rcx]
  0000000000102400: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102404: 48 8B 49 18        mov         rcx,qword ptr [rcx+18h]
  0000000000102408: 48 89 4C 24 30     mov         qword ptr [rsp+30h],rcx
  000000000010240D: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102411: 48 8B 49 10        mov         rcx,qword ptr [rcx+10h]
  0000000000102415: 48 89 4C 24 28     mov         qword ptr [rsp+28h],rcx
  000000000010241A: 48 89 44 24 20     mov         qword ptr [rsp+20h],rax
  000000000010241F: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102423: 4C 8B 48 08        mov         r9,qword ptr [rax+8]
  0000000000102427: 44 8B 45 00        mov         r8d,dword ptr [rbp]
  000000000010242B: 48 8D 15 EE 44 00  lea         rdx,[??_C@_0CB@KEMHOPLN@A?3?$CF08x?5S?3?$CF08x?5T?3?$CFs?5V?3?5?$CFd?5N?3?50x?$CF@]
                    00
  0000000000102432: 48 8B 0D 47 4C 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102439: E8 82 F4 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010243E: EB 83              jmp         00000000001023C3
  0000000000102440: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000102444: 5D                 pop         rbp
  0000000000102445: C3                 ret
  0000000000102446: CC CC CC CC CC CC CC CC CC CC                    лллллллллл
?GetLowestFree@MemoryMap@@QEAA_KI@Z:
  0000000000102450: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  0000000000102454: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000102459: 55                 push        rbp
  000000000010245A: 48 83 EC 70        sub         rsp,70h
  000000000010245E: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000102463: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000102467: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  000000000010246B: 48 89 45 00        mov         qword ptr [rbp],rax
  000000000010246F: EB 16              jmp         0000000000102487
  0000000000102471: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000102475: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  0000000000102479: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  000000000010247D: 48 03 C8           add         rcx,rax
  0000000000102480: 48 8B C1           mov         rax,rcx
  0000000000102483: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102487: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  000000000010248B: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  000000000010248F: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000102493: 48 03 41 08        add         rax,qword ptr [rcx+8]
  0000000000102497: 48 39 45 00        cmp         qword ptr [rbp],rax
  000000000010249B: 73 19              jae         00000000001024B6
  000000000010249D: 8B 45 68           mov         eax,dword ptr [rbp+68h]
  00000000001024A0: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001024A4: 48 39 41 18        cmp         qword ptr [rcx+18h],rax
  00000000001024A8: 72 0A              jb          00000000001024B4
  00000000001024AA: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001024AE: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  00000000001024B2: EB 15              jmp         00000000001024C9
  00000000001024B4: EB BB              jmp         0000000000102471
  00000000001024B6: 33 C0              xor         eax,eax
  00000000001024B8: 83 F8 01           cmp         eax,1
  00000000001024BB: 74 0C              je          00000000001024C9
  00000000001024BD: 48 8D 0D B0 43 00  lea         rcx,[??_C@_05LAPONLG@false@]
                    00
  00000000001024C4: E8 47 F7 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  00000000001024C9: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001024CD: 5D                 pop         rbp
  00000000001024CE: C3                 ret
  00000000001024CF: CC                                               л
?MergeConventionalPages@MemoryMap@@QEAAXXZ:
  00000000001024D0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001024D5: 55                 push        rbp
  00000000001024D6: 48 81 EC A0 00 00  sub         rsp,0A0h
                    00
  00000000001024DD: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  00000000001024E2: 48 C7 45 00 00 00  mov         qword ptr [rbp],0
                    00 00
  00000000001024EA: C7 45 08 0F 00 00  mov         dword ptr [rbp+8],0Fh
                    00
  00000000001024F1: 33 D2              xor         edx,edx
  00000000001024F3: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  00000000001024FA: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  00000000001024FE: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  0000000000102505: 48 F7 71 10        div         rax,qword ptr [rcx+10h]
  0000000000102509: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  000000000010250C: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  0000000000102513: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  0000000000102517: 48 89 45 10        mov         qword ptr [rbp+10h],rax
  000000000010251B: EB 19              jmp         0000000000102536
  000000000010251D: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  0000000000102524: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  0000000000102528: 48 8B 4D 10        mov         rcx,qword ptr [rbp+10h]
  000000000010252C: 48 03 C8           add         rcx,rax
  000000000010252F: 48 8B C1           mov         rax,rcx
  0000000000102532: 48 89 45 10        mov         qword ptr [rbp+10h],rax
  0000000000102536: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010253D: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  0000000000102541: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  0000000000102548: 48 03 41 08        add         rax,qword ptr [rcx+8]
  000000000010254C: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  0000000000102553: 48 2B 41 10        sub         rax,qword ptr [rcx+10h]
  0000000000102557: 48 39 45 10        cmp         qword ptr [rbp+10h],rax
  000000000010255B: 73 38              jae         0000000000102595
  000000000010255D: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  0000000000102564: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  0000000000102568: 48 8B 4D 10        mov         rcx,qword ptr [rbp+10h]
  000000000010256C: 48 03 C8           add         rcx,rax
  000000000010256F: 48 8B C1           mov         rax,rcx
  0000000000102572: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000102576: 48 8B 45 10        mov         rax,qword ptr [rbp+10h]
  000000000010257A: 83 38 07           cmp         dword ptr [rax],7
  000000000010257D: 75 14              jne         0000000000102593
  000000000010257F: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000102583: 83 38 07           cmp         dword ptr [rax],7
  0000000000102586: 75 0B              jne         0000000000102593
  0000000000102588: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010258C: 48 FF C0           inc         rax
  000000000010258F: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102593: EB 88              jmp         000000000010251D
  0000000000102595: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  000000000010259C: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001025A0: 48 0F AF 48 10     imul        rcx,qword ptr [rax+10h]
  00000000001025A5: 48 8B C1           mov         rax,rcx
  00000000001025A8: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  00000000001025AF: 48 8B 49 08        mov         rcx,qword ptr [rcx+8]
  00000000001025B3: 48 2B C8           sub         rcx,rax
  00000000001025B6: 48 8B C1           mov         rax,rcx
  00000000001025B9: 89 45 20           mov         dword ptr [rbp+20h],eax
  00000000001025BC: 4C 8B 45 00        mov         r8,qword ptr [rbp]
  00000000001025C0: 48 8D 15 39 42 00  lea         rdx,[??_C@_05IOODGHGJ@m?3?5?$CFd@]
                    00
  00000000001025C7: 48 8B 0D B2 4A 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  00000000001025CE: E8 ED F2 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  00000000001025D3: 44 8B 4D 20        mov         r9d,dword ptr [rbp+20h]
  00000000001025D7: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  00000000001025DE: 4C 8B 40 08        mov         r8,qword ptr [rax+8]
  00000000001025E2: 48 8D 15 1F 42 00  lea         rdx,[??_C@_0M@PDEDNFMH@B?3?5?$CFx?5A?3?5?$CFx@]
                    00
  00000000001025E9: 48 8B 0D 90 4A 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  00000000001025F0: E8 CB F2 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  00000000001025F5: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  00000000001025FC: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  0000000000102600: 48 C1 E8 00        shr         rax,0
  0000000000102604: 8B 4D 20           mov         ecx,dword ptr [rbp+20h]
  0000000000102607: C1 E9 0C           shr         ecx,0Ch
  000000000010260A: 8B C9              mov         ecx,ecx
  000000000010260C: 48 3B C1           cmp         rax,rcx
  000000000010260F: 75 0C              jne         000000000010261D
  0000000000102611: 48 8D 0D 08 42 00  lea         rcx,[??_C@_0EF@OBBADAFB@?$CIm_memoryMapSize?5?$DO?$DO?5EFI_PAGE_SI@]
                    00
  0000000000102618: E8 F3 F5 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  000000000010261D: 48 8B 85 90 00 00  mov         rax,qword ptr [rbp+90h]
                    00
  0000000000102624: 48 8B 50 20        mov         rdx,qword ptr [rax+20h]
  0000000000102628: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  000000000010262F: E8 0C 02 00 00     call        ?ResolveAddress@MemoryMap@@AEAAPEAUEFI_MEMORY_DESCRIPTOR@@_K@Z
  0000000000102634: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000102638: 48 8B 45 28        mov         rax,qword ptr [rbp+28h]
  000000000010263C: 4C 8B 48 18        mov         r9,qword ptr [rax+18h]
  0000000000102640: 48 8B 45 28        mov         rax,qword ptr [rbp+28h]
  0000000000102644: 4C 8B 40 08        mov         r8,qword ptr [rax+8]
  0000000000102648: 48 8D 15 19 42 00  lea         rdx,[??_C@_0M@PAEFOOLI@F?3?5?$CFx?5A?3?5?$CFx@]
                    00
  000000000010264F: 48 8B 0D 2A 4A 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000102656: E8 65 F2 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010265B: E9 50 01 00 00     jmp         00000000001027B0
  0000000000102660: C7 45 30 00 00 00  mov         dword ptr [rbp+30h],0
                    00
  0000000000102667: EB 08              jmp         0000000000102671
  0000000000102669: 8B 45 30           mov         eax,dword ptr [rbp+30h]
  000000000010266C: FF C0              inc         eax
  000000000010266E: 89 45 30           mov         dword ptr [rbp+30h],eax
  0000000000102671: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  0000000000102674: FF C8              dec         eax
  0000000000102676: 39 45 30           cmp         dword ptr [rbp+30h],eax
  0000000000102679: 0F 83 31 01 00 00  jae         00000000001027B0
  000000000010267F: 8B 45 30           mov         eax,dword ptr [rbp+30h]
  0000000000102682: 48 6B C0 28        imul        rax,rax,28h
  0000000000102686: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  000000000010268D: 48 8B 49 20        mov         rcx,qword ptr [rcx+20h]
  0000000000102691: 83 3C 01 07        cmp         dword ptr [rcx+rax],7
  0000000000102695: 74 02              je          0000000000102699
  0000000000102697: EB D0              jmp         0000000000102669
  0000000000102699: 8B 45 30           mov         eax,dword ptr [rbp+30h]
  000000000010269C: FF C0              inc         eax
  000000000010269E: 89 45 34           mov         dword ptr [rbp+34h],eax
  00000000001026A1: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  00000000001026A4: 39 45 34           cmp         dword ptr [rbp+34h],eax
  00000000001026A7: 0F 83 8E 00 00 00  jae         000000000010273B
  00000000001026AD: 8B 45 34           mov         eax,dword ptr [rbp+34h]
  00000000001026B0: 48 6B C0 28        imul        rax,rax,28h
  00000000001026B4: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  00000000001026BB: 48 8B 49 20        mov         rcx,qword ptr [rcx+20h]
  00000000001026BF: 83 3C 01 07        cmp         dword ptr [rcx+rax],7
  00000000001026C3: 75 76              jne         000000000010273B
  00000000001026C5: 8B 45 30           mov         eax,dword ptr [rbp+30h]
  00000000001026C8: 48 6B C0 28        imul        rax,rax,28h
  00000000001026CC: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  00000000001026D3: 48 8B 49 20        mov         rcx,qword ptr [rcx+20h]
  00000000001026D7: 8B 55 34           mov         edx,dword ptr [rbp+34h]
  00000000001026DA: 48 6B D2 28        imul        rdx,rdx,28h
  00000000001026DE: 4C 8B 85 90 00 00  mov         r8,qword ptr [rbp+90h]
                    00
  00000000001026E5: 4D 8B 40 20        mov         r8,qword ptr [r8+20h]
  00000000001026E9: 48 8B 44 01 18     mov         rax,qword ptr [rcx+rax+18h]
  00000000001026EE: 49 03 44 10 18     add         rax,qword ptr [r8+rdx+18h]
  00000000001026F3: 8B 4D 30           mov         ecx,dword ptr [rbp+30h]
  00000000001026F6: 48 6B C9 28        imul        rcx,rcx,28h
  00000000001026FA: 48 8B 95 90 00 00  mov         rdx,qword ptr [rbp+90h]
                    00
  0000000000102701: 48 8B 52 20        mov         rdx,qword ptr [rdx+20h]
  0000000000102705: 48 89 44 0A 18     mov         qword ptr [rdx+rcx+18h],rax
  000000000010270A: 8B 45 34           mov         eax,dword ptr [rbp+34h]
  000000000010270D: 48 6B C0 28        imul        rax,rax,28h
  0000000000102711: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  0000000000102718: 48 8B 49 20        mov         rcx,qword ptr [rcx+20h]
  000000000010271C: C7 04 01 0F 00 00  mov         dword ptr [rcx+rax],0Fh
                    00
  0000000000102723: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102727: 48 FF C0           inc         rax
  000000000010272A: 48 89 45 00        mov         qword ptr [rbp],rax
  000000000010272E: 8B 45 34           mov         eax,dword ptr [rbp+34h]
  0000000000102731: FF C0              inc         eax
  0000000000102733: 89 45 34           mov         dword ptr [rbp+34h],eax
  0000000000102736: E9 66 FF FF FF     jmp         00000000001026A1
  000000000010273B: 8B 45 34           mov         eax,dword ptr [rbp+34h]
  000000000010273E: 8B 4D 0C           mov         ecx,dword ptr [rbp+0Ch]
  0000000000102741: 2B C8              sub         ecx,eax
  0000000000102743: 8B C1              mov         eax,ecx
  0000000000102745: 8B C0              mov         eax,eax
  0000000000102747: 48 8B 8D 90 00 00  mov         rcx,qword ptr [rbp+90h]
                    00
  000000000010274E: 48 8B 49 10        mov         rcx,qword ptr [rcx+10h]
  0000000000102752: 48 0F AF C8        imul        rcx,rax
  0000000000102756: 48 8B C1           mov         rax,rcx
  0000000000102759: 8B 4D 34           mov         ecx,dword ptr [rbp+34h]
  000000000010275C: 48 6B C9 28        imul        rcx,rcx,28h
  0000000000102760: 48 8B 95 90 00 00  mov         rdx,qword ptr [rbp+90h]
                    00
  0000000000102767: 48 03 4A 20        add         rcx,qword ptr [rdx+20h]
  000000000010276B: 8B 55 30           mov         edx,dword ptr [rbp+30h]
  000000000010276E: FF C2              inc         edx
  0000000000102770: 8B D2              mov         edx,edx
  0000000000102772: 48 6B D2 28        imul        rdx,rdx,28h
  0000000000102776: 4C 8B 85 90 00 00  mov         r8,qword ptr [rbp+90h]
                    00
  000000000010277D: 49 03 50 20        add         rdx,qword ptr [r8+20h]
  0000000000102781: 48 89 55 78        mov         qword ptr [rbp+78h],rdx
  0000000000102785: 44 8B C0           mov         r8d,eax
  0000000000102788: 48 8B D1           mov         rdx,rcx
  000000000010278B: 48 8B 45 78        mov         rax,qword ptr [rbp+78h]
  000000000010278F: 48 8B C8           mov         rcx,rax
  0000000000102792: E8 49 01 00 00     call        ?memcpy@CRT@@SAXPEAX0I@Z
  0000000000102797: 8B 45 30           mov         eax,dword ptr [rbp+30h]
  000000000010279A: 8B 4D 34           mov         ecx,dword ptr [rbp+34h]
  000000000010279D: 2B C8              sub         ecx,eax
  000000000010279F: 8B C1              mov         eax,ecx
  00000000001027A1: 8B 4D 0C           mov         ecx,dword ptr [rbp+0Ch]
  00000000001027A4: 2B C8              sub         ecx,eax
  00000000001027A6: 8B C1              mov         eax,ecx
  00000000001027A8: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  00000000001027AB: E9 B9 FE FF FF     jmp         0000000000102669
  00000000001027B0: 48 8D A5 80 00 00  lea         rsp,[rbp+80h]
                    00
  00000000001027B7: 5D                 pop         rbp
  00000000001027B8: C3                 ret
  00000000001027B9: CC CC CC CC CC CC CC                             ллллллл
?ReclaimBootPages@MemoryMap@@QEAAXXZ:
  00000000001027C0: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001027C5: 55                 push        rbp
  00000000001027C6: 48 83 EC 50        sub         rsp,50h
  00000000001027CA: 48 8B EC           mov         rbp,rsp
  00000000001027CD: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001027D1: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001027D5: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001027D9: EB 16              jmp         00000000001027F1
  00000000001027DB: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001027DF: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  00000000001027E3: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001027E7: 48 03 C8           add         rcx,rax
  00000000001027EA: 48 8B C1           mov         rax,rcx
  00000000001027ED: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001027F1: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001027F5: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  00000000001027F9: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001027FD: 48 03 41 08        add         rax,qword ptr [rcx+8]
  0000000000102801: 48 39 45 00        cmp         qword ptr [rbp],rax
  0000000000102805: 73 27              jae         000000000010282E
  0000000000102807: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010280B: 83 38 03           cmp         dword ptr [rax],3
  000000000010280E: 74 12              je          0000000000102822
  0000000000102810: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102814: 83 38 04           cmp         dword ptr [rax],4
  0000000000102817: 74 09              je          0000000000102822
  0000000000102819: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010281D: 83 38 01           cmp         dword ptr [rax],1
  0000000000102820: 75 0A              jne         000000000010282C
  0000000000102822: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102826: C7 00 07 00 00 00  mov         dword ptr [rax],7
  000000000010282C: EB AD              jmp         00000000001027DB
  000000000010282E: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000102832: 5D                 pop         rbp
  0000000000102833: C3                 ret
  0000000000102834: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
?ResolveAddress@MemoryMap@@AEAAPEAUEFI_MEMORY_DESCRIPTOR@@_K@Z:
  0000000000102840: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000102845: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  000000000010284A: 55                 push        rbp
  000000000010284B: 48 83 EC 70        sub         rsp,70h
  000000000010284F: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000102854: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000102858: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  000000000010285C: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102860: EB 16              jmp         0000000000102878
  0000000000102862: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000102866: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  000000000010286A: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  000000000010286E: 48 03 C8           add         rcx,rax
  0000000000102871: 48 8B C1           mov         rax,rcx
  0000000000102874: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102878: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  000000000010287C: 48 8B 40 20        mov         rax,qword ptr [rax+20h]
  0000000000102880: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000102884: 48 03 41 08        add         rax,qword ptr [rcx+8]
  0000000000102888: 48 39 45 00        cmp         qword ptr [rbp],rax
  000000000010288C: 73 36              jae         00000000001028C4
  000000000010288E: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102892: 48 8B 4D 68        mov         rcx,qword ptr [rbp+68h]
  0000000000102896: 48 39 48 08        cmp         qword ptr [rax+8],rcx
  000000000010289A: 77 26              ja          00000000001028C2
  000000000010289C: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001028A0: 48 8B 40 08        mov         rax,qword ptr [rax+8]
  00000000001028A4: 48 8B 4D 68        mov         rcx,qword ptr [rbp+68h]
  00000000001028A8: 48 2B C8           sub         rcx,rax
  00000000001028AB: 48 8B C1           mov         rax,rcx
  00000000001028AE: 48 C1 E8 0C        shr         rax,0Ch
  00000000001028B2: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  00000000001028B6: 48 3B 41 18        cmp         rax,qword ptr [rcx+18h]
  00000000001028BA: 73 06              jae         00000000001028C2
  00000000001028BC: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001028C0: EB 0E              jmp         00000000001028D0
  00000000001028C2: EB 9E              jmp         0000000000102862
  00000000001028C4: 48 8D 0D 7D 40 00  lea         rcx,[??_C@_0BG@KKJJEKMJ@ResolveAddress?5failed@]
                    00
  00000000001028CB: E8 40 F3 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  00000000001028D0: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001028D4: 5D                 pop         rbp
  00000000001028D5: C3                 ret
  00000000001028D6: CC CC CC CC CC CC CC CC CC CC                    лллллллллл
?memcpy@CRT@@SAXPEAX0I@Z:
  00000000001028E0: 44 89 44 24 18     mov         dword ptr [rsp+18h],r8d
  00000000001028E5: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001028EA: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001028EF: 55                 push        rbp
  00000000001028F0: 48 83 EC 70        sub         rsp,70h
  00000000001028F4: 48 8B EC           mov         rbp,rsp
  00000000001028F7: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  00000000001028FE: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102902: 48 8B 85 88 00 00  mov         rax,qword ptr [rbp+88h]
                    00
  0000000000102909: 48 89 45 08        mov         qword ptr [rbp+8],rax
  000000000010290D: C7 45 10 08 00 00  mov         dword ptr [rbp+10h],8
                    00
  0000000000102914: C7 45 14 00 00 00  mov         dword ptr [rbp+14h],0
                    00
  000000000010291B: EB 08              jmp         0000000000102925
  000000000010291D: 8B 45 14           mov         eax,dword ptr [rbp+14h]
  0000000000102920: FF C0              inc         eax
  0000000000102922: 89 45 14           mov         dword ptr [rbp+14h],eax
  0000000000102925: 33 D2              xor         edx,edx
  0000000000102927: 8B 85 90 00 00 00  mov         eax,dword ptr [rbp+90h]
  000000000010292D: F7 75 10           div         eax,dword ptr [rbp+10h]
  0000000000102930: 39 45 14           cmp         dword ptr [rbp+14h],eax
  0000000000102933: 73 28              jae         000000000010295D
  0000000000102935: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102939: 48 8B 4D 08        mov         rcx,qword ptr [rbp+8]
  000000000010293D: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000102940: 48 89 08           mov         qword ptr [rax],rcx
  0000000000102943: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102947: 48 83 C0 08        add         rax,8
  000000000010294B: 48 89 45 00        mov         qword ptr [rbp],rax
  000000000010294F: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000102953: 48 83 C0 08        add         rax,8
  0000000000102957: 48 89 45 08        mov         qword ptr [rbp+8],rax
  000000000010295B: EB C0              jmp         000000000010291D
  000000000010295D: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102961: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000102965: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000102969: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  000000000010296D: C7 45 28 00 00 00  mov         dword ptr [rbp+28h],0
                    00
  0000000000102974: EB 08              jmp         000000000010297E
  0000000000102976: 8B 45 28           mov         eax,dword ptr [rbp+28h]
  0000000000102979: FF C0              inc         eax
  000000000010297B: 89 45 28           mov         dword ptr [rbp+28h],eax
  000000000010297E: 33 D2              xor         edx,edx
  0000000000102980: 8B 85 90 00 00 00  mov         eax,dword ptr [rbp+90h]
  0000000000102986: F7 75 10           div         eax,dword ptr [rbp+10h]
  0000000000102989: 8B C2              mov         eax,edx
  000000000010298B: 39 45 28           cmp         dword ptr [rbp+28h],eax
  000000000010298E: 73 25              jae         00000000001029B5
  0000000000102990: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000102994: 48 8B 4D 20        mov         rcx,qword ptr [rbp+20h]
  0000000000102998: 0F B6 09           movzx       ecx,byte ptr [rcx]
  000000000010299B: 88 08              mov         byte ptr [rax],cl
  000000000010299D: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  00000000001029A1: 48 FF C0           inc         rax
  00000000001029A4: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  00000000001029A8: 48 8B 45 20        mov         rax,qword ptr [rbp+20h]
  00000000001029AC: 48 FF C0           inc         rax
  00000000001029AF: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  00000000001029B3: EB C1              jmp         0000000000102976
  00000000001029B5: 48 8D 65 70        lea         rsp,[rbp+70h]
  00000000001029B9: 5D                 pop         rbp
  00000000001029BA: C3                 ret
  00000000001029BB: CC CC CC CC CC                                   ллллл
?imax@String@@CAHHH@Z:
  00000000001029C0: 89 54 24 10        mov         dword ptr [rsp+10h],edx
  00000000001029C4: 89 4C 24 08        mov         dword ptr [rsp+8],ecx
  00000000001029C8: 55                 push        rbp
  00000000001029C9: 48 83 EC 50        sub         rsp,50h
  00000000001029CD: 48 8B EC           mov         rbp,rsp
  00000000001029D0: 8B 45 68           mov         eax,dword ptr [rbp+68h]
  00000000001029D3: 39 45 60           cmp         dword ptr [rbp+60h],eax
  00000000001029D6: 7E 08              jle         00000000001029E0
  00000000001029D8: 8B 45 60           mov         eax,dword ptr [rbp+60h]
  00000000001029DB: 89 45 40           mov         dword ptr [rbp+40h],eax
  00000000001029DE: EB 06              jmp         00000000001029E6
  00000000001029E0: 8B 45 68           mov         eax,dword ptr [rbp+68h]
  00000000001029E3: 89 45 40           mov         dword ptr [rbp+40h],eax
  00000000001029E6: 8B 45 40           mov         eax,dword ptr [rbp+40h]
  00000000001029E9: 48 8D 65 50        lea         rsp,[rbp+50h]
  00000000001029ED: 5D                 pop         rbp
  00000000001029EE: C3                 ret
  00000000001029EF: CC                                               л
?ksprintn@String@@SAPEADPEAD_KHPEAHH@Z:
  00000000001029F0: 4C 89 4C 24 20     mov         qword ptr [rsp+20h],r9
  00000000001029F5: 44 89 44 24 18     mov         dword ptr [rsp+18h],r8d
  00000000001029FA: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001029FF: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000102A04: 55                 push        rbp
  0000000000102A05: 48 83 EC 60        sub         rsp,60h
  0000000000102A09: 48 8B EC           mov         rbp,rsp
  0000000000102A0C: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102A10: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102A14: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102A18: C6 00 00           mov         byte ptr [rax],0
  0000000000102A1B: 48 63 85 80 00 00  movsxd      rax,dword ptr [rbp+80h]
                    00
  0000000000102A22: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000102A26: 33 D2              xor         edx,edx
  0000000000102A28: 48 8B 45 78        mov         rax,qword ptr [rbp+78h]
  0000000000102A2C: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000102A30: 48 F7 F1           div         rax,rcx
  0000000000102A33: 48 8B C2           mov         rax,rdx
  0000000000102A36: 48 8D 0D 23 3F 00  lea         rcx,[106960h]
                    00
  0000000000102A3D: 0F B6 04 01        movzx       eax,byte ptr [rcx+rax]
  0000000000102A41: 88 45 08           mov         byte ptr [rbp+8],al
  0000000000102A44: 83 BD 90 00 00 00  cmp         dword ptr [rbp+90h],0
                    00
  0000000000102A4B: 74 31              je          0000000000102A7E
  0000000000102A4D: 0F BE 45 08        movsx       eax,byte ptr [rbp+8]
  0000000000102A51: 83 F8 61           cmp         eax,61h
  0000000000102A54: 7C 12              jl          0000000000102A68
  0000000000102A56: 0F BE 45 08        movsx       eax,byte ptr [rbp+8]
  0000000000102A5A: 83 F8 7A           cmp         eax,7Ah
  0000000000102A5D: 7F 09              jg          0000000000102A68
  0000000000102A5F: C7 45 4C 01 00 00  mov         dword ptr [rbp+4Ch],1
                    00
  0000000000102A66: EB 07              jmp         0000000000102A6F
  0000000000102A68: C7 45 4C 00 00 00  mov         dword ptr [rbp+4Ch],0
                    00
  0000000000102A6F: 0F BE 45 08        movsx       eax,byte ptr [rbp+8]
  0000000000102A73: 6B 4D 4C 20        imul        ecx,dword ptr [rbp+4Ch],20h
  0000000000102A77: 2B C1              sub         eax,ecx
  0000000000102A79: 89 45 50           mov         dword ptr [rbp+50h],eax
  0000000000102A7C: EB 07              jmp         0000000000102A85
  0000000000102A7E: 0F BE 45 08        movsx       eax,byte ptr [rbp+8]
  0000000000102A82: 89 45 50           mov         dword ptr [rbp+50h],eax
  0000000000102A85: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102A89: 48 FF C0           inc         rax
  0000000000102A8C: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000102A90: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102A94: 0F B6 4D 50        movzx       ecx,byte ptr [rbp+50h]
  0000000000102A98: 88 08              mov         byte ptr [rax],cl
  0000000000102A9A: 48 63 85 80 00 00  movsxd      rax,dword ptr [rbp+80h]
                    00
  0000000000102AA1: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000102AA5: 33 D2              xor         edx,edx
  0000000000102AA7: 48 8B 45 78        mov         rax,qword ptr [rbp+78h]
  0000000000102AAB: 48 8B 4D 50        mov         rcx,qword ptr [rbp+50h]
  0000000000102AAF: 48 F7 F1           div         rax,rcx
  0000000000102AB2: 48 89 45 78        mov         qword ptr [rbp+78h],rax
  0000000000102AB6: 48 83 7D 78 00     cmp         qword ptr [rbp+78h],0
  0000000000102ABB: 0F 85 5A FF FF FF  jne         0000000000102A1B
  0000000000102AC1: 48 83 BD 88 00 00  cmp         qword ptr [rbp+88h],0
                    00 00
  0000000000102AC9: 74 17              je          0000000000102AE2
  0000000000102ACB: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000102ACF: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000102AD3: 48 2B C8           sub         rcx,rax
  0000000000102AD6: 48 8B C1           mov         rax,rcx
  0000000000102AD9: 48 8B 8D 88 00 00  mov         rcx,qword ptr [rbp+88h]
                    00
  0000000000102AE0: 89 01              mov         dword ptr [rcx],eax
  0000000000102AE2: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000102AE6: 48 8D 65 60        lea         rsp,[rbp+60h]
  0000000000102AEA: 5D                 pop         rbp
  0000000000102AEB: C3                 ret
  0000000000102AEC: CC CC CC CC                                      лллл
?kvprintf@String@@SAHPEBDP6AXHPEAX@Z1HPEAD@Z:
  0000000000102AF0: 44 89 4C 24 20     mov         dword ptr [rsp+20h],r9d
  0000000000102AF5: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000102AFA: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000102AFF: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000102B04: 55                 push        rbp
  0000000000102B05: 48 81 EC B0 01 00  sub         rsp,1B0h
                    00
  0000000000102B0C: 48 8D 6C 24 30     lea         rbp,[rsp+30h]
  0000000000102B11: C7 85 D4 00 00 00  mov         dword ptr [rbp+0D4h],0
                    00 00 00 00
  0000000000102B1B: C7 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],0
                    00 00 00 00
  0000000000102B25: 48 C7 85 80 00 00  mov         qword ptr [rbp+80h],0
                    00 00 00 00 00
  0000000000102B30: 48 C7 45 68 00 00  mov         qword ptr [rbp+68h],0
                    00 00
  0000000000102B38: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000102B40: 75 0D              jne         0000000000102B4F
  0000000000102B42: 48 8B 85 A0 01 00  mov         rax,qword ptr [rbp+1A0h]
                    00
  0000000000102B49: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000102B4D: EB 08              jmp         0000000000102B57
  0000000000102B4F: 48 C7 45 50 00 00  mov         qword ptr [rbp+50h],0
                    00 00
  0000000000102B57: 48 83 BD 90 01 00  cmp         qword ptr [rbp+190h],0
                    00 00
  0000000000102B5F: 75 0E              jne         0000000000102B6F
  0000000000102B61: 48 8D 05 30 3E 00  lea         rax,[??_C@_0M@DCHBFLHK@?$CIfmt?5null?$CJ?6@]
                    00
  0000000000102B68: 48 89 85 90 01 00  mov         qword ptr [rbp+190h],rax
                    00
  0000000000102B6F: 83 BD A8 01 00 00  cmp         dword ptr [rbp+1A8h],2
                    02
  0000000000102B76: 7C 09              jl          0000000000102B81
  0000000000102B78: 83 BD A8 01 00 00  cmp         dword ptr [rbp+1A8h],24h
                    24
  0000000000102B7F: 7E 0A              jle         0000000000102B8B
  0000000000102B81: C7 85 A8 01 00 00  mov         dword ptr [rbp+1A8h],0Ah
                    0A 00 00 00
  0000000000102B8B: C6 85 D0 00 00 00  mov         byte ptr [rbp+0D0h],20h
                    20
  0000000000102B92: C7 85 98 00 00 00  mov         dword ptr [rbp+98h],0
                    00 00 00 00
  0000000000102B9C: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102BA3: 0F B6 00           movzx       eax,byte ptr [rax]
  0000000000102BA6: 89 45 78           mov         dword ptr [rbp+78h],eax
  0000000000102BA9: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102BAC: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102BB2: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102BB9: 48 FF C0           inc         rax
  0000000000102BBC: 48 89 85 90 01 00  mov         qword ptr [rbp+190h],rax
                    00
  0000000000102BC3: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],25h
                    25
  0000000000102BCA: 75 09              jne         0000000000102BD5
  0000000000102BCC: 83 BD D4 00 00 00  cmp         dword ptr [rbp+0D4h],0
                    00
  0000000000102BD3: 74 64              je          0000000000102C39
  0000000000102BD5: 83 7D 78 00        cmp         dword ptr [rbp+78h],0
  0000000000102BD9: 75 0B              jne         0000000000102BE6
  0000000000102BDB: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000102BE1: E9 3B 1D 00 00     jmp         0000000000104921
  0000000000102BE6: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102BE9: 89 85 DC 00 00 00  mov         dword ptr [rbp+0DCh],eax
  0000000000102BEF: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000102BF7: 74 15              je          0000000000102C0E
  0000000000102BF9: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000102C00: 8B 8D DC 00 00 00  mov         ecx,dword ptr [rbp+0DCh]
  0000000000102C06: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000102C0C: EB 18              jmp         0000000000102C26
  0000000000102C0E: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102C12: 0F B6 8D DC 00 00  movzx       ecx,byte ptr [rbp+0DCh]
                    00
  0000000000102C19: 88 08              mov         byte ptr [rax],cl
  0000000000102C1B: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102C1F: 48 FF C0           inc         rax
  0000000000102C22: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000102C26: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000102C2C: FF C0              inc         eax
  0000000000102C2E: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000102C34: E9 63 FF FF FF     jmp         0000000000102B9C
  0000000000102C39: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102C40: 48 FF C8           dec         rax
  0000000000102C43: 48 89 45 60        mov         qword ptr [rbp+60h],rax
  0000000000102C47: C7 85 90 00 00 00  mov         dword ptr [rbp+90h],0
                    00 00 00 00
  0000000000102C51: C7 85 8C 00 00 00  mov         dword ptr [rbp+8Ch],0
                    00 00 00 00
  0000000000102C5B: C7 85 9C 00 00 00  mov         dword ptr [rbp+9Ch],0
                    00 00 00 00
  0000000000102C65: C7 85 A0 00 00 00  mov         dword ptr [rbp+0A0h],0
                    00 00 00 00
  0000000000102C6F: C7 85 A4 00 00 00  mov         dword ptr [rbp+0A4h],0
                    00 00 00 00
  0000000000102C79: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],0
                    00 00 00 00
  0000000000102C83: C7 85 AC 00 00 00  mov         dword ptr [rbp+0ACh],0
                    00 00 00 00
  0000000000102C8D: C7 85 C4 00 00 00  mov         dword ptr [rbp+0C4h],0
                    00 00 00 00
  0000000000102C97: C7 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],0
                    00 00 00 00
  0000000000102CA1: C7 85 CC 00 00 00  mov         dword ptr [rbp+0CCh],0
                    00 00 00 00
  0000000000102CAB: C7 85 B0 00 00 00  mov         dword ptr [rbp+0B0h],0
                    00 00 00 00
  0000000000102CB5: C7 85 B4 00 00 00  mov         dword ptr [rbp+0B4h],0
                    00 00 00 00
  0000000000102CBF: C7 85 B8 00 00 00  mov         dword ptr [rbp+0B8h],0
                    00 00 00 00
  0000000000102CC9: C7 85 BC 00 00 00  mov         dword ptr [rbp+0BCh],0
                    00 00 00 00
  0000000000102CD3: C7 85 C0 00 00 00  mov         dword ptr [rbp+0C0h],0
                    00 00 00 00
  0000000000102CDD: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102CE4: 0F B6 00           movzx       eax,byte ptr [rax]
  0000000000102CE7: 89 45 78           mov         dword ptr [rbp+78h],eax
  0000000000102CEA: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102CED: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102CF3: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102CFA: 48 FF C0           inc         rax
  0000000000102CFD: 48 89 85 90 01 00  mov         qword ptr [rbp+190h],rax
                    00
  0000000000102D04: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000102D0A: 89 85 7C 01 00 00  mov         dword ptr [rbp+17Ch],eax
  0000000000102D10: 8B 85 7C 01 00 00  mov         eax,dword ptr [rbp+17Ch]
  0000000000102D16: 83 E8 23           sub         eax,23h
  0000000000102D19: 89 85 7C 01 00 00  mov         dword ptr [rbp+17Ch],eax
  0000000000102D1F: 83 BD 7C 01 00 00  cmp         dword ptr [rbp+17Ch],57h
                    57
  0000000000102D26: 0F 87 7A 1B 00 00  ja          00000000001048A6
  0000000000102D2C: 48 63 85 7C 01 00  movsxd      rax,dword ptr [rbp+17Ch]
                    00
  0000000000102D33: 48 8D 0D C6 D2 FF  lea         rcx,[100000h]
                    FF
  0000000000102D3A: 0F B6 84 01 9C 49  movzx       eax,byte ptr [rcx+rax+499Ch]
                    00 00
  0000000000102D42: 8B 84 81 2C 49 00  mov         eax,dword ptr [rcx+rax*4+492Ch]
                    00
  0000000000102D49: 48 03 C1           add         rax,rcx
  0000000000102D4C: FF E0              jmp         rax
  0000000000102D4E: C7 85 AC 00 00 00  mov         dword ptr [rbp+0ACh],1
                    01 00 00 00
  0000000000102D58: EB 83              jmp         0000000000102CDD
  0000000000102D5A: C7 85 A0 00 00 00  mov         dword ptr [rbp+0A0h],1
                    01 00 00 00
  0000000000102D64: E9 74 FF FF FF     jmp         0000000000102CDD
  0000000000102D69: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],1
                    01 00 00 00
  0000000000102D73: E9 65 FF FF FF     jmp         0000000000102CDD
  0000000000102D78: C7 85 9C 00 00 00  mov         dword ptr [rbp+9Ch],1
                    01 00 00 00
  0000000000102D82: E9 56 FF FF FF     jmp         0000000000102CDD
  0000000000102D87: 8B 45 78           mov         eax,dword ptr [rbp+78h]
  0000000000102D8A: 89 85 E0 00 00 00  mov         dword ptr [rbp+0E0h],eax
  0000000000102D90: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000102D98: 74 15              je          0000000000102DAF
  0000000000102D9A: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000102DA1: 8B 8D E0 00 00 00  mov         ecx,dword ptr [rbp+0E0h]
  0000000000102DA7: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000102DAD: EB 18              jmp         0000000000102DC7
  0000000000102DAF: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102DB3: 0F B6 8D E0 00 00  movzx       ecx,byte ptr [rbp+0E0h]
                    00
  0000000000102DBA: 88 08              mov         byte ptr [rax],cl
  0000000000102DBC: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102DC0: 48 FF C0           inc         rax
  0000000000102DC3: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000102DC7: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000102DCD: FF C0              inc         eax
  0000000000102DCF: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000102DD5: E9 42 1B 00 00     jmp         000000000010491C
  0000000000102DDA: 83 BD AC 00 00 00  cmp         dword ptr [rbp+0ACh],0
                    00
  0000000000102DE1: 0F 85 9F 00 00 00  jne         0000000000102E86
  0000000000102DE7: 33 C0              xor         eax,eax
  0000000000102DE9: 85 C0              test        eax,eax
  0000000000102DEB: 74 27              je          0000000000102E14
  0000000000102DED: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102DF4: 48 83 C0 08        add         rax,8
  0000000000102DF8: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000102DFF: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102E06: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000102E0A: 8B 00              mov         eax,dword ptr [rax]
  0000000000102E0C: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102E12: EB 22              jmp         0000000000102E36
  0000000000102E14: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102E1B: 48 83 C0 08        add         rax,8
  0000000000102E1F: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000102E26: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102E2D: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000102E30: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102E36: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000102E3C: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000102E42: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000102E49: 7D 39              jge         0000000000102E84
  0000000000102E4B: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  0000000000102E52: 75 0C              jne         0000000000102E60
  0000000000102E54: C7 85 78 01 00 00  mov         dword ptr [rbp+178h],1
                    01 00 00 00
  0000000000102E5E: EB 0A              jmp         0000000000102E6A
  0000000000102E60: C7 85 78 01 00 00  mov         dword ptr [rbp+178h],0
                    00 00 00 00
  0000000000102E6A: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000102E70: 89 85 9C 00 00 00  mov         dword ptr [rbp+9Ch],eax
  0000000000102E76: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000102E7C: F7 D8              neg         eax
  0000000000102E7E: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000102E84: EB 5B              jmp         0000000000102EE1
  0000000000102E86: 33 C0              xor         eax,eax
  0000000000102E88: 85 C0              test        eax,eax
  0000000000102E8A: 74 27              je          0000000000102EB3
  0000000000102E8C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102E93: 48 83 C0 08        add         rax,8
  0000000000102E97: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000102E9E: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102EA5: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000102EA9: 8B 00              mov         eax,dword ptr [rax]
  0000000000102EAB: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102EB1: EB 22              jmp         0000000000102ED5
  0000000000102EB3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102EBA: 48 83 C0 08        add         rax,8
  0000000000102EBE: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000102EC5: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000102ECC: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000102ECF: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102ED5: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000102EDB: 89 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],eax
  0000000000102EE1: E9 F7 FD FF FF     jmp         0000000000102CDD
  0000000000102EE6: 83 BD AC 00 00 00  cmp         dword ptr [rbp+0ACh],0
                    00
  0000000000102EED: 75 0C              jne         0000000000102EFB
  0000000000102EEF: C6 85 D0 00 00 00  mov         byte ptr [rbp+0D0h],30h
                    30
  0000000000102EF6: E9 E2 FD FF FF     jmp         0000000000102CDD
  0000000000102EFB: C7 45 7C 00 00 00  mov         dword ptr [rbp+7Ch],0
                    00
  0000000000102F02: EB 11              jmp         0000000000102F15
  0000000000102F04: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102F0B: 48 FF C0           inc         rax
  0000000000102F0E: 48 89 85 90 01 00  mov         qword ptr [rbp+190h],rax
                    00
  0000000000102F15: 6B 45 7C 0A        imul        eax,dword ptr [rbp+7Ch],0Ah
  0000000000102F19: 8B 4D 78           mov         ecx,dword ptr [rbp+78h]
  0000000000102F1C: 8D 44 08 D0        lea         eax,[rax+rcx-30h]
  0000000000102F20: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  0000000000102F23: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  0000000000102F2A: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000102F2D: 89 45 78           mov         dword ptr [rbp+78h],eax
  0000000000102F30: 83 7D 78 30        cmp         dword ptr [rbp+78h],30h
  0000000000102F34: 7C 06              jl          0000000000102F3C
  0000000000102F36: 83 7D 78 39        cmp         dword ptr [rbp+78h],39h
  0000000000102F3A: 7E 02              jle         0000000000102F3E
  0000000000102F3C: EB 02              jmp         0000000000102F40
  0000000000102F3E: EB C4              jmp         0000000000102F04
  0000000000102F40: 83 BD AC 00 00 00  cmp         dword ptr [rbp+0ACh],0
                    00
  0000000000102F47: 74 0B              je          0000000000102F54
  0000000000102F49: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000102F4C: 89 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],eax
  0000000000102F52: EB 09              jmp         0000000000102F5D
  0000000000102F54: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000102F57: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000102F5D: E9 7B FD FF FF     jmp         0000000000102CDD
  0000000000102F62: C7 85 9C 00 00 00  mov         dword ptr [rbp+9Ch],1
                    01 00 00 00
  0000000000102F6C: C7 85 C4 00 00 00  mov         dword ptr [rbp+0C4h],1
                    01 00 00 00
  0000000000102F76: E9 6E 0B 00 00     jmp         0000000000103AE9
  0000000000102F7B: E9 69 0B 00 00     jmp         0000000000103AE9
  0000000000102F80: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000102F86: FF C8              dec         eax
  0000000000102F88: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000102F8E: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  0000000000102F95: 0F 85 80 00 00 00  jne         000000000010301B
  0000000000102F9B: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000102FA2: 7E 77              jle         000000000010301B
  0000000000102FA4: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000102FAA: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000102FB0: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000102FB6: FF C8              dec         eax
  0000000000102FB8: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000102FBE: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  0000000000102FC5: 74 54              je          000000000010301B
  0000000000102FC7: 0F BE 85 D0 00 00  movsx       eax,byte ptr [rbp+0D0h]
                    00
  0000000000102FCE: 89 85 E4 00 00 00  mov         dword ptr [rbp+0E4h],eax
  0000000000102FD4: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000102FDC: 74 15              je          0000000000102FF3
  0000000000102FDE: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000102FE5: 8B 8D E4 00 00 00  mov         ecx,dword ptr [rbp+0E4h]
  0000000000102FEB: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000102FF1: EB 18              jmp         000000000010300B
  0000000000102FF3: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000102FF7: 0F B6 8D E4 00 00  movzx       ecx,byte ptr [rbp+0E4h]
                    00
  0000000000102FFE: 88 08              mov         byte ptr [rax],cl
  0000000000103000: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103004: 48 FF C0           inc         rax
  0000000000103007: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010300B: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000103011: FF C0              inc         eax
  0000000000103013: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000103019: EB 89              jmp         0000000000102FA4
  000000000010301B: 33 C0              xor         eax,eax
  000000000010301D: 85 C0              test        eax,eax
  000000000010301F: 74 27              je          0000000000103048
  0000000000103021: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103028: 48 83 C0 08        add         rax,8
  000000000010302C: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103033: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010303A: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010303E: 8B 00              mov         eax,dword ptr [rax]
  0000000000103040: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103046: EB 22              jmp         000000000010306A
  0000000000103048: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010304F: 48 83 C0 08        add         rax,8
  0000000000103053: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010305A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103061: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000103064: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  000000000010306A: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000103070: 89 85 E8 00 00 00  mov         dword ptr [rbp+0E8h],eax
  0000000000103076: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  000000000010307E: 74 15              je          0000000000103095
  0000000000103080: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103087: 8B 8D E8 00 00 00  mov         ecx,dword ptr [rbp+0E8h]
  000000000010308D: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000103093: EB 18              jmp         00000000001030AD
  0000000000103095: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103099: 0F B6 8D E8 00 00  movzx       ecx,byte ptr [rbp+0E8h]
                    00
  00000000001030A0: 88 08              mov         byte ptr [rax],cl
  00000000001030A2: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001030A6: 48 FF C0           inc         rax
  00000000001030A9: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001030AD: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001030B3: FF C0              inc         eax
  00000000001030B5: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001030BB: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  00000000001030C2: 0F 84 80 00 00 00  je          0000000000103148
  00000000001030C8: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  00000000001030CF: 7E 77              jle         0000000000103148
  00000000001030D1: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  00000000001030D7: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  00000000001030DD: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  00000000001030E3: FF C8              dec         eax
  00000000001030E5: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  00000000001030EB: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  00000000001030F2: 74 54              je          0000000000103148
  00000000001030F4: 0F BE 85 D0 00 00  movsx       eax,byte ptr [rbp+0D0h]
                    00
  00000000001030FB: 89 85 EC 00 00 00  mov         dword ptr [rbp+0ECh],eax
  0000000000103101: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000103109: 74 15              je          0000000000103120
  000000000010310B: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103112: 8B 8D EC 00 00 00  mov         ecx,dword ptr [rbp+0ECh]
  0000000000103118: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  000000000010311E: EB 18              jmp         0000000000103138
  0000000000103120: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103124: 0F B6 8D EC 00 00  movzx       ecx,byte ptr [rbp+0ECh]
                    00
  000000000010312B: 88 08              mov         byte ptr [rax],cl
  000000000010312D: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103131: 48 FF C0           inc         rax
  0000000000103134: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000103138: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  000000000010313E: FF C0              inc         eax
  0000000000103140: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000103146: EB 89              jmp         00000000001030D1
  0000000000103148: E9 CF 17 00 00     jmp         000000000010491C
  000000000010314D: 33 C0              xor         eax,eax
  000000000010314F: 85 C0              test        eax,eax
  0000000000103151: 74 29              je          000000000010317C
  0000000000103153: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010315A: 48 83 C0 08        add         rax,8
  000000000010315E: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103165: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010316C: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103170: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103173: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  000000000010317A: EB 24              jmp         00000000001031A0
  000000000010317C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103183: 48 83 C0 08        add         rax,8
  0000000000103187: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010318E: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103195: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103199: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001031A0: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  00000000001031A7: 48 89 45 70        mov         qword ptr [rbp+70h],rax
  00000000001031AB: 33 C0              xor         eax,eax
  00000000001031AD: 85 C0              test        eax,eax
  00000000001031AF: 74 29              je          00000000001031DA
  00000000001031B1: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001031B8: 48 83 C0 08        add         rax,8
  00000000001031BC: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001031C3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001031CA: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001031CE: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001031D1: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001031D8: EB 24              jmp         00000000001031FE
  00000000001031DA: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001031E1: 48 83 C0 08        add         rax,8
  00000000001031E5: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001031EC: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001031F3: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001031F7: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001031FE: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103205: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  0000000000103209: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000103210: 75 0A              jne         000000000010321C
  0000000000103212: C7 85 98 00 00 00  mov         dword ptr [rbp+98h],10h
                    10 00 00 00
  000000000010321C: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000103222: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103228: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  000000000010322E: FF C8              dec         eax
  0000000000103230: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000103236: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  000000000010323D: 0F 84 51 01 00 00  je          0000000000103394
  0000000000103243: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000103247: 0F B6 00           movzx       eax,byte ptr [rax]
  000000000010324A: C1 F8 04           sar         eax,4
  000000000010324D: 48 98              cdqe
  000000000010324F: 48 8D 0D 0A 37 00  lea         rcx,[106960h]
                    00
  0000000000103256: 0F BE 04 01        movsx       eax,byte ptr [rcx+rax]
  000000000010325A: 89 85 F0 00 00 00  mov         dword ptr [rbp+0F0h],eax
  0000000000103260: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000103268: 74 15              je          000000000010327F
  000000000010326A: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103271: 8B 8D F0 00 00 00  mov         ecx,dword ptr [rbp+0F0h]
  0000000000103277: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  000000000010327D: EB 18              jmp         0000000000103297
  000000000010327F: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103283: 0F B6 8D F0 00 00  movzx       ecx,byte ptr [rbp+0F0h]
                    00
  000000000010328A: 88 08              mov         byte ptr [rax],cl
  000000000010328C: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103290: 48 FF C0           inc         rax
  0000000000103293: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000103297: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  000000000010329D: FF C0              inc         eax
  000000000010329F: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001032A5: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  00000000001032A9: 0F B6 00           movzx       eax,byte ptr [rax]
  00000000001032AC: 83 E0 0F           and         eax,0Fh
  00000000001032AF: 48 98              cdqe
  00000000001032B1: 48 8D 0D A8 36 00  lea         rcx,[106960h]
                    00
  00000000001032B8: 0F BE 04 01        movsx       eax,byte ptr [rcx+rax]
  00000000001032BC: 89 85 F4 00 00 00  mov         dword ptr [rbp+0F4h],eax
  00000000001032C2: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001032CA: 74 15              je          00000000001032E1
  00000000001032CC: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001032D3: 8B 8D F4 00 00 00  mov         ecx,dword ptr [rbp+0F4h]
  00000000001032D9: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001032DF: EB 18              jmp         00000000001032F9
  00000000001032E1: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001032E5: 0F B6 8D F4 00 00  movzx       ecx,byte ptr [rbp+0F4h]
                    00
  00000000001032EC: 88 08              mov         byte ptr [rax],cl
  00000000001032EE: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001032F2: 48 FF C0           inc         rax
  00000000001032F5: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001032F9: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001032FF: FF C0              inc         eax
  0000000000103301: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000103307: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  000000000010330B: 48 FF C0           inc         rax
  000000000010330E: 48 89 45 70        mov         qword ptr [rbp+70h],rax
  0000000000103312: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000103319: 74 74              je          000000000010338F
  000000000010331B: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  000000000010331F: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000103323: EB 0B              jmp         0000000000103330
  0000000000103325: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000103329: 48 FF C0           inc         rax
  000000000010332C: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000103330: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000103334: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000103337: 85 C0              test        eax,eax
  0000000000103339: 74 54              je          000000000010338F
  000000000010333B: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  000000000010333F: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000103342: 89 85 F8 00 00 00  mov         dword ptr [rbp+0F8h],eax
  0000000000103348: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000103350: 74 15              je          0000000000103367
  0000000000103352: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103359: 8B 8D F8 00 00 00  mov         ecx,dword ptr [rbp+0F8h]
  000000000010335F: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000103365: EB 18              jmp         000000000010337F
  0000000000103367: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010336B: 0F B6 8D F8 00 00  movzx       ecx,byte ptr [rbp+0F8h]
                    00
  0000000000103372: 88 08              mov         byte ptr [rax],cl
  0000000000103374: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103378: 48 FF C0           inc         rax
  000000000010337B: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010337F: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000103385: FF C0              inc         eax
  0000000000103387: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  000000000010338D: EB 96              jmp         0000000000103325
  000000000010338F: E9 88 FE FF FF     jmp         000000000010321C
  0000000000103394: E9 83 15 00 00     jmp         000000000010491C
  0000000000103399: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],0Ah
                    0A 00 00 00
  00000000001033A3: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],1
                    01 00 00 00
  00000000001033AD: E9 28 0B 00 00     jmp         0000000000103EDA
  00000000001033B2: E9 23 0B 00 00     jmp         0000000000103EDA
  00000000001033B7: 83 BD B4 00 00 00  cmp         dword ptr [rbp+0B4h],0
                    00
  00000000001033BE: 74 16              je          00000000001033D6
  00000000001033C0: C7 85 B4 00 00 00  mov         dword ptr [rbp+0B4h],0
                    00 00 00 00
  00000000001033CA: C7 85 B0 00 00 00  mov         dword ptr [rbp+0B0h],1
                    01 00 00 00
  00000000001033D4: EB 0A              jmp         00000000001033E0
  00000000001033D6: C7 85 B4 00 00 00  mov         dword ptr [rbp+0B4h],1
                    01 00 00 00
  00000000001033E0: E9 F8 F8 FF FF     jmp         0000000000102CDD
  00000000001033E5: C7 85 B8 00 00 00  mov         dword ptr [rbp+0B8h],1
                    01 00 00 00
  00000000001033EF: E9 E9 F8 FF FF     jmp         0000000000102CDD
  00000000001033F4: 83 BD 8C 00 00 00  cmp         dword ptr [rbp+8Ch],0
                    00
  00000000001033FB: 74 16              je          0000000000103413
  00000000001033FD: C7 85 8C 00 00 00  mov         dword ptr [rbp+8Ch],0
                    00 00 00 00
  0000000000103407: C7 85 90 00 00 00  mov         dword ptr [rbp+90h],1
                    01 00 00 00
  0000000000103411: EB 0A              jmp         000000000010341D
  0000000000103413: C7 85 8C 00 00 00  mov         dword ptr [rbp+8Ch],1
                    01 00 00 00
  000000000010341D: E9 BB F8 FF FF     jmp         0000000000102CDD
  0000000000103422: 83 BD B8 00 00 00  cmp         dword ptr [rbp+0B8h],0
                    00
  0000000000103429: 74 69              je          0000000000103494
  000000000010342B: 33 C0              xor         eax,eax
  000000000010342D: 85 C0              test        eax,eax
  000000000010342F: 74 29              je          000000000010345A
  0000000000103431: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103438: 48 83 C0 08        add         rax,8
  000000000010343C: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103443: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010344A: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010344E: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103451: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103458: EB 24              jmp         000000000010347E
  000000000010345A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103461: 48 83 C0 08        add         rax,8
  0000000000103465: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010346C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103473: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103477: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  000000000010347E: 48 63 85 D8 00 00  movsxd      rax,dword ptr [rbp+0D8h]
                    00
  0000000000103485: 48 8B 8D 78 01 00  mov         rcx,qword ptr [rbp+178h]
                    00
  000000000010348C: 48 89 01           mov         qword ptr [rcx],rax
  000000000010348F: E9 96 02 00 00     jmp         000000000010372A
  0000000000103494: 83 BD 90 00 00 00  cmp         dword ptr [rbp+90h],0
                    00
  000000000010349B: 74 69              je          0000000000103506
  000000000010349D: 33 C0              xor         eax,eax
  000000000010349F: 85 C0              test        eax,eax
  00000000001034A1: 74 29              je          00000000001034CC
  00000000001034A3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001034AA: 48 83 C0 08        add         rax,8
  00000000001034AE: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001034B5: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001034BC: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001034C0: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001034C3: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001034CA: EB 24              jmp         00000000001034F0
  00000000001034CC: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001034D3: 48 83 C0 08        add         rax,8
  00000000001034D7: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001034DE: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001034E5: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001034E9: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001034F0: 48 63 85 D8 00 00  movsxd      rax,dword ptr [rbp+0D8h]
                    00
  00000000001034F7: 48 8B 8D 78 01 00  mov         rcx,qword ptr [rbp+178h]
                    00
  00000000001034FE: 48 89 01           mov         qword ptr [rcx],rax
  0000000000103501: E9 24 02 00 00     jmp         000000000010372A
  0000000000103506: 83 BD 8C 00 00 00  cmp         dword ptr [rbp+8Ch],0
                    00
  000000000010350D: 74 67              je          0000000000103576
  000000000010350F: 33 C0              xor         eax,eax
  0000000000103511: 85 C0              test        eax,eax
  0000000000103513: 74 29              je          000000000010353E
  0000000000103515: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010351C: 48 83 C0 08        add         rax,8
  0000000000103520: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103527: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010352E: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103532: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103535: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  000000000010353C: EB 24              jmp         0000000000103562
  000000000010353E: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103545: 48 83 C0 08        add         rax,8
  0000000000103549: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103550: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103557: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010355B: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103562: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103569: 8B 8D D8 00 00 00  mov         ecx,dword ptr [rbp+0D8h]
  000000000010356F: 89 08              mov         dword ptr [rax],ecx
  0000000000103571: E9 B4 01 00 00     jmp         000000000010372A
  0000000000103576: 83 BD C0 00 00 00  cmp         dword ptr [rbp+0C0h],0
                    00
  000000000010357D: 74 69              je          00000000001035E8
  000000000010357F: 33 C0              xor         eax,eax
  0000000000103581: 85 C0              test        eax,eax
  0000000000103583: 74 29              je          00000000001035AE
  0000000000103585: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010358C: 48 83 C0 08        add         rax,8
  0000000000103590: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103597: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010359E: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001035A2: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001035A5: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001035AC: EB 24              jmp         00000000001035D2
  00000000001035AE: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001035B5: 48 83 C0 08        add         rax,8
  00000000001035B9: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001035C0: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001035C7: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001035CB: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001035D2: 48 63 85 D8 00 00  movsxd      rax,dword ptr [rbp+0D8h]
                    00
  00000000001035D9: 48 8B 8D 78 01 00  mov         rcx,qword ptr [rbp+178h]
                    00
  00000000001035E0: 48 89 01           mov         qword ptr [rcx],rax
  00000000001035E3: E9 42 01 00 00     jmp         000000000010372A
  00000000001035E8: 83 BD B4 00 00 00  cmp         dword ptr [rbp+0B4h],0
                    00
  00000000001035EF: 74 69              je          000000000010365A
  00000000001035F1: 33 C0              xor         eax,eax
  00000000001035F3: 85 C0              test        eax,eax
  00000000001035F5: 74 29              je          0000000000103620
  00000000001035F7: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001035FE: 48 83 C0 08        add         rax,8
  0000000000103602: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103609: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103610: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103614: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103617: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  000000000010361E: EB 24              jmp         0000000000103644
  0000000000103620: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103627: 48 83 C0 08        add         rax,8
  000000000010362B: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103632: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103639: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010363D: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103644: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  000000000010364B: 0F B7 8D D8 00 00  movzx       ecx,word ptr [rbp+0D8h]
                    00
  0000000000103652: 66 89 08           mov         word ptr [rax],cx
  0000000000103655: E9 D0 00 00 00     jmp         000000000010372A
  000000000010365A: 83 BD B0 00 00 00  cmp         dword ptr [rbp+0B0h],0
                    00
  0000000000103661: 74 65              je          00000000001036C8
  0000000000103663: 33 C0              xor         eax,eax
  0000000000103665: 85 C0              test        eax,eax
  0000000000103667: 74 29              je          0000000000103692
  0000000000103669: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103670: 48 83 C0 08        add         rax,8
  0000000000103674: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010367B: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103682: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103686: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103689: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103690: EB 24              jmp         00000000001036B6
  0000000000103692: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103699: 48 83 C0 08        add         rax,8
  000000000010369D: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001036A4: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001036AB: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001036AF: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001036B6: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  00000000001036BD: 0F B6 8D D8 00 00  movzx       ecx,byte ptr [rbp+0D8h]
                    00
  00000000001036C4: 88 08              mov         byte ptr [rax],cl
  00000000001036C6: EB 62              jmp         000000000010372A
  00000000001036C8: 33 C0              xor         eax,eax
  00000000001036CA: 85 C0              test        eax,eax
  00000000001036CC: 74 29              je          00000000001036F7
  00000000001036CE: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001036D5: 48 83 C0 08        add         rax,8
  00000000001036D9: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001036E0: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001036E7: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001036EB: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001036EE: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001036F5: EB 24              jmp         000000000010371B
  00000000001036F7: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001036FE: 48 83 C0 08        add         rax,8
  0000000000103702: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103709: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103710: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103714: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  000000000010371B: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103722: 8B 8D D8 00 00 00  mov         ecx,dword ptr [rbp+0D8h]
  0000000000103728: 89 08              mov         dword ptr [rax],ecx
  000000000010372A: E9 ED 11 00 00     jmp         000000000010491C
  000000000010372F: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],8
                    08 00 00 00
  0000000000103739: E9 AB 03 00 00     jmp         0000000000103AE9
  000000000010373E: E9 A6 03 00 00     jmp         0000000000103AE9
  0000000000103743: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],10h
                    10 00 00 00
  000000000010374D: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000103754: 75 0C              jne         0000000000103762
  0000000000103756: C7 85 78 01 00 00  mov         dword ptr [rbp+178h],1
                    01 00 00 00
  0000000000103760: EB 0A              jmp         000000000010376C
  0000000000103762: C7 85 78 01 00 00  mov         dword ptr [rbp+178h],0
                    00 00 00 00
  000000000010376C: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000103772: 89 85 A0 00 00 00  mov         dword ptr [rbp+0A0h],eax
  0000000000103778: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],0
                    00 00 00 00
  0000000000103782: 33 C0              xor         eax,eax
  0000000000103784: 85 C0              test        eax,eax
  0000000000103786: 74 29              je          00000000001037B1
  0000000000103788: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010378F: 48 83 C0 08        add         rax,8
  0000000000103793: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010379A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001037A1: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001037A5: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001037A8: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001037AF: EB 24              jmp         00000000001037D5
  00000000001037B1: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001037B8: 48 83 C0 08        add         rax,8
  00000000001037BC: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001037C3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001037CA: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001037CE: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001037D5: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  00000000001037DC: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  00000000001037E3: E9 4B 0A 00 00     jmp         0000000000104233
  00000000001037E8: E9 46 0A 00 00     jmp         0000000000104233
  00000000001037ED: C7 85 90 00 00 00  mov         dword ptr [rbp+90h],1
                    01 00 00 00
  00000000001037F7: E9 E1 F4 FF FF     jmp         0000000000102CDD
  00000000001037FC: 8B 85 A8 01 00 00  mov         eax,dword ptr [rbp+1A8h]
  0000000000103802: 89 85 88 00 00 00  mov         dword ptr [rbp+88h],eax
  0000000000103808: 83 BD A8 00 00 00  cmp         dword ptr [rbp+0A8h],0
                    00
  000000000010380F: 74 0A              je          000000000010381B
  0000000000103811: E9 C4 06 00 00     jmp         0000000000103EDA
  0000000000103816: E9 BF 06 00 00     jmp         0000000000103EDA
  000000000010381B: E9 C9 02 00 00     jmp         0000000000103AE9
  0000000000103820: E9 C4 02 00 00     jmp         0000000000103AE9
  0000000000103825: 33 C0              xor         eax,eax
  0000000000103827: 85 C0              test        eax,eax
  0000000000103829: 74 29              je          0000000000103854
  000000000010382B: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103832: 48 83 C0 08        add         rax,8
  0000000000103836: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010383D: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103844: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103848: 48 8B 00           mov         rax,qword ptr [rax]
  000000000010384B: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103852: EB 24              jmp         0000000000103878
  0000000000103854: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010385B: 48 83 C0 08        add         rax,8
  000000000010385F: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103866: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010386D: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103871: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103878: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  000000000010387F: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  0000000000103883: 48 83 7D 58 00     cmp         qword ptr [rbp+58h],0
  0000000000103888: 75 0B              jne         0000000000103895
  000000000010388A: 48 8D 05 13 31 00  lea         rax,[??_C@_06OJHGLDPL@?$CInull?$CJ@]
                    00
  0000000000103891: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  0000000000103895: 83 BD AC 00 00 00  cmp         dword ptr [rbp+0ACh],0
                    00
  000000000010389C: 75 0E              jne         00000000001038AC
  000000000010389E: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  00000000001038A2: E8 C9 11 00 00     call        ?strlen@String@@SAIPEBD@Z
  00000000001038A7: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  00000000001038AA: EB 30              jmp         00000000001038DC
  00000000001038AC: C7 45 7C 00 00 00  mov         dword ptr [rbp+7Ch],0
                    00
  00000000001038B3: EB 08              jmp         00000000001038BD
  00000000001038B5: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  00000000001038B8: FF C0              inc         eax
  00000000001038BA: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  00000000001038BD: 8B 85 C8 00 00 00  mov         eax,dword ptr [rbp+0C8h]
  00000000001038C3: 39 45 7C           cmp         dword ptr [rbp+7Ch],eax
  00000000001038C6: 7D 14              jge         00000000001038DC
  00000000001038C8: 48 63 45 7C        movsxd      rax,dword ptr [rbp+7Ch]
  00000000001038CC: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  00000000001038D0: 0F BE 04 01        movsx       eax,byte ptr [rcx+rax]
  00000000001038D4: 85 C0              test        eax,eax
  00000000001038D6: 74 04              je          00000000001038DC
  00000000001038D8: EB DB              jmp         00000000001038B5
  00000000001038DA: EB D9              jmp         00000000001038B5
  00000000001038DC: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  00000000001038DF: 8B 8D 98 00 00 00  mov         ecx,dword ptr [rbp+98h]
  00000000001038E5: 2B C8              sub         ecx,eax
  00000000001038E7: 8B C1              mov         eax,ecx
  00000000001038E9: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  00000000001038EF: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  00000000001038F6: 0F 85 80 00 00 00  jne         000000000010397C
  00000000001038FC: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000103903: 7E 77              jle         000000000010397C
  0000000000103905: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  000000000010390B: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103911: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000103917: FF C8              dec         eax
  0000000000103919: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  000000000010391F: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  0000000000103926: 74 54              je          000000000010397C
  0000000000103928: 0F BE 85 D0 00 00  movsx       eax,byte ptr [rbp+0D0h]
                    00
  000000000010392F: 89 85 FC 00 00 00  mov         dword ptr [rbp+0FCh],eax
  0000000000103935: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  000000000010393D: 74 15              je          0000000000103954
  000000000010393F: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103946: 8B 8D FC 00 00 00  mov         ecx,dword ptr [rbp+0FCh]
  000000000010394C: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000103952: EB 18              jmp         000000000010396C
  0000000000103954: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103958: 0F B6 8D FC 00 00  movzx       ecx,byte ptr [rbp+0FCh]
                    00
  000000000010395F: 88 08              mov         byte ptr [rax],cl
  0000000000103961: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103965: 48 FF C0           inc         rax
  0000000000103968: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010396C: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000103972: FF C0              inc         eax
  0000000000103974: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  000000000010397A: EB 89              jmp         0000000000103905
  000000000010397C: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  000000000010397F: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103985: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000103988: FF C8              dec         eax
  000000000010398A: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  000000000010398D: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  0000000000103994: 74 5F              je          00000000001039F5
  0000000000103996: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  000000000010399A: 0F BE 00           movsx       eax,byte ptr [rax]
  000000000010399D: 89 85 00 01 00 00  mov         dword ptr [rbp+100h],eax
  00000000001039A3: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  00000000001039A7: 48 FF C0           inc         rax
  00000000001039AA: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  00000000001039AE: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001039B6: 74 15              je          00000000001039CD
  00000000001039B8: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001039BF: 8B 8D 00 01 00 00  mov         ecx,dword ptr [rbp+100h]
  00000000001039C5: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001039CB: EB 18              jmp         00000000001039E5
  00000000001039CD: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001039D1: 0F B6 8D 00 01 00  movzx       ecx,byte ptr [rbp+100h]
                    00
  00000000001039D8: 88 08              mov         byte ptr [rax],cl
  00000000001039DA: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001039DE: 48 FF C0           inc         rax
  00000000001039E1: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001039E5: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001039EB: FF C0              inc         eax
  00000000001039ED: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001039F3: EB 87              jmp         000000000010397C
  00000000001039F5: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  00000000001039FC: 0F 84 80 00 00 00  je          0000000000103A82
  0000000000103A02: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],0
                    00
  0000000000103A09: 7E 77              jle         0000000000103A82
  0000000000103A0B: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000103A11: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103A17: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000103A1D: FF C8              dec         eax
  0000000000103A1F: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000103A25: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  0000000000103A2C: 74 54              je          0000000000103A82
  0000000000103A2E: 0F BE 85 D0 00 00  movsx       eax,byte ptr [rbp+0D0h]
                    00
  0000000000103A35: 89 85 04 01 00 00  mov         dword ptr [rbp+104h],eax
  0000000000103A3B: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000103A43: 74 15              je          0000000000103A5A
  0000000000103A45: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000103A4C: 8B 8D 04 01 00 00  mov         ecx,dword ptr [rbp+104h]
  0000000000103A52: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000103A58: EB 18              jmp         0000000000103A72
  0000000000103A5A: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103A5E: 0F B6 8D 04 01 00  movzx       ecx,byte ptr [rbp+104h]
                    00
  0000000000103A65: 88 08              mov         byte ptr [rax],cl
  0000000000103A67: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000103A6B: 48 FF C0           inc         rax
  0000000000103A6E: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000103A72: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000103A78: FF C0              inc         eax
  0000000000103A7A: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000103A80: EB 89              jmp         0000000000103A0B
  0000000000103A82: E9 95 0E 00 00     jmp         000000000010491C
  0000000000103A87: C7 85 BC 00 00 00  mov         dword ptr [rbp+0BCh],1
                    01 00 00 00
  0000000000103A91: E9 47 F2 FF FF     jmp         0000000000102CDD
  0000000000103A96: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],0Ah
                    0A 00 00 00
  0000000000103AA0: EB 47              jmp         0000000000103AE9
  0000000000103AA2: EB 45              jmp         0000000000103AE9
  0000000000103AA4: C7 85 CC 00 00 00  mov         dword ptr [rbp+0CCh],1
                    01 00 00 00
  0000000000103AAE: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],10h
                    10 00 00 00
  0000000000103AB8: EB 2F              jmp         0000000000103AE9
  0000000000103ABA: EB 2D              jmp         0000000000103AE9
  0000000000103ABC: C7 85 88 00 00 00  mov         dword ptr [rbp+88h],10h
                    10 00 00 00
  0000000000103AC6: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],1
                    01 00 00 00
  0000000000103AD0: E9 05 04 00 00     jmp         0000000000103EDA
  0000000000103AD5: E9 00 04 00 00     jmp         0000000000103EDA
  0000000000103ADA: C7 85 C0 00 00 00  mov         dword ptr [rbp+0C0h],1
                    01 00 00 00
  0000000000103AE4: E9 F4 F1 FF FF     jmp         0000000000102CDD
  0000000000103AE9: C7 85 A8 00 00 00  mov         dword ptr [rbp+0A8h],0
                    00 00 00 00
  0000000000103AF3: 83 BD B8 00 00 00  cmp         dword ptr [rbp+0B8h],0
                    00
  0000000000103AFA: 74 66              je          0000000000103B62
  0000000000103AFC: 33 C0              xor         eax,eax
  0000000000103AFE: 85 C0              test        eax,eax
  0000000000103B00: 74 29              je          0000000000103B2B
  0000000000103B02: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B09: 48 83 C0 08        add         rax,8
  0000000000103B0D: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103B14: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B1B: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103B1F: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103B22: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103B29: EB 24              jmp         0000000000103B4F
  0000000000103B2B: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B32: 48 83 C0 08        add         rax,8
  0000000000103B36: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103B3D: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B44: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103B48: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103B4F: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103B56: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103B5D: E9 EF 02 00 00     jmp         0000000000103E51
  0000000000103B62: 83 BD 90 00 00 00  cmp         dword ptr [rbp+90h],0
                    00
  0000000000103B69: 75 09              jne         0000000000103B74
  0000000000103B6B: 83 BD 98 00 00 00  cmp         dword ptr [rbp+98h],10h
                    10
  0000000000103B72: 75 66              jne         0000000000103BDA
  0000000000103B74: 33 C0              xor         eax,eax
  0000000000103B76: 85 C0              test        eax,eax
  0000000000103B78: 74 29              je          0000000000103BA3
  0000000000103B7A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B81: 48 83 C0 08        add         rax,8
  0000000000103B85: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103B8C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103B93: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103B97: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103B9A: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103BA1: EB 24              jmp         0000000000103BC7
  0000000000103BA3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103BAA: 48 83 C0 08        add         rax,8
  0000000000103BAE: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103BB5: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103BBC: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103BC0: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103BC7: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103BCE: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103BD5: E9 77 02 00 00     jmp         0000000000103E51
  0000000000103BDA: 83 BD BC 00 00 00  cmp         dword ptr [rbp+0BCh],0
                    00
  0000000000103BE1: 74 66              je          0000000000103C49
  0000000000103BE3: 33 C0              xor         eax,eax
  0000000000103BE5: 85 C0              test        eax,eax
  0000000000103BE7: 74 29              je          0000000000103C12
  0000000000103BE9: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103BF0: 48 83 C0 08        add         rax,8
  0000000000103BF4: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103BFB: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C02: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103C06: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103C09: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103C10: EB 24              jmp         0000000000103C36
  0000000000103C12: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C19: 48 83 C0 08        add         rax,8
  0000000000103C1D: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103C24: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C2B: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103C2F: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103C36: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103C3D: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103C44: E9 08 02 00 00     jmp         0000000000103E51
  0000000000103C49: 83 BD 8C 00 00 00  cmp         dword ptr [rbp+8Ch],0
                    00
  0000000000103C50: 74 61              je          0000000000103CB3
  0000000000103C52: 33 C0              xor         eax,eax
  0000000000103C54: 85 C0              test        eax,eax
  0000000000103C56: 74 27              je          0000000000103C7F
  0000000000103C58: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C5F: 48 83 C0 08        add         rax,8
  0000000000103C63: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103C6A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C71: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103C75: 8B 00              mov         eax,dword ptr [rax]
  0000000000103C77: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103C7D: EB 22              jmp         0000000000103CA1
  0000000000103C7F: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C86: 48 83 C0 08        add         rax,8
  0000000000103C8A: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103C91: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103C98: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000103C9B: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103CA1: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000103CA7: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103CAE: E9 9E 01 00 00     jmp         0000000000103E51
  0000000000103CB3: 83 BD C0 00 00 00  cmp         dword ptr [rbp+0C0h],0
                    00
  0000000000103CBA: 74 66              je          0000000000103D22
  0000000000103CBC: 33 C0              xor         eax,eax
  0000000000103CBE: 85 C0              test        eax,eax
  0000000000103CC0: 74 29              je          0000000000103CEB
  0000000000103CC2: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103CC9: 48 83 C0 08        add         rax,8
  0000000000103CCD: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103CD4: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103CDB: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103CDF: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103CE2: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103CE9: EB 24              jmp         0000000000103D0F
  0000000000103CEB: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103CF2: 48 83 C0 08        add         rax,8
  0000000000103CF6: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103CFD: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103D04: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103D08: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103D0F: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103D16: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103D1D: E9 2F 01 00 00     jmp         0000000000103E51
  0000000000103D22: 83 BD B4 00 00 00  cmp         dword ptr [rbp+0B4h],0
                    00
  0000000000103D29: 74 62              je          0000000000103D8D
  0000000000103D2B: 33 C0              xor         eax,eax
  0000000000103D2D: 85 C0              test        eax,eax
  0000000000103D2F: 74 27              je          0000000000103D58
  0000000000103D31: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103D38: 48 83 C0 08        add         rax,8
  0000000000103D3C: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103D43: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103D4A: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103D4E: 8B 00              mov         eax,dword ptr [rax]
  0000000000103D50: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103D56: EB 22              jmp         0000000000103D7A
  0000000000103D58: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103D5F: 48 83 C0 08        add         rax,8
  0000000000103D63: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103D6A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103D71: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000103D74: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103D7A: 0F B7 85 78 01 00  movzx       eax,word ptr [rbp+178h]
                    00
  0000000000103D81: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103D88: E9 C4 00 00 00     jmp         0000000000103E51
  0000000000103D8D: 83 BD B0 00 00 00  cmp         dword ptr [rbp+0B0h],0
                    00
  0000000000103D94: 74 5F              je          0000000000103DF5
  0000000000103D96: 33 C0              xor         eax,eax
  0000000000103D98: 85 C0              test        eax,eax
  0000000000103D9A: 74 27              je          0000000000103DC3
  0000000000103D9C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103DA3: 48 83 C0 08        add         rax,8
  0000000000103DA7: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103DAE: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103DB5: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103DB9: 8B 00              mov         eax,dword ptr [rax]
  0000000000103DBB: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103DC1: EB 22              jmp         0000000000103DE5
  0000000000103DC3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103DCA: 48 83 C0 08        add         rax,8
  0000000000103DCE: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103DD5: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103DDC: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000103DDF: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103DE5: 0F B6 85 78 01 00  movzx       eax,byte ptr [rbp+178h]
                    00
  0000000000103DEC: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103DF3: EB 5C              jmp         0000000000103E51
  0000000000103DF5: 33 C0              xor         eax,eax
  0000000000103DF7: 85 C0              test        eax,eax
  0000000000103DF9: 74 27              je          0000000000103E22
  0000000000103DFB: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E02: 48 83 C0 08        add         rax,8
  0000000000103E06: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103E0D: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E14: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103E18: 8B 00              mov         eax,dword ptr [rax]
  0000000000103E1A: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103E20: EB 22              jmp         0000000000103E44
  0000000000103E22: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E29: 48 83 C0 08        add         rax,8
  0000000000103E2D: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103E34: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E3B: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000103E3E: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000103E44: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  0000000000103E4A: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103E51: 83 BD C4 00 00 00  cmp         dword ptr [rbp+0C4h],0
                    00
  0000000000103E58: 74 76              je          0000000000103ED0
  0000000000103E5A: 33 C0              xor         eax,eax
  0000000000103E5C: 85 C0              test        eax,eax
  0000000000103E5E: 74 29              je          0000000000103E89
  0000000000103E60: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E67: 48 83 C0 08        add         rax,8
  0000000000103E6B: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103E72: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E79: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103E7D: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103E80: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103E87: EB 24              jmp         0000000000103EAD
  0000000000103E89: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103E90: 48 83 C0 08        add         rax,8
  0000000000103E94: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103E9B: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103EA2: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103EA6: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103EAD: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103EB4: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000103EB8: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000103EBC: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000103EBF: 89 85 88 00 00 00  mov         dword ptr [rbp+88h],eax
  0000000000103EC5: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000103EC9: 48 FF C0           inc         rax
  0000000000103ECC: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000103ED0: E9 5E 03 00 00     jmp         0000000000104233
  0000000000103ED5: E9 59 03 00 00     jmp         0000000000104233
  0000000000103EDA: 83 BD B8 00 00 00  cmp         dword ptr [rbp+0B8h],0
                    00
  0000000000103EE1: 74 66              je          0000000000103F49
  0000000000103EE3: 33 C0              xor         eax,eax
  0000000000103EE5: 85 C0              test        eax,eax
  0000000000103EE7: 74 29              je          0000000000103F12
  0000000000103EE9: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103EF0: 48 83 C0 08        add         rax,8
  0000000000103EF4: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103EFB: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F02: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103F06: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103F09: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103F10: EB 24              jmp         0000000000103F36
  0000000000103F12: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F19: 48 83 C0 08        add         rax,8
  0000000000103F1D: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103F24: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F2B: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103F2F: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103F36: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103F3D: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103F44: E9 EA 02 00 00     jmp         0000000000104233
  0000000000103F49: 83 BD 90 00 00 00  cmp         dword ptr [rbp+90h],0
                    00
  0000000000103F50: 74 66              je          0000000000103FB8
  0000000000103F52: 33 C0              xor         eax,eax
  0000000000103F54: 85 C0              test        eax,eax
  0000000000103F56: 74 29              je          0000000000103F81
  0000000000103F58: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F5F: 48 83 C0 08        add         rax,8
  0000000000103F63: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103F6A: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F71: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103F75: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103F78: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103F7F: EB 24              jmp         0000000000103FA5
  0000000000103F81: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F88: 48 83 C0 08        add         rax,8
  0000000000103F8C: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103F93: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103F9A: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103F9E: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103FA5: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  0000000000103FAC: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000103FB3: E9 7B 02 00 00     jmp         0000000000104233
  0000000000103FB8: 83 BD BC 00 00 00  cmp         dword ptr [rbp+0BCh],0
                    00
  0000000000103FBF: 74 66              je          0000000000104027
  0000000000103FC1: 33 C0              xor         eax,eax
  0000000000103FC3: 85 C0              test        eax,eax
  0000000000103FC5: 74 29              je          0000000000103FF0
  0000000000103FC7: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103FCE: 48 83 C0 08        add         rax,8
  0000000000103FD2: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000103FD9: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103FE0: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000103FE4: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000103FE7: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000103FEE: EB 24              jmp         0000000000104014
  0000000000103FF0: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000103FF7: 48 83 C0 08        add         rax,8
  0000000000103FFB: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000104002: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104009: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010400D: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  0000000000104014: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  000000000010401B: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000104022: E9 0C 02 00 00     jmp         0000000000104233
  0000000000104027: 83 BD 8C 00 00 00  cmp         dword ptr [rbp+8Ch],0
                    00
  000000000010402E: 74 62              je          0000000000104092
  0000000000104030: 33 C0              xor         eax,eax
  0000000000104032: 85 C0              test        eax,eax
  0000000000104034: 74 27              je          000000000010405D
  0000000000104036: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010403D: 48 83 C0 08        add         rax,8
  0000000000104041: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000104048: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010404F: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000104053: 8B 00              mov         eax,dword ptr [rax]
  0000000000104055: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  000000000010405B: EB 22              jmp         000000000010407F
  000000000010405D: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104064: 48 83 C0 08        add         rax,8
  0000000000104068: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010406F: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104076: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000104079: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  000000000010407F: 48 63 85 78 01 00  movsxd      rax,dword ptr [rbp+178h]
                    00
  0000000000104086: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  000000000010408D: E9 A1 01 00 00     jmp         0000000000104233
  0000000000104092: 83 BD C0 00 00 00  cmp         dword ptr [rbp+0C0h],0
                    00
  0000000000104099: 74 66              je          0000000000104101
  000000000010409B: 33 C0              xor         eax,eax
  000000000010409D: 85 C0              test        eax,eax
  000000000010409F: 74 29              je          00000000001040CA
  00000000001040A1: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001040A8: 48 83 C0 08        add         rax,8
  00000000001040AC: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001040B3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001040BA: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001040BE: 48 8B 00           mov         rax,qword ptr [rax]
  00000000001040C1: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001040C8: EB 24              jmp         00000000001040EE
  00000000001040CA: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001040D1: 48 83 C0 08        add         rax,8
  00000000001040D5: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001040DC: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001040E3: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001040E7: 48 89 85 78 01 00  mov         qword ptr [rbp+178h],rax
                    00
  00000000001040EE: 48 8B 85 78 01 00  mov         rax,qword ptr [rbp+178h]
                    00
  00000000001040F5: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  00000000001040FC: E9 32 01 00 00     jmp         0000000000104233
  0000000000104101: 83 BD B4 00 00 00  cmp         dword ptr [rbp+0B4h],0
                    00
  0000000000104108: 74 63              je          000000000010416D
  000000000010410A: 33 C0              xor         eax,eax
  000000000010410C: 85 C0              test        eax,eax
  000000000010410E: 74 27              je          0000000000104137
  0000000000104110: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104117: 48 83 C0 08        add         rax,8
  000000000010411B: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000104122: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104129: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  000000000010412D: 8B 00              mov         eax,dword ptr [rax]
  000000000010412F: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000104135: EB 22              jmp         0000000000104159
  0000000000104137: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010413E: 48 83 C0 08        add         rax,8
  0000000000104142: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000104149: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104150: 8B 40 F8           mov         eax,dword ptr [rax-8]
  0000000000104153: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000104159: 48 0F BF 85 78 01  movsx       rax,word ptr [rbp+178h]
                    00 00
  0000000000104161: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000104168: E9 C6 00 00 00     jmp         0000000000104233
  000000000010416D: 83 BD B0 00 00 00  cmp         dword ptr [rbp+0B0h],0
                    00
  0000000000104174: 74 60              je          00000000001041D6
  0000000000104176: 33 C0              xor         eax,eax
  0000000000104178: 85 C0              test        eax,eax
  000000000010417A: 74 27              je          00000000001041A3
  000000000010417C: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104183: 48 83 C0 08        add         rax,8
  0000000000104187: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  000000000010418E: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  0000000000104195: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  0000000000104199: 8B 00              mov         eax,dword ptr [rax]
  000000000010419B: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  00000000001041A1: EB 22              jmp         00000000001041C5
  00000000001041A3: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001041AA: 48 83 C0 08        add         rax,8
  00000000001041AE: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001041B5: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001041BC: 8B 40 F8           mov         eax,dword ptr [rax-8]
  00000000001041BF: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  00000000001041C5: 48 0F BE 85 78 01  movsx       rax,byte ptr [rbp+178h]
                    00 00
  00000000001041CD: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  00000000001041D4: EB 5D              jmp         0000000000104233
  00000000001041D6: 33 C0              xor         eax,eax
  00000000001041D8: 85 C0              test        eax,eax
  00000000001041DA: 74 27              je          0000000000104203
  00000000001041DC: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001041E3: 48 83 C0 08        add         rax,8
  00000000001041E7: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  00000000001041EE: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  00000000001041F5: 48 8B 40 F8        mov         rax,qword ptr [rax-8]
  00000000001041F9: 8B 00              mov         eax,dword ptr [rax]
  00000000001041FB: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000104201: EB 22              jmp         0000000000104225
  0000000000104203: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010420A: 48 83 C0 08        add         rax,8
  000000000010420E: 48 89 85 B0 01 00  mov         qword ptr [rbp+1B0h],rax
                    00
  0000000000104215: 48 8B 85 B0 01 00  mov         rax,qword ptr [rbp+1B0h]
                    00
  000000000010421C: 8B 40 F8           mov         eax,dword ptr [rax-8]
  000000000010421F: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000104225: 48 63 85 78 01 00  movsxd      rax,dword ptr [rbp+178h]
                    00
  000000000010422C: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000104233: 83 BD A8 00 00 00  cmp         dword ptr [rbp+0A8h],0
                    00
  000000000010423A: 74 25              je          0000000000104261
  000000000010423C: 48 83 BD 80 00 00  cmp         qword ptr [rbp+80h],0
                    00 00
  0000000000104244: 7D 1B              jge         0000000000104261
  0000000000104246: C7 85 A4 00 00 00  mov         dword ptr [rbp+0A4h],1
                    01 00 00 00
  0000000000104250: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  0000000000104257: 48 F7 D8           neg         rax
  000000000010425A: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000104261: 8B 85 CC 00 00 00  mov         eax,dword ptr [rbp+0CCh]
  0000000000104267: 89 44 24 20        mov         dword ptr [rsp+20h],eax
  000000000010426B: 4C 8D 4D 7C        lea         r9,[rbp+7Ch]
  000000000010426F: 44 8B 85 88 00 00  mov         r8d,dword ptr [rbp+88h]
                    00
  0000000000104276: 48 8B 95 80 00 00  mov         rdx,qword ptr [rbp+80h]
                    00
  000000000010427D: 48 8D 4D 00        lea         rcx,[rbp]
  0000000000104281: E8 6A E7 FF FF     call        ?ksprintn@String@@SAPEADPEAD_KHPEAHH@Z
  0000000000104286: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  000000000010428A: C7 85 94 00 00 00  mov         dword ptr [rbp+94h],0
                    00 00 00 00
  0000000000104294: 83 BD A0 00 00 00  cmp         dword ptr [rbp+0A0h],0
                    00
  000000000010429B: 74 3B              je          00000000001042D8
  000000000010429D: 48 83 BD 80 00 00  cmp         qword ptr [rbp+80h],0
                    00 00
  00000000001042A5: 74 31              je          00000000001042D8
  00000000001042A7: 83 BD 88 00 00 00  cmp         dword ptr [rbp+88h],8
                    08
  00000000001042AE: 75 10              jne         00000000001042C0
  00000000001042B0: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  00000000001042B6: FF C0              inc         eax
  00000000001042B8: 89 85 94 00 00 00  mov         dword ptr [rbp+94h],eax
  00000000001042BE: EB 18              jmp         00000000001042D8
  00000000001042C0: 83 BD 88 00 00 00  cmp         dword ptr [rbp+88h],10h
                    10
  00000000001042C7: 75 0F              jne         00000000001042D8
  00000000001042C9: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  00000000001042CF: 83 C0 02           add         eax,2
  00000000001042D2: 89 85 94 00 00 00  mov         dword ptr [rbp+94h],eax
  00000000001042D8: 83 BD A4 00 00 00  cmp         dword ptr [rbp+0A4h],0
                    00
  00000000001042DF: 74 0E              je          00000000001042EF
  00000000001042E1: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  00000000001042E7: FF C0              inc         eax
  00000000001042E9: 89 85 94 00 00 00  mov         dword ptr [rbp+94h],eax
  00000000001042EF: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  00000000001042F6: 75 22              jne         000000000010431A
  00000000001042F8: 0F BE 85 D0 00 00  movsx       eax,byte ptr [rbp+0D0h]
                    00
  00000000001042FF: 83 F8 30           cmp         eax,30h
  0000000000104302: 75 16              jne         000000000010431A
  0000000000104304: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  000000000010430A: 8B 8D 98 00 00 00  mov         ecx,dword ptr [rbp+98h]
  0000000000104310: 2B C8              sub         ecx,eax
  0000000000104312: 8B C1              mov         eax,ecx
  0000000000104314: 89 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],eax
  000000000010431A: 8B 55 7C           mov         edx,dword ptr [rbp+7Ch]
  000000000010431D: 8B 8D C8 00 00 00  mov         ecx,dword ptr [rbp+0C8h]
  0000000000104323: E8 98 E6 FF FF     call        ?imax@String@@CAHHH@Z
  0000000000104328: 8B 8D 94 00 00 00  mov         ecx,dword ptr [rbp+94h]
  000000000010432E: 03 C8              add         ecx,eax
  0000000000104330: 8B C1              mov         eax,ecx
  0000000000104332: 8B 8D 98 00 00 00  mov         ecx,dword ptr [rbp+98h]
  0000000000104338: 2B C8              sub         ecx,eax
  000000000010433A: 8B C1              mov         eax,ecx
  000000000010433C: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000104342: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000104345: 8B 8D C8 00 00 00  mov         ecx,dword ptr [rbp+0C8h]
  000000000010434B: 2B C8              sub         ecx,eax
  000000000010434D: 8B C1              mov         eax,ecx
  000000000010434F: 89 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],eax
  0000000000104355: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  000000000010435C: 75 74              jne         00000000001043D2
  000000000010435E: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000104364: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  000000000010436A: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000104370: FF C8              dec         eax
  0000000000104372: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000104378: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  000000000010437F: 7E 51              jle         00000000001043D2
  0000000000104381: C7 85 08 01 00 00  mov         dword ptr [rbp+108h],20h
                    20 00 00 00
  000000000010438B: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104393: 74 15              je          00000000001043AA
  0000000000104395: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  000000000010439C: 8B 8D 08 01 00 00  mov         ecx,dword ptr [rbp+108h]
  00000000001043A2: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001043A8: EB 18              jmp         00000000001043C2
  00000000001043AA: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001043AE: 0F B6 8D 08 01 00  movzx       ecx,byte ptr [rbp+108h]
                    00
  00000000001043B5: 88 08              mov         byte ptr [rax],cl
  00000000001043B7: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001043BB: 48 FF C0           inc         rax
  00000000001043BE: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001043C2: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001043C8: FF C0              inc         eax
  00000000001043CA: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001043D0: EB 8C              jmp         000000000010435E
  00000000001043D2: 83 BD A4 00 00 00  cmp         dword ptr [rbp+0A4h],0
                    00
  00000000001043D9: 74 4F              je          000000000010442A
  00000000001043DB: C7 85 0C 01 00 00  mov         dword ptr [rbp+10Ch],2Dh
                    2D 00 00 00
  00000000001043E5: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001043ED: 74 15              je          0000000000104404
  00000000001043EF: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001043F6: 8B 8D 0C 01 00 00  mov         ecx,dword ptr [rbp+10Ch]
  00000000001043FC: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000104402: EB 18              jmp         000000000010441C
  0000000000104404: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104408: 0F B6 8D 0C 01 00  movzx       ecx,byte ptr [rbp+10Ch]
                    00
  000000000010440F: 88 08              mov         byte ptr [rax],cl
  0000000000104411: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104415: 48 FF C0           inc         rax
  0000000000104418: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010441C: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104422: FF C0              inc         eax
  0000000000104424: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  000000000010442A: 83 BD A0 00 00 00  cmp         dword ptr [rbp+0A0h],0
                    00
  0000000000104431: 0F 84 16 01 00 00  je          000000000010454D
  0000000000104437: 48 83 BD 80 00 00  cmp         qword ptr [rbp+80h],0
                    00 00
  000000000010443F: 0F 84 08 01 00 00  je          000000000010454D
  0000000000104445: 83 BD 88 00 00 00  cmp         dword ptr [rbp+88h],8
                    08
  000000000010444C: 75 54              jne         00000000001044A2
  000000000010444E: C7 85 10 01 00 00  mov         dword ptr [rbp+110h],30h
                    30 00 00 00
  0000000000104458: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104460: 74 15              je          0000000000104477
  0000000000104462: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000104469: 8B 8D 10 01 00 00  mov         ecx,dword ptr [rbp+110h]
  000000000010446F: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000104475: EB 18              jmp         000000000010448F
  0000000000104477: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010447B: 0F B6 8D 10 01 00  movzx       ecx,byte ptr [rbp+110h]
                    00
  0000000000104482: 88 08              mov         byte ptr [rax],cl
  0000000000104484: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104488: 48 FF C0           inc         rax
  000000000010448B: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010448F: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104495: FF C0              inc         eax
  0000000000104497: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  000000000010449D: E9 AB 00 00 00     jmp         000000000010454D
  00000000001044A2: 83 BD 88 00 00 00  cmp         dword ptr [rbp+88h],10h
                    10
  00000000001044A9: 0F 85 9E 00 00 00  jne         000000000010454D
  00000000001044AF: C7 85 14 01 00 00  mov         dword ptr [rbp+114h],30h
                    30 00 00 00
  00000000001044B9: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001044C1: 74 15              je          00000000001044D8
  00000000001044C3: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001044CA: 8B 8D 14 01 00 00  mov         ecx,dword ptr [rbp+114h]
  00000000001044D0: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001044D6: EB 18              jmp         00000000001044F0
  00000000001044D8: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001044DC: 0F B6 8D 14 01 00  movzx       ecx,byte ptr [rbp+114h]
                    00
  00000000001044E3: 88 08              mov         byte ptr [rax],cl
  00000000001044E5: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001044E9: 48 FF C0           inc         rax
  00000000001044EC: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001044F0: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001044F6: FF C0              inc         eax
  00000000001044F8: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001044FE: C7 85 18 01 00 00  mov         dword ptr [rbp+118h],78h
                    78 00 00 00
  0000000000104508: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104510: 74 15              je          0000000000104527
  0000000000104512: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000104519: 8B 8D 18 01 00 00  mov         ecx,dword ptr [rbp+118h]
  000000000010451F: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000104525: EB 18              jmp         000000000010453F
  0000000000104527: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010452B: 0F B6 8D 18 01 00  movzx       ecx,byte ptr [rbp+118h]
                    00
  0000000000104532: 88 08              mov         byte ptr [rax],cl
  0000000000104534: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104538: 48 FF C0           inc         rax
  000000000010453B: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010453F: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104545: FF C0              inc         eax
  0000000000104547: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  000000000010454D: 8B 85 C8 00 00 00  mov         eax,dword ptr [rbp+0C8h]
  0000000000104553: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  0000000000104559: 8B 85 C8 00 00 00  mov         eax,dword ptr [rbp+0C8h]
  000000000010455F: FF C8              dec         eax
  0000000000104561: 89 85 C8 00 00 00  mov         dword ptr [rbp+0C8h],eax
  0000000000104567: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  000000000010456E: 7E 51              jle         00000000001045C1
  0000000000104570: C7 85 1C 01 00 00  mov         dword ptr [rbp+11Ch],30h
                    30 00 00 00
  000000000010457A: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104582: 74 15              je          0000000000104599
  0000000000104584: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  000000000010458B: 8B 8D 1C 01 00 00  mov         ecx,dword ptr [rbp+11Ch]
  0000000000104591: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000104597: EB 18              jmp         00000000001045B1
  0000000000104599: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010459D: 0F B6 8D 1C 01 00  movzx       ecx,byte ptr [rbp+11Ch]
                    00
  00000000001045A4: 88 08              mov         byte ptr [rax],cl
  00000000001045A6: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001045AA: 48 FF C0           inc         rax
  00000000001045AD: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001045B1: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001045B7: FF C0              inc         eax
  00000000001045B9: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001045BF: EB 8C              jmp         000000000010454D
  00000000001045C1: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  00000000001045C5: 0F BE 00           movsx       eax,byte ptr [rax]
  00000000001045C8: 85 C0              test        eax,eax
  00000000001045CA: 74 5F              je          000000000010462B
  00000000001045CC: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  00000000001045D0: 0F BE 00           movsx       eax,byte ptr [rax]
  00000000001045D3: 89 85 20 01 00 00  mov         dword ptr [rbp+120h],eax
  00000000001045D9: 48 8B 45 58        mov         rax,qword ptr [rbp+58h]
  00000000001045DD: 48 FF C8           dec         rax
  00000000001045E0: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  00000000001045E4: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001045EC: 74 15              je          0000000000104603
  00000000001045EE: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001045F5: 8B 8D 20 01 00 00  mov         ecx,dword ptr [rbp+120h]
  00000000001045FB: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  0000000000104601: EB 18              jmp         000000000010461B
  0000000000104603: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104607: 0F B6 8D 20 01 00  movzx       ecx,byte ptr [rbp+120h]
                    00
  000000000010460E: 88 08              mov         byte ptr [rax],cl
  0000000000104610: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104614: 48 FF C0           inc         rax
  0000000000104617: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010461B: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104621: FF C0              inc         eax
  0000000000104623: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000104629: EB 96              jmp         00000000001045C1
  000000000010462B: 83 BD C4 00 00 00  cmp         dword ptr [rbp+0C4h],0
                    00
  0000000000104632: 0F 84 EF 01 00 00  je          0000000000104827
  0000000000104638: 48 83 BD 80 00 00  cmp         qword ptr [rbp+80h],0
                    00 00
  0000000000104640: 0F 84 E1 01 00 00  je          0000000000104827
  0000000000104646: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  000000000010464C: 89 85 94 00 00 00  mov         dword ptr [rbp+94h],eax
  0000000000104652: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000104656: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000104659: 85 C0              test        eax,eax
  000000000010465B: 0F 84 49 01 00 00  je          00000000001047AA
  0000000000104661: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000104665: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000104668: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  000000000010466B: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  000000000010466F: 48 FF C0           inc         rax
  0000000000104672: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000104676: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000104679: FF C8              dec         eax
  000000000010467B: B9 01 00 00 00     mov         ecx,1
  0000000000104680: 89 8D 78 01 00 00  mov         dword ptr [rbp+178h],ecx
  0000000000104686: 0F B6 C8           movzx       ecx,al
  0000000000104689: 8B 85 78 01 00 00  mov         eax,dword ptr [rbp+178h]
  000000000010468F: D3 E0              shl         eax,cl
  0000000000104691: 48 98              cdqe
  0000000000104693: 48 8B 8D 80 00 00  mov         rcx,qword ptr [rbp+80h]
                    00
  000000000010469A: 48 23 C8           and         rcx,rax
  000000000010469D: 48 8B C1           mov         rax,rcx
  00000000001046A0: 48 85 C0           test        rax,rax
  00000000001046A3: 0F 84 DF 00 00 00  je          0000000000104788
  00000000001046A9: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  00000000001046AF: 39 85 D8 00 00 00  cmp         dword ptr [rbp+0D8h],eax
  00000000001046B5: 74 09              je          00000000001046C0
  00000000001046B7: C6 85 78 01 00 00  mov         byte ptr [rbp+178h],2Ch
                    2C
  00000000001046BE: EB 07              jmp         00000000001046C7
  00000000001046C0: C6 85 78 01 00 00  mov         byte ptr [rbp+178h],3Ch
                    3C
  00000000001046C7: 0F BE 85 78 01 00  movsx       eax,byte ptr [rbp+178h]
                    00
  00000000001046CE: 89 85 24 01 00 00  mov         dword ptr [rbp+124h],eax
  00000000001046D4: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001046DC: 74 15              je          00000000001046F3
  00000000001046DE: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001046E5: 8B 8D 24 01 00 00  mov         ecx,dword ptr [rbp+124h]
  00000000001046EB: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001046F1: EB 18              jmp         000000000010470B
  00000000001046F3: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001046F7: 0F B6 8D 24 01 00  movzx       ecx,byte ptr [rbp+124h]
                    00
  00000000001046FE: 88 08              mov         byte ptr [rax],cl
  0000000000104700: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104704: 48 FF C0           inc         rax
  0000000000104707: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  000000000010470B: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104711: FF C0              inc         eax
  0000000000104713: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000104719: EB 0B              jmp         0000000000104726
  000000000010471B: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  000000000010471F: 48 FF C0           inc         rax
  0000000000104722: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000104726: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  000000000010472A: 0F BE 00           movsx       eax,byte ptr [rax]
  000000000010472D: 89 45 7C           mov         dword ptr [rbp+7Ch],eax
  0000000000104730: 83 7D 7C 20        cmp         dword ptr [rbp+7Ch],20h
  0000000000104734: 7E 50              jle         0000000000104786
  0000000000104736: 8B 45 7C           mov         eax,dword ptr [rbp+7Ch]
  0000000000104739: 89 85 28 01 00 00  mov         dword ptr [rbp+128h],eax
  000000000010473F: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104747: 74 15              je          000000000010475E
  0000000000104749: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  0000000000104750: 8B 8D 28 01 00 00  mov         ecx,dword ptr [rbp+128h]
  0000000000104756: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  000000000010475C: EB 18              jmp         0000000000104776
  000000000010475E: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104762: 0F B6 8D 28 01 00  movzx       ecx,byte ptr [rbp+128h]
                    00
  0000000000104769: 88 08              mov         byte ptr [rax],cl
  000000000010476B: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010476F: 48 FF C0           inc         rax
  0000000000104772: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000104776: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  000000000010477C: FF C0              inc         eax
  000000000010477E: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000104784: EB 95              jmp         000000000010471B
  0000000000104786: EB 1D              jmp         00000000001047A5
  0000000000104788: EB 0B              jmp         0000000000104795
  000000000010478A: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  000000000010478E: 48 FF C0           inc         rax
  0000000000104791: 48 89 45 68        mov         qword ptr [rbp+68h],rax
  0000000000104795: 48 8B 45 68        mov         rax,qword ptr [rbp+68h]
  0000000000104799: 0F BE 00           movsx       eax,byte ptr [rax]
  000000000010479C: 83 F8 20           cmp         eax,20h
  000000000010479F: 7E 04              jle         00000000001047A5
  00000000001047A1: EB E7              jmp         000000000010478A
  00000000001047A3: EB E5              jmp         000000000010478A
  00000000001047A5: E9 A8 FE FF FF     jmp         0000000000104652
  00000000001047AA: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  00000000001047B0: 39 85 D8 00 00 00  cmp         dword ptr [rbp+0D8h],eax
  00000000001047B6: 74 6F              je          0000000000104827
  00000000001047B8: C7 85 2C 01 00 00  mov         dword ptr [rbp+12Ch],3Eh
                    3E 00 00 00
  00000000001047C2: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001047CA: 74 15              je          00000000001047E1
  00000000001047CC: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001047D3: 8B 8D 2C 01 00 00  mov         ecx,dword ptr [rbp+12Ch]
  00000000001047D9: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001047DF: EB 18              jmp         00000000001047F9
  00000000001047E1: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001047E5: 0F B6 8D 2C 01 00  movzx       ecx,byte ptr [rbp+12Ch]
                    00
  00000000001047EC: 88 08              mov         byte ptr [rax],cl
  00000000001047EE: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001047F2: 48 FF C0           inc         rax
  00000000001047F5: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  00000000001047F9: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  00000000001047FF: FF C0              inc         eax
  0000000000104801: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000104807: 8B 85 94 00 00 00  mov         eax,dword ptr [rbp+94h]
  000000000010480D: 8B 8D D8 00 00 00  mov         ecx,dword ptr [rbp+0D8h]
  0000000000104813: 2B C8              sub         ecx,eax
  0000000000104815: 8B C1              mov         eax,ecx
  0000000000104817: 8B 8D 98 00 00 00  mov         ecx,dword ptr [rbp+98h]
  000000000010481D: 2B C8              sub         ecx,eax
  000000000010481F: 8B C1              mov         eax,ecx
  0000000000104821: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  0000000000104827: 83 BD 9C 00 00 00  cmp         dword ptr [rbp+9Ch],0
                    00
  000000000010482E: 74 74              je          00000000001048A4
  0000000000104830: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000104836: 89 85 78 01 00 00  mov         dword ptr [rbp+178h],eax
  000000000010483C: 8B 85 98 00 00 00  mov         eax,dword ptr [rbp+98h]
  0000000000104842: FF C8              dec         eax
  0000000000104844: 89 85 98 00 00 00  mov         dword ptr [rbp+98h],eax
  000000000010484A: 83 BD 78 01 00 00  cmp         dword ptr [rbp+178h],0
                    00
  0000000000104851: 7E 51              jle         00000000001048A4
  0000000000104853: C7 85 30 01 00 00  mov         dword ptr [rbp+130h],20h
                    20 00 00 00
  000000000010485D: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  0000000000104865: 74 15              je          000000000010487C
  0000000000104867: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  000000000010486E: 8B 8D 30 01 00 00  mov         ecx,dword ptr [rbp+130h]
  0000000000104874: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  000000000010487A: EB 18              jmp         0000000000104894
  000000000010487C: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104880: 0F B6 8D 30 01 00  movzx       ecx,byte ptr [rbp+130h]
                    00
  0000000000104887: 88 08              mov         byte ptr [rax],cl
  0000000000104889: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  000000000010488D: 48 FF C0           inc         rax
  0000000000104890: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000104894: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  000000000010489A: FF C0              inc         eax
  000000000010489C: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  00000000001048A2: EB 8C              jmp         0000000000104830
  00000000001048A4: EB 76              jmp         000000000010491C
  00000000001048A6: 48 8B 85 90 01 00  mov         rax,qword ptr [rbp+190h]
                    00
  00000000001048AD: 48 39 45 60        cmp         qword ptr [rbp+60h],rax
  00000000001048B1: 73 5F              jae         0000000000104912
  00000000001048B3: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001048B7: 0F BE 00           movsx       eax,byte ptr [rax]
  00000000001048BA: 89 85 34 01 00 00  mov         dword ptr [rbp+134h],eax
  00000000001048C0: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  00000000001048C4: 48 FF C0           inc         rax
  00000000001048C7: 48 89 45 60        mov         qword ptr [rbp+60h],rax
  00000000001048CB: 48 83 BD 98 01 00  cmp         qword ptr [rbp+198h],0
                    00 00
  00000000001048D3: 74 15              je          00000000001048EA
  00000000001048D5: 48 8B 95 A0 01 00  mov         rdx,qword ptr [rbp+1A0h]
                    00
  00000000001048DC: 8B 8D 34 01 00 00  mov         ecx,dword ptr [rbp+134h]
  00000000001048E2: FF 95 98 01 00 00  call        qword ptr [rbp+198h]
  00000000001048E8: EB 18              jmp         0000000000104902
  00000000001048EA: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001048EE: 0F B6 8D 34 01 00  movzx       ecx,byte ptr [rbp+134h]
                    00
  00000000001048F5: 88 08              mov         byte ptr [rax],cl
  00000000001048F7: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  00000000001048FB: 48 FF C0           inc         rax
  00000000001048FE: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000104902: 8B 85 D8 00 00 00  mov         eax,dword ptr [rbp+0D8h]
  0000000000104908: FF C0              inc         eax
  000000000010490A: 89 85 D8 00 00 00  mov         dword ptr [rbp+0D8h],eax
  0000000000104910: EB 94              jmp         00000000001048A6
  0000000000104912: C7 85 D4 00 00 00  mov         dword ptr [rbp+0D4h],1
                    01 00 00 00
  000000000010491C: E9 6A E2 FF FF     jmp         0000000000102B8B
  0000000000104921: 48 8D A5 80 01 00  lea         rsp,[rbp+180h]
                    00
  0000000000104928: 5D                 pop         rbp
  0000000000104929: C3                 ret
  000000000010492A: 66 90              xchg        ax,ax
  000000000010492C: 5A                 pop         rdx
  000000000010492D: 2D 00 00 87 2D     sub         eax,2D870000h
  0000000000104932: 00 00              add         byte ptr [rax],al
  0000000000104934: DA 2D 00 00 69 2D  fisubr      dword ptr [2D79493Ah]
  000000000010493A: 00 00              add         byte ptr [rax],al
  000000000010493C: 78 2D              js          000000000010496B
  000000000010493E: 00 00              add         byte ptr [rax],al
  0000000000104940: 4E 2D 00 00 E6 2E  sub         rax,2EE60000h
  0000000000104946: 00 00              add         byte ptr [rax],al
  0000000000104948: FB                 sti
  0000000000104949: 2E 00 00           add         byte ptr cs:[rax],al
  000000000010494C: 4D 31 00           xor         qword ptr [r8],r8
  000000000010494F: 00 A4 3A 00 00 62  add         byte ptr [rdx+rdi+2F620000h],ah
                    2F
  0000000000104956: 00 00              add         byte ptr [rax],al
  0000000000104958: 80 2F 00           sub         byte ptr [rdi],0
  000000000010495B: 00 99 33 00 00 B7  add         byte ptr [rcx-48FFFFCDh],bl
  0000000000104961: 33 00              xor         eax,dword ptr [rax]
  0000000000104963: 00 E5              add         ch,ah
  0000000000104965: 33 00              xor         eax,dword ptr [rax]
  0000000000104967: 00 F4              add         ah,dh
  0000000000104969: 33 00              xor         eax,dword ptr [rax]
  000000000010496B: 00 22              add         byte ptr [rdx],ah
  000000000010496D: 34 00              xor         al,0
  000000000010496F: 00 2F              add         byte ptr [rdi],ch
  0000000000104971: 37
  0000000000104972: 00 00              add         byte ptr [rax],al
  0000000000104974: 43
  0000000000104975: 37
  0000000000104976: 00 00              add         byte ptr [rax],al
  0000000000104978: ED                 in          eax,dx
  0000000000104979: 37
  000000000010497A: 00 00              add         byte ptr [rax],al
  000000000010497C: FC                 cld
  000000000010497D: 37
  000000000010497E: 00 00              add         byte ptr [rax],al
  0000000000104980: 25 38 00 00 87     and         eax,87000038h
  0000000000104985: 3A 00              cmp         al,byte ptr [rax]
  0000000000104987: 00 96 3A 00 00 AE  add         byte ptr [rsi-51FFFFC6h],dl
  000000000010498D: 3A 00              cmp         al,byte ptr [rax]
  000000000010498F: 00 BC 3A 00 00 DA  add         byte ptr [rdx+rdi+3ADA0000h],bh
                    3A
  0000000000104996: 00 00              add         byte ptr [rax],al
  0000000000104998: A6                 cmps        byte ptr [rsi],byte ptr [rdi]
  0000000000104999: 48 00 00           add         byte ptr [rax],al
  000000000010499C: 00 1B              add         byte ptr [rbx],bl
  000000000010499E: 01 1B              add         dword ptr [rbx],ebx
  00000000001049A0: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049A2: 1B 02              sbb         eax,dword ptr [rdx]
  00000000001049A4: 03 1B              add         ebx,dword ptr [rbx]
  00000000001049A6: 04 05              add         al,5
  00000000001049A8: 1B 06              sbb         eax,dword ptr [rsi]
  00000000001049AA: 07
  00000000001049AB: 07
  00000000001049AC: 07
  00000000001049AD: 07
  00000000001049AE: 07
  00000000001049AF: 07
  00000000001049B0: 07
  00000000001049B1: 07
  00000000001049B2: 07
  00000000001049B3: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049B5: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049B7: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049B9: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049BB: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049BD: 08 1B              or          byte ptr [rbx],bl
  00000000001049BF: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049C1: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049C3: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049C5: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049C7: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049C9: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049CB: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049CD: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049CF: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049D1: 09 1B              or          dword ptr [rbx],ebx
  00000000001049D3: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049D5: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049D7: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049D9: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049DB: 0A 0B              or          cl,byte ptr [rbx]
  00000000001049DD: 0C 1B              or          al,1Bh
  00000000001049DF: 1B 1B              sbb         ebx,dword ptr [rbx]
  00000000001049E1: 0D 0C 0E 1B 0F     or          eax,0F1B0E0Ch
  00000000001049E6: 1B 10              sbb         edx,dword ptr [rax]
  00000000001049E8: 11 12              adc         dword ptr [rdx],edx
  00000000001049EA: 13 14 15 16 17 1B  adc         edx,dword ptr [rdx+1B1B1716h]
                    1B
  00000000001049F1: 18 19              sbb         byte ptr [rcx],bl
  00000000001049F3: 1A
  00000000001049F4: CC CC CC CC CC CC CC CC CC CC CC CC              лллллллллллл
?sprintf@String@@SAHPEADPEBDZZ:
  0000000000104A00: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000104A05: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104A0A: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000104A0F: 4C 89 4C 24 20     mov         qword ptr [rsp+20h],r9
  0000000000104A14: 55                 push        rbp
  0000000000104A15: 48 81 EC 90 00 00  sub         rsp,90h
                    00
  0000000000104A1C: 48 8D 6C 24 30     lea         rbp,[rsp+30h]
  0000000000104A21: 48 8D 85 80 00 00  lea         rax,[rbp+80h]
                    00
  0000000000104A28: 48 89 45 08        mov         qword ptr [rbp+8],rax
  0000000000104A2C: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000104A30: 48 89 44 24 20     mov         qword ptr [rsp+20h],rax
  0000000000104A35: 41 B9 0A 00 00 00  mov         r9d,0Ah
  0000000000104A3B: 4C 8B 45 70        mov         r8,qword ptr [rbp+70h]
  0000000000104A3F: 33 D2              xor         edx,edx
  0000000000104A41: 48 8B 4D 78        mov         rcx,qword ptr [rbp+78h]
  0000000000104A45: E8 A6 E0 FF FF     call        ?kvprintf@String@@SAHPEBDP6AXHPEAX@Z1HPEAD@Z
  0000000000104A4A: 89 45 00           mov         dword ptr [rbp],eax
  0000000000104A4D: 48 63 45 00        movsxd      rax,dword ptr [rbp]
  0000000000104A51: 48 8B 4D 70        mov         rcx,qword ptr [rbp+70h]
  0000000000104A55: C6 04 01 00        mov         byte ptr [rcx+rax],0
  0000000000104A59: 48 C7 45 08 00 00  mov         qword ptr [rbp+8],0
                    00 00
  0000000000104A61: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104A64: 48 8D 65 60        lea         rsp,[rbp+60h]
  0000000000104A68: 5D                 pop         rbp
  0000000000104A69: C3                 ret
  0000000000104A6A: CC CC CC CC CC CC                                лллллл
?strlen@String@@SAIPEBD@Z:
  0000000000104A70: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104A75: 55                 push        rbp
  0000000000104A76: 48 83 EC 70        sub         rsp,70h
  0000000000104A7A: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000104A7F: 48 83 7D 60 00     cmp         qword ptr [rbp+60h],0
  0000000000104A84: 75 0C              jne         0000000000104A92
  0000000000104A86: 48 8D 0D FB 1E 00  lea         rcx,[??_C@_0N@MKGBMKPP@s?5?$CB?$DN?5nullptr@]
                    00
  0000000000104A8D: E8 7E D1 FF FF     call        ?KernelBugcheck@@YAXPEBD@Z
  0000000000104A92: 48 C7 45 00 00 00  mov         qword ptr [rbp],0
                    00 00
  0000000000104A9A: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000104A9E: 0F BE 00           movsx       eax,byte ptr [rax]
  0000000000104AA1: 89 45 48           mov         dword ptr [rbp+48h],eax
  0000000000104AA4: 48 8B 45 60        mov         rax,qword ptr [rbp+60h]
  0000000000104AA8: 48 FF C0           inc         rax
  0000000000104AAB: 48 89 45 60        mov         qword ptr [rbp+60h],rax
  0000000000104AAF: 83 7D 48 00        cmp         dword ptr [rbp+48h],0
  0000000000104AB3: 74 0D              je          0000000000104AC2
  0000000000104AB5: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000104AB9: 48 FF C0           inc         rax
  0000000000104ABC: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000104AC0: EB D8              jmp         0000000000104A9A
  0000000000104AC2: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104AC5: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000104AC9: 5D                 pop         rbp
  0000000000104ACA: C3                 ret
  0000000000104ACB: CC CC CC CC CC                                   ллллл
?vsprintf@String@@SAHPEADPEBD0@Z:
  0000000000104AD0: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000104AD5: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000104ADA: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104ADF: 55                 push        rbp
  0000000000104AE0: 48 81 EC 80 00 00  sub         rsp,80h
                    00
  0000000000104AE7: 48 8D 6C 24 30     lea         rbp,[rsp+30h]
  0000000000104AEC: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000104AF0: 48 89 44 24 20     mov         qword ptr [rsp+20h],rax
  0000000000104AF5: 41 B9 0A 00 00 00  mov         r9d,0Ah
  0000000000104AFB: 4C 8B 45 60        mov         r8,qword ptr [rbp+60h]
  0000000000104AFF: 33 D2              xor         edx,edx
  0000000000104B01: 48 8B 4D 68        mov         rcx,qword ptr [rbp+68h]
  0000000000104B05: E8 E6 DF FF FF     call        ?kvprintf@String@@SAHPEBDP6AXHPEAX@Z1HPEAD@Z
  0000000000104B0A: 89 45 00           mov         dword ptr [rbp],eax
  0000000000104B0D: 48 63 45 00        movsxd      rax,dword ptr [rbp]
  0000000000104B11: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000104B15: C6 04 01 00        mov         byte ptr [rcx+rax],0
  0000000000104B19: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104B1C: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000104B20: 5D                 pop         rbp
  0000000000104B21: C3                 ret
  0000000000104B22: CC CC CC CC CC CC CC CC CC CC CC CC CC CC        лллллллллллллл
??0System@@QEAA@PEAU_EFI_CONFIGURATION_TABLE@@_K@Z:
  0000000000104B30: 4C 89 44 24 18     mov         qword ptr [rsp+18h],r8
  0000000000104B35: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000104B3A: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104B3F: 55                 push        rbp
  0000000000104B40: 48 83 EC 40        sub         rsp,40h
  0000000000104B44: 48 8B EC           mov         rbp,rsp
  0000000000104B47: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104B4B: 48 8B 4D 58        mov         rcx,qword ptr [rbp+58h]
  0000000000104B4F: 48 89 08           mov         qword ptr [rax],rcx
  0000000000104B52: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104B56: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000104B5A: 48 89 48 08        mov         qword ptr [rax+8],rcx
  0000000000104B5E: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000104B62: 48 8D 65 40        lea         rsp,[rbp+40h]
  0000000000104B66: 5D                 pop         rbp
  0000000000104B67: C3                 ret
  0000000000104B68: CC CC CC CC CC CC CC CC                          лллллллл
?GetInstalledSystemRam@System@@QEAA_KXZ:
  0000000000104B70: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104B75: 55                 push        rbp
  0000000000104B76: 53                 push        rbx
  0000000000104B77: 56                 push        rsi
  0000000000104B78: 57                 push        rdi
  0000000000104B79: 48 81 EC F8 00 00  sub         rsp,0F8h
                    00
  0000000000104B80: 48 8D 6C 24 70     lea         rbp,[rsp+70h]
  0000000000104B85: C7 45 00 00 00 00  mov         dword ptr [rbp],0
                    00
  0000000000104B8C: EB 08              jmp         0000000000104B96
  0000000000104B8E: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104B91: FF C0              inc         eax
  0000000000104B93: 89 45 00           mov         dword ptr [rbp],eax
  0000000000104B96: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104B99: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  0000000000104BA0: 48 3B 41 08        cmp         rax,qword ptr [rcx+8]
  0000000000104BA4: 0F 83 B9 02 00 00  jae         0000000000104E63
  0000000000104BAA: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104BAD: 48 6B C0 18        imul        rax,rax,18h
  0000000000104BB1: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  0000000000104BB8: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000104BBB: 48 03 C8           add         rcx,rax
  0000000000104BBE: 48 8B C1           mov         rax,rcx
  0000000000104BC1: 48 89 45 08        mov         qword ptr [rbp+8],rax
  0000000000104BC5: B8 01 00 00 00     mov         eax,1
  0000000000104BCA: 48 6B C0 07        imul        rax,rax,7
  0000000000104BCE: 48 8B 4D 08        mov         rcx,qword ptr [rbp+8]
  0000000000104BD2: 0F B6 44 01 08     movzx       eax,byte ptr [rcx+rax+8]
  0000000000104BD7: B9 01 00 00 00     mov         ecx,1
  0000000000104BDC: 48 6B C9 06        imul        rcx,rcx,6
  0000000000104BE0: 48 8B 55 08        mov         rdx,qword ptr [rbp+8]
  0000000000104BE4: 0F B6 4C 0A 08     movzx       ecx,byte ptr [rdx+rcx+8]
  0000000000104BE9: BA 01 00 00 00     mov         edx,1
  0000000000104BEE: 48 6B D2 05        imul        rdx,rdx,5
  0000000000104BF2: 4C 8B 45 08        mov         r8,qword ptr [rbp+8]
  0000000000104BF6: 41 0F B6 54 10 08  movzx       edx,byte ptr [r8+rdx+8]
  0000000000104BFC: 41 B8 01 00 00 00  mov         r8d,1
  0000000000104C02: 4D 6B C0 04        imul        r8,r8,4
  0000000000104C06: 4C 8B 4D 08        mov         r9,qword ptr [rbp+8]
  0000000000104C0A: 47 0F B6 44 01 08  movzx       r8d,byte ptr [r9+r8+8]
  0000000000104C10: 41 B9 01 00 00 00  mov         r9d,1
  0000000000104C16: 4D 6B C9 03        imul        r9,r9,3
  0000000000104C1A: 4C 8B 55 08        mov         r10,qword ptr [rbp+8]
  0000000000104C1E: 47 0F B6 4C 0A 08  movzx       r9d,byte ptr [r10+r9+8]
  0000000000104C24: 41 BA 01 00 00 00  mov         r10d,1
  0000000000104C2A: 4D 6B D2 02        imul        r10,r10,2
  0000000000104C2E: 4C 8B 5D 08        mov         r11,qword ptr [rbp+8]
  0000000000104C32: 47 0F B6 54 13 08  movzx       r10d,byte ptr [r11+r10+8]
  0000000000104C38: 41 BB 01 00 00 00  mov         r11d,1
  0000000000104C3E: 4D 6B DB 01        imul        r11,r11,1
  0000000000104C42: 48 8B 5D 08        mov         rbx,qword ptr [rbp+8]
  0000000000104C46: 42 0F B6 5C 1B 08  movzx       ebx,byte ptr [rbx+r11+8]
  0000000000104C4C: 41 BB 01 00 00 00  mov         r11d,1
  0000000000104C52: 4D 6B DB 00        imul        r11,r11,0
  0000000000104C56: 48 8B 7D 08        mov         rdi,qword ptr [rbp+8]
  0000000000104C5A: 42 0F B6 7C 1F 08  movzx       edi,byte ptr [rdi+r11+8]
  0000000000104C60: 4C 8B 5D 08        mov         r11,qword ptr [rbp+8]
  0000000000104C64: 45 0F B7 5B 06     movzx       r11d,word ptr [r11+6]
  0000000000104C69: 48 8B 75 08        mov         rsi,qword ptr [rbp+8]
  0000000000104C6D: 0F B7 76 04        movzx       esi,word ptr [rsi+4]
  0000000000104C71: 89 44 24 60        mov         dword ptr [rsp+60h],eax
  0000000000104C75: 89 4C 24 58        mov         dword ptr [rsp+58h],ecx
  0000000000104C79: 89 54 24 50        mov         dword ptr [rsp+50h],edx
  0000000000104C7D: 44 89 44 24 48     mov         dword ptr [rsp+48h],r8d
  0000000000104C82: 44 89 4C 24 40     mov         dword ptr [rsp+40h],r9d
  0000000000104C87: 44 89 54 24 38     mov         dword ptr [rsp+38h],r10d
  0000000000104C8C: 89 5C 24 30        mov         dword ptr [rsp+30h],ebx
  0000000000104C90: 89 7C 24 28        mov         dword ptr [rsp+28h],edi
  0000000000104C94: 44 89 5C 24 20     mov         dword ptr [rsp+20h],r11d
  0000000000104C99: 44 8B CE           mov         r9d,esi
  0000000000104C9C: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000104CA0: 44 8B 00           mov         r8d,dword ptr [rax]
  0000000000104CA3: 48 8D 15 16 1D 00  lea         rdx,[??_C@_0EN@MBKMAGKL@Guid?5?$DN?5?$HL?$CF08lX?9?$CF04hX?9?$CF04hX?9?$CF02hh@]
                    00
  0000000000104CAA: 48 8B 0D CF 23 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104CB1: E8 0A CC FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104CB6: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104CB9: 48 6B C0 18        imul        rax,rax,18h
  0000000000104CBD: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  0000000000104CC4: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000104CC7: 48 03 C8           add         rcx,rax
  0000000000104CCA: 48 8B C1           mov         rax,rcx
  0000000000104CCD: 41 B8 10 00 00 00  mov         r8d,10h
  0000000000104CD3: 48 8D 15 D6 1C 00  lea         rdx,[1069B0h]
                    00
  0000000000104CDA: 48 8B C8           mov         rcx,rax
  0000000000104CDD: E8 CE 04 00 00     call        ?memcmp@CRT@@SAHPEBX0I@Z
  0000000000104CE2: 85 C0              test        eax,eax
  0000000000104CE4: 74 05              je          0000000000104CEB
  0000000000104CE6: E9 A3 FE FF FF     jmp         0000000000104B8E
  0000000000104CEB: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104CEE: 48 6B C0 18        imul        rax,rax,18h
  0000000000104CF2: 48 8B 8D B0 00 00  mov         rcx,qword ptr [rbp+0B0h]
                    00
  0000000000104CF9: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000104CFC: 48 8B 44 01 10     mov         rax,qword ptr [rcx+rax+10h]
  0000000000104D01: 48 89 45 10        mov         qword ptr [rbp+10h],rax
  0000000000104D05: 48 8B 45 10        mov         rax,qword ptr [rbp+10h]
  0000000000104D09: 48 8B 40 10        mov         rax,qword ptr [rax+10h]
  0000000000104D0D: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000104D11: 48 8B 45 10        mov         rax,qword ptr [rbp+10h]
  0000000000104D15: 8B 40 0C           mov         eax,dword ptr [rax+0Ch]
  0000000000104D18: 48 8B 4D 10        mov         rcx,qword ptr [rbp+10h]
  0000000000104D1C: 48 03 41 10        add         rax,qword ptr [rcx+10h]
  0000000000104D20: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  0000000000104D24: 48 C7 45 28 00 00  mov         qword ptr [rbp+28h],0
                    00 00
  0000000000104D2C: 48 8B 45 20        mov         rax,qword ptr [rbp+20h]
  0000000000104D30: 48 39 45 18        cmp         qword ptr [rbp+18h],rax
  0000000000104D34: 0F 83 F0 00 00 00  jae         0000000000104E2A
  0000000000104D3A: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104D3E: 0F B6 40 01        movzx       eax,byte ptr [rax+1]
  0000000000104D42: 48 8B 4D 18        mov         rcx,qword ptr [rbp+18h]
  0000000000104D46: 0F B6 09           movzx       ecx,byte ptr [rcx]
  0000000000104D49: 44 8B C8           mov         r9d,eax
  0000000000104D4C: 44 8B C1           mov         r8d,ecx
  0000000000104D4F: 48 8D 15 BA 1C 00  lea         rdx,[??_C@_0N@IEILAMDH@T?3?5?$CFd?0?5L?3?5?$CFd@]
                    00
  0000000000104D56: 48 8B 0D 23 23 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104D5D: E8 5E CB FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104D62: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104D66: 0F B6 00           movzx       eax,byte ptr [rax]
  0000000000104D69: 83 F8 11           cmp         eax,11h
  0000000000104D6C: 75 78              jne         0000000000104DE6
  0000000000104D6E: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104D72: 0F B7 40 0C        movzx       eax,word ptr [rax+0Ch]
  0000000000104D76: 66 89 45 30        mov         word ptr [rbp+30h],ax
  0000000000104D7A: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000104D7E: 3D FF 7F 00 00     cmp         eax,7FFFh
  0000000000104D83: 75 1B              jne         0000000000104DA0
  0000000000104D85: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104D89: 8B 40 1C           mov         eax,dword ptr [rax+1Ch]
  0000000000104D8C: 48 C1 E0 14        shl         rax,14h
  0000000000104D90: 48 8B 4D 28        mov         rcx,qword ptr [rbp+28h]
  0000000000104D94: 48 03 C8           add         rcx,rax
  0000000000104D97: 48 8B C1           mov         rax,rcx
  0000000000104D9A: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000104D9E: EB 46              jmp         0000000000104DE6
  0000000000104DA0: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000104DA4: 3D FF FF 00 00     cmp         eax,0FFFFh
  0000000000104DA9: 74 3B              je          0000000000104DE6
  0000000000104DAB: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000104DAF: 25 00 80 00 00     and         eax,8000h
  0000000000104DB4: 85 C0              test        eax,eax
  0000000000104DB6: 74 18              je          0000000000104DD0
  0000000000104DB8: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000104DBC: 48 C1 E0 0A        shl         rax,0Ah
  0000000000104DC0: 48 8B 4D 28        mov         rcx,qword ptr [rbp+28h]
  0000000000104DC4: 48 03 C8           add         rcx,rax
  0000000000104DC7: 48 8B C1           mov         rax,rcx
  0000000000104DCA: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000104DCE: EB 16              jmp         0000000000104DE6
  0000000000104DD0: 0F B7 45 30        movzx       eax,word ptr [rbp+30h]
  0000000000104DD4: 48 C1 E0 14        shl         rax,14h
  0000000000104DD8: 48 8B 4D 28        mov         rcx,qword ptr [rbp+28h]
  0000000000104DDC: 48 03 C8           add         rcx,rax
  0000000000104DDF: 48 8B C1           mov         rax,rcx
  0000000000104DE2: 48 89 45 28        mov         qword ptr [rbp+28h],rax
  0000000000104DE6: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104DEA: 0F B6 40 01        movzx       eax,byte ptr [rax+1]
  0000000000104DEE: 48 8B 4D 18        mov         rcx,qword ptr [rbp+18h]
  0000000000104DF2: 48 03 C8           add         rcx,rax
  0000000000104DF5: 48 8B C1           mov         rax,rcx
  0000000000104DF8: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000104DFC: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104E00: 0F B7 00           movzx       eax,word ptr [rax]
  0000000000104E03: 85 C0              test        eax,eax
  0000000000104E05: 74 0D              je          0000000000104E14
  0000000000104E07: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104E0B: 48 FF C0           inc         rax
  0000000000104E0E: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000104E12: EB E8              jmp         0000000000104DFC
  0000000000104E14: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104E18: 48 83 C0 02        add         rax,2
  0000000000104E1C: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000104E20: E9 07 FF FF FF     jmp         0000000000104D2C
  0000000000104E25: E9 02 FF FF FF     jmp         0000000000104D2C
  0000000000104E2A: 33 D2              xor         edx,edx
  0000000000104E2C: 48 8B 45 28        mov         rax,qword ptr [rbp+28h]
  0000000000104E30: B9 00 04 00 00     mov         ecx,400h
  0000000000104E35: 48 F7 F1           div         rax,rcx
  0000000000104E38: 33 D2              xor         edx,edx
  0000000000104E3A: B9 00 04 00 00     mov         ecx,400h
  0000000000104E3F: 48 F7 F1           div         rax,rcx
  0000000000104E42: 4C 8B C0           mov         r8,rax
  0000000000104E45: 48 8D 15 D4 1B 00  lea         rdx,[??_C@_06CNJLBILI@RAM?5?$CFd@]
                    00
  0000000000104E4C: 48 8B 0D 2D 22 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104E53: E8 68 CA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104E58: 48 8B 45 28        mov         rax,qword ptr [rbp+28h]
  0000000000104E5C: EB 07              jmp         0000000000104E65
  0000000000104E5E: E9 2B FD FF FF     jmp         0000000000104B8E
  0000000000104E63: 33 C0              xor         eax,eax
  0000000000104E65: 48 8D A5 88 00 00  lea         rsp,[rbp+88h]
                    00
  0000000000104E6C: 5F                 pop         rdi
  0000000000104E6D: 5E                 pop         rsi
  0000000000104E6E: 5B                 pop         rbx
  0000000000104E6F: 5D                 pop         rbp
  0000000000104E70: C3                 ret
  0000000000104E71: CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC     ллллллллллллллл
?IsPagingEnabled@System@@QEAA_NXZ:
  0000000000104E80: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104E85: 55                 push        rbp
  0000000000104E86: 48 83 EC 70        sub         rsp,70h
  0000000000104E8A: 48 8D 6C 24 20     lea         rbp,[rsp+20h]
  0000000000104E8F: 0F 20 C0           mov         rax,cr0
  0000000000104E92: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000104E96: 44 8B 45 00        mov         r8d,dword ptr [rbp]
  0000000000104E9A: 48 8D 15 87 1B 00  lea         rdx,[??_C@_02NJNOFBBI@?$CFx@]
                    00
  0000000000104EA1: 48 8B 0D D8 21 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104EA8: E8 13 CA FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104EAD: B8 00 00 00 80     mov         eax,80000000h
  0000000000104EB2: 48 8B 4D 00        mov         rcx,qword ptr [rbp]
  0000000000104EB6: 48 23 C8           and         rcx,rax
  0000000000104EB9: 48 8B C1           mov         rax,rcx
  0000000000104EBC: 48 85 C0           test        rax,rax
  0000000000104EBF: 74 09              je          0000000000104ECA
  0000000000104EC1: C7 45 48 01 00 00  mov         dword ptr [rbp+48h],1
                    00
  0000000000104EC8: EB 07              jmp         0000000000104ED1
  0000000000104ECA: C7 45 48 00 00 00  mov         dword ptr [rbp+48h],0
                    00
  0000000000104ED1: 0F B6 45 48        movzx       eax,byte ptr [rbp+48h]
  0000000000104ED5: 48 8D 65 50        lea         rsp,[rbp+50h]
  0000000000104ED9: 5D                 pop         rbp
  0000000000104EDA: C3                 ret
  0000000000104EDB: CC CC CC CC CC                                   ллллл
?ResolveAddress@System@@QEAA_K_K@Z:
  0000000000104EE0: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  0000000000104EE5: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  0000000000104EEA: 55                 push        rbp
  0000000000104EEB: 48 81 EC 10 01 00  sub         rsp,110h
                    00
  0000000000104EF2: 48 8D 6C 24 40     lea         rbp,[rsp+40h]
  0000000000104EF7: 4C 8B 85 E8 00 00  mov         r8,qword ptr [rbp+0E8h]
                    00
  0000000000104EFE: 48 8D 15 2B 1B 00  lea         rdx,[??_C@_0BC@NNNLFDCF@Resolving?3?50x?$CF16x@]
                    00
  0000000000104F05: 48 8B 0D 74 21 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104F0C: E8 AF C9 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104F11: 48 8B 85 E8 00 00  mov         rax,qword ptr [rbp+0E8h]
                    00
  0000000000104F18: 48 25 FF 0F 00 00  and         rax,0FFFh
  0000000000104F1E: 89 45 00           mov         dword ptr [rbp],eax
  0000000000104F21: 48 8B 85 E8 00 00  mov         rax,qword ptr [rbp+0E8h]
                    00
  0000000000104F28: 48 C1 E8 0C        shr         rax,0Ch
  0000000000104F2C: 48 25 FF 01 00 00  and         rax,1FFh
  0000000000104F32: 89 45 04           mov         dword ptr [rbp+4],eax
  0000000000104F35: 48 8B 85 E8 00 00  mov         rax,qword ptr [rbp+0E8h]
                    00
  0000000000104F3C: 48 C1 E8 15        shr         rax,15h
  0000000000104F40: 48 25 FF 01 00 00  and         rax,1FFh
  0000000000104F46: 89 45 08           mov         dword ptr [rbp+8],eax
  0000000000104F49: 48 8B 85 E8 00 00  mov         rax,qword ptr [rbp+0E8h]
                    00
  0000000000104F50: 48 C1 E8 1E        shr         rax,1Eh
  0000000000104F54: 48 25 FF 01 00 00  and         rax,1FFh
  0000000000104F5A: 89 45 0C           mov         dword ptr [rbp+0Ch],eax
  0000000000104F5D: 48 8B 85 E8 00 00  mov         rax,qword ptr [rbp+0E8h]
                    00
  0000000000104F64: 48 C1 E8 27        shr         rax,27h
  0000000000104F68: 48 25 FF 01 00 00  and         rax,1FFh
  0000000000104F6E: 89 45 10           mov         dword ptr [rbp+10h],eax
  0000000000104F71: 8B 45 00           mov         eax,dword ptr [rbp]
  0000000000104F74: 89 44 24 30        mov         dword ptr [rsp+30h],eax
  0000000000104F78: 8B 45 04           mov         eax,dword ptr [rbp+4]
  0000000000104F7B: 89 44 24 28        mov         dword ptr [rsp+28h],eax
  0000000000104F7F: 8B 45 08           mov         eax,dword ptr [rbp+8]
  0000000000104F82: 89 44 24 20        mov         dword ptr [rsp+20h],eax
  0000000000104F86: 44 8B 4D 0C        mov         r9d,dword ptr [rbp+0Ch]
  0000000000104F8A: 44 8B 45 10        mov         r8d,dword ptr [rbp+10h]
  0000000000104F8E: 48 8D 15 B3 1A 00  lea         rdx,[??_C@_0CC@NONAEBKI@L4?3?5?$CFx?5L3?3?5?$CFx?5L2?3?5?$CFx?5L1?3?5?$CFx?5O?3?5@]
                    00
  0000000000104F95: 48 8B 0D E4 20 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104F9C: E8 1F C9 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104FA1: 0F 20 D8           mov         rax,cr3
  0000000000104FA4: 48 25 00 F0 FF FF  and         rax,0FFFFFFFFFFFFF000h
  0000000000104FAA: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  0000000000104FAE: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000104FB2: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  0000000000104FB6: 4C 8B 45 20        mov         r8,qword ptr [rbp+20h]
  0000000000104FBA: 48 8D 15 AF 1A 00  lea         rdx,[??_C@_0L@KCPPLAPB@L4?3?50x?$CF16x@]
                    00
  0000000000104FC1: 48 8B 0D B8 20 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  0000000000104FC8: E8 F3 C8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000104FCD: 8B 45 10           mov         eax,dword ptr [rbp+10h]
  0000000000104FD0: 89 45 28           mov         dword ptr [rbp+28h],eax
  0000000000104FD3: EB 08              jmp         0000000000104FDD
  0000000000104FD5: 8B 45 28           mov         eax,dword ptr [rbp+28h]
  0000000000104FD8: FF C0              inc         eax
  0000000000104FDA: 89 45 28           mov         dword ptr [rbp+28h],eax
  0000000000104FDD: 8B 45 10           mov         eax,dword ptr [rbp+10h]
  0000000000104FE0: 83 C0 05           add         eax,5
  0000000000104FE3: 39 45 28           cmp         dword ptr [rbp+28h],eax
  0000000000104FE6: 73 3B              jae         0000000000105023
  0000000000104FE8: 48 63 45 28        movsxd      rax,dword ptr [rbp+28h]
  0000000000104FEC: 48 8B 4D 20        mov         rcx,qword ptr [rbp+20h]
  0000000000104FF0: 48 8D 04 C1        lea         rax,[rcx+rax*8]
  0000000000104FF4: 48 89 45 38        mov         qword ptr [rbp+38h],rax
  0000000000104FF8: 48 8D 45 38        lea         rax,[rbp+38h]
  0000000000104FFC: 48 89 45 30        mov         qword ptr [rbp+30h],rax
  0000000000105000: 48 8B 45 30        mov         rax,qword ptr [rbp+30h]
  0000000000105004: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000105007: 44 8B 4D 28        mov         r9d,dword ptr [rbp+28h]
  000000000010500B: 4C 8B 00           mov         r8,qword ptr [rax]
  000000000010500E: 48 8D 15 6B 1A 00  lea         rdx,[??_C@_0BC@BDBCLIDG@?5?5L4?3?50x?$CF16x?5?9?5?$CFd@]
                    00
  0000000000105015: 48 8B 0D 64 20 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  000000000010501C: E8 9F C8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000105021: EB B2              jmp         0000000000104FD5
  0000000000105023: 8B 45 10           mov         eax,dword ptr [rbp+10h]
  0000000000105026: 48 8B 4D 20        mov         rcx,qword ptr [rbp+20h]
  000000000010502A: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  000000000010502E: 48 25 00 F0 FF FF  and         rax,0FFFFFFFFFFFFF000h
  0000000000105034: 48 89 45 40        mov         qword ptr [rbp+40h],rax
  0000000000105038: 4C 8B 45 40        mov         r8,qword ptr [rbp+40h]
  000000000010503C: 48 8D 15 55 1A 00  lea         rdx,[??_C@_0L@EACDKLII@L3?3?50x?$CF16x@]
                    00
  0000000000105043: 48 8B 0D 36 20 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  000000000010504A: E8 71 C8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  000000000010504F: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  0000000000105052: 89 45 48           mov         dword ptr [rbp+48h],eax
  0000000000105055: EB 08              jmp         000000000010505F
  0000000000105057: 8B 45 48           mov         eax,dword ptr [rbp+48h]
  000000000010505A: FF C0              inc         eax
  000000000010505C: 89 45 48           mov         dword ptr [rbp+48h],eax
  000000000010505F: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  0000000000105062: 83 C0 05           add         eax,5
  0000000000105065: 39 45 48           cmp         dword ptr [rbp+48h],eax
  0000000000105068: 73 3B              jae         00000000001050A5
  000000000010506A: 48 63 45 48        movsxd      rax,dword ptr [rbp+48h]
  000000000010506E: 48 8B 4D 40        mov         rcx,qword ptr [rbp+40h]
  0000000000105072: 48 8D 04 C1        lea         rax,[rcx+rax*8]
  0000000000105076: 48 89 45 58        mov         qword ptr [rbp+58h],rax
  000000000010507A: 48 8D 45 58        lea         rax,[rbp+58h]
  000000000010507E: 48 89 45 50        mov         qword ptr [rbp+50h],rax
  0000000000105082: 48 8B 45 50        mov         rax,qword ptr [rbp+50h]
  0000000000105086: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000105089: 44 8B 4D 48        mov         r9d,dword ptr [rbp+48h]
  000000000010508D: 4C 8B 00           mov         r8,qword ptr [rax]
  0000000000105090: 48 8D 15 11 1A 00  lea         rdx,[??_C@_0BC@IIBDLHCM@?5?5L3?3?50x?$CF16x?5?9?5?$CFd@]
                    00
  0000000000105097: 48 8B 0D E2 1F 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  000000000010509E: E8 1D C8 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  00000000001050A3: EB B2              jmp         0000000000105057
  00000000001050A5: 8B 45 0C           mov         eax,dword ptr [rbp+0Ch]
  00000000001050A8: 48 8B 4D 40        mov         rcx,qword ptr [rbp+40h]
  00000000001050AC: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  00000000001050B0: 48 25 00 F0 FF FF  and         rax,0FFFFFFFFFFFFF000h
  00000000001050B6: 48 89 45 60        mov         qword ptr [rbp+60h],rax
  00000000001050BA: 8B 45 08           mov         eax,dword ptr [rbp+8]
  00000000001050BD: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001050C1: 4C 8B 0C C1        mov         r9,qword ptr [rcx+rax*8]
  00000000001050C5: 4C 8B 45 60        mov         r8,qword ptr [rbp+60h]
  00000000001050C9: 48 8D 15 F0 19 00  lea         rdx,[??_C@_0BE@HEILFI@L2?3?50x?$CF16x?5?9?50x?$CF16x@]
                    00
  00000000001050D0: 48 8B 0D A9 1F 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  00000000001050D7: E8 E4 C7 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  00000000001050DC: 8B 45 08           mov         eax,dword ptr [rbp+8]
  00000000001050DF: 89 45 68           mov         dword ptr [rbp+68h],eax
  00000000001050E2: EB 08              jmp         00000000001050EC
  00000000001050E4: 8B 45 68           mov         eax,dword ptr [rbp+68h]
  00000000001050E7: FF C0              inc         eax
  00000000001050E9: 89 45 68           mov         dword ptr [rbp+68h],eax
  00000000001050EC: 8B 45 08           mov         eax,dword ptr [rbp+8]
  00000000001050EF: 83 C0 05           add         eax,5
  00000000001050F2: 39 45 68           cmp         dword ptr [rbp+68h],eax
  00000000001050F5: 73 3B              jae         0000000000105132
  00000000001050F7: 48 63 45 68        movsxd      rax,dword ptr [rbp+68h]
  00000000001050FB: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  00000000001050FF: 48 8D 04 C1        lea         rax,[rcx+rax*8]
  0000000000105103: 48 89 45 78        mov         qword ptr [rbp+78h],rax
  0000000000105107: 48 8D 45 78        lea         rax,[rbp+78h]
  000000000010510B: 48 89 45 70        mov         qword ptr [rbp+70h],rax
  000000000010510F: 48 8B 45 70        mov         rax,qword ptr [rbp+70h]
  0000000000105113: 48 8B 00           mov         rax,qword ptr [rax]
  0000000000105116: 44 8B 4D 68        mov         r9d,dword ptr [rbp+68h]
  000000000010511A: 4C 8B 00           mov         r8,qword ptr [rax]
  000000000010511D: 48 8D 15 B4 19 00  lea         rdx,[??_C@_0BC@DBOIGMME@?5?5L2?3?50x?$CF16x?5?9?5?$CFd@]
                    00
  0000000000105124: 48 8B 0D 55 1F 00  mov         rcx,qword ptr [?loading@@3PEAVLoadingScreen@@EA]
                    00
  000000000010512B: E8 90 C7 FF FF     call        ?WriteLineFormat@LoadingScreen@@QEAAXPEBDZZ
  0000000000105130: EB B2              jmp         00000000001050E4
  0000000000105132: 8B 45 08           mov         eax,dword ptr [rbp+8]
  0000000000105135: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000105139: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  000000000010513D: 48 C1 E8 07        shr         rax,7
  0000000000105141: 48 83 E0 01        and         rax,1
  0000000000105145: 48 83 F8 01        cmp         rax,1
  0000000000105149: 75 24              jne         000000000010516F
  000000000010514B: 8B 45 08           mov         eax,dword ptr [rbp+8]
  000000000010514E: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000105152: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  0000000000105156: 48 25 00 00 E0 FF  and         rax,0FFFFFFFFFFE00000h
  000000000010515C: 48 8B 8D E8 00 00  mov         rcx,qword ptr [rbp+0E8h]
                    00
  0000000000105163: 48 81 E1 FF FF 1F  and         rcx,1FFFFFh
                    00
  000000000010516A: 48 0B C1           or          rax,rcx
  000000000010516D: EB 32              jmp         00000000001051A1
  000000000010516F: 8B 45 08           mov         eax,dword ptr [rbp+8]
  0000000000105172: 48 8B 4D 60        mov         rcx,qword ptr [rbp+60h]
  0000000000105176: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  000000000010517A: 48 25 00 F0 FF FF  and         rax,0FFFFFFFFFFFFF000h
  0000000000105180: 48 89 85 80 00 00  mov         qword ptr [rbp+80h],rax
                    00
  0000000000105187: 8B 45 04           mov         eax,dword ptr [rbp+4]
  000000000010518A: 48 8B 8D 80 00 00  mov         rcx,qword ptr [rbp+80h]
                    00
  0000000000105191: 48 8B 04 C1        mov         rax,qword ptr [rcx+rax*8]
  0000000000105195: 48 25 00 F0 FF FF  and         rax,0FFFFFFFFFFFFF000h
  000000000010519B: 8B 4D 00           mov         ecx,dword ptr [rbp]
  000000000010519E: 48 03 C1           add         rax,rcx
  00000000001051A1: 48 8D A5 D0 00 00  lea         rsp,[rbp+0D0h]
                    00
  00000000001051A8: 5D                 pop         rbp
  00000000001051A9: C3                 ret
  00000000001051AA: CC CC CC CC CC CC                                лллллл
?memcmp@CRT@@SAHPEBX0I@Z:
  00000000001051B0: 44 89 44 24 18     mov         dword ptr [rsp+18h],r8d
  00000000001051B5: 48 89 54 24 10     mov         qword ptr [rsp+10h],rdx
  00000000001051BA: 48 89 4C 24 08     mov         qword ptr [rsp+8],rcx
  00000000001051BF: 55                 push        rbp
  00000000001051C0: 48 83 EC 70        sub         rsp,70h
  00000000001051C4: 48 8B EC           mov         rbp,rsp
  00000000001051C7: 48 8B 85 80 00 00  mov         rax,qword ptr [rbp+80h]
                    00
  00000000001051CE: 48 89 45 00        mov         qword ptr [rbp],rax
  00000000001051D2: 48 8B 85 88 00 00  mov         rax,qword ptr [rbp+88h]
                    00
  00000000001051D9: 48 89 45 08        mov         qword ptr [rbp+8],rax
  00000000001051DD: C7 45 10 08 00 00  mov         dword ptr [rbp+10h],8
                    00
  00000000001051E4: C7 45 14 00 00 00  mov         dword ptr [rbp+14h],0
                    00
  00000000001051EB: 33 D2              xor         edx,edx
  00000000001051ED: 8B 85 90 00 00 00  mov         eax,dword ptr [rbp+90h]
  00000000001051F3: F7 75 10           div         eax,dword ptr [rbp+10h]
  00000000001051F6: 39 45 14           cmp         dword ptr [rbp+14h],eax
  00000000001051F9: 73 58              jae         0000000000105253
  00000000001051FB: 48 8B 45 00        mov         rax,qword ptr [rbp]
  00000000001051FF: 48 8B 4D 08        mov         rcx,qword ptr [rbp+8]
  0000000000105203: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000105206: 48 39 08           cmp         qword ptr [rax],rcx
  0000000000105209: 73 0C              jae         0000000000105217
  000000000010520B: B8 FF FF FF FF     mov         eax,0FFFFFFFFh
  0000000000105210: E9 BD 00 00 00     jmp         00000000001052D2
  0000000000105215: EB 1A              jmp         0000000000105231
  0000000000105217: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010521B: 48 8B 4D 08        mov         rcx,qword ptr [rbp+8]
  000000000010521F: 48 8B 09           mov         rcx,qword ptr [rcx]
  0000000000105222: 48 39 08           cmp         qword ptr [rax],rcx
  0000000000105225: 76 0A              jbe         0000000000105231
  0000000000105227: B8 01 00 00 00     mov         eax,1
  000000000010522C: E9 A1 00 00 00     jmp         00000000001052D2
  0000000000105231: 8B 45 14           mov         eax,dword ptr [rbp+14h]
  0000000000105234: FF C0              inc         eax
  0000000000105236: 89 45 14           mov         dword ptr [rbp+14h],eax
  0000000000105239: 48 8B 45 00        mov         rax,qword ptr [rbp]
  000000000010523D: 48 83 C0 08        add         rax,8
  0000000000105241: 48 89 45 00        mov         qword ptr [rbp],rax
  0000000000105245: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  0000000000105249: 48 83 C0 08        add         rax,8
  000000000010524D: 48 89 45 08        mov         qword ptr [rbp+8],rax
  0000000000105251: EB 98              jmp         00000000001051EB
  0000000000105253: 48 8B 45 00        mov         rax,qword ptr [rbp]
  0000000000105257: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  000000000010525B: 48 8B 45 08        mov         rax,qword ptr [rbp+8]
  000000000010525F: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  0000000000105263: C7 45 14 00 00 00  mov         dword ptr [rbp+14h],0
                    00
  000000000010526A: 33 D2              xor         edx,edx
  000000000010526C: 8B 85 90 00 00 00  mov         eax,dword ptr [rbp+90h]
  0000000000105272: F7 75 10           div         eax,dword ptr [rbp+10h]
  0000000000105275: 8B C2              mov         eax,edx
  0000000000105277: 39 45 14           cmp         dword ptr [rbp+14h],eax
  000000000010527A: 73 54              jae         00000000001052D0
  000000000010527C: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  0000000000105280: 0F B6 00           movzx       eax,byte ptr [rax]
  0000000000105283: 48 8B 4D 20        mov         rcx,qword ptr [rbp+20h]
  0000000000105287: 0F B6 09           movzx       ecx,byte ptr [rcx]
  000000000010528A: 3B C1              cmp         eax,ecx
  000000000010528C: 7D 09              jge         0000000000105297
  000000000010528E: B8 FF FF FF FF     mov         eax,0FFFFFFFFh
  0000000000105293: EB 3D              jmp         00000000001052D2
  0000000000105295: EB 19              jmp         00000000001052B0
  0000000000105297: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  000000000010529B: 0F B6 00           movzx       eax,byte ptr [rax]
  000000000010529E: 48 8B 4D 20        mov         rcx,qword ptr [rbp+20h]
  00000000001052A2: 0F B6 09           movzx       ecx,byte ptr [rcx]
  00000000001052A5: 3B C1              cmp         eax,ecx
  00000000001052A7: 7E 07              jle         00000000001052B0
  00000000001052A9: B8 01 00 00 00     mov         eax,1
  00000000001052AE: EB 22              jmp         00000000001052D2
  00000000001052B0: 8B 45 14           mov         eax,dword ptr [rbp+14h]
  00000000001052B3: FF C0              inc         eax
  00000000001052B5: 89 45 14           mov         dword ptr [rbp+14h],eax
  00000000001052B8: 48 8B 45 18        mov         rax,qword ptr [rbp+18h]
  00000000001052BC: 48 FF C0           inc         rax
  00000000001052BF: 48 89 45 18        mov         qword ptr [rbp+18h],rax
  00000000001052C3: 48 8B 45 20        mov         rax,qword ptr [rbp+20h]
  00000000001052C7: 48 FF C0           inc         rax
  00000000001052CA: 48 89 45 20        mov         qword ptr [rbp+20h],rax
  00000000001052CE: EB 9A              jmp         000000000010526A
  00000000001052D0: 33 C0              xor         eax,eax
  00000000001052D2: 48 8D 65 70        lea         rsp,[rbp+70h]
  00000000001052D6: 5D                 pop         rbp
  00000000001052D7: C3                 ret
  00000000001052D8: CC CC CC CC CC CC CC CC                          лллллллл
DE_ISR_HANDLER:
  00000000001052E0: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  00000000001052EA: FF D0              call        rax
  00000000001052EC: 48 CF              iretq
DB_ISR_HANDLER:
  00000000001052EE: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  00000000001052F8: FF D0              call        rax
  00000000001052FA: 48 CF              iretq
NMI_ISR_HANDLER:
  00000000001052FC: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105306: FF D0              call        rax
  0000000000105308: 48 CF              iretq
BP_ISR_HANDLER:
  000000000010530A: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105314: FF D0              call        rax
  0000000000105316: 48 CF              iretq
OF_ISR_HANDLER:
  0000000000105318: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105322: FF D0              call        rax
  0000000000105324: 48 CF              iretq
BR_ISR_HANDLER:
  0000000000105326: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105330: FF D0              call        rax
  0000000000105332: 48 CF              iretq
UD_ISR_HANDLER:
  0000000000105334: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  000000000010533E: FF D0              call        rax
  0000000000105340: 48 CF              iretq
NM_ISR_HANDLER:
  0000000000105342: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  000000000010534C: FF D0              call        rax
  000000000010534E: 48 CF              iretq
DF_EXC_HANDLER:
  0000000000105350: F4                 hlt
  0000000000105351: 55                 push        rbp
  0000000000105352: 48 8B EC           mov         rbp,rsp
  0000000000105355: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  000000000010535F: FF D0              call        rax
  0000000000105361: 5D                 pop         rbp
CSO_ISR_HANDLER:
  0000000000105362: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  000000000010536C: FF D0              call        rax
  000000000010536E: 48 CF              iretq
TS_EXC_HANDLER:
  0000000000105370: F4                 hlt
  0000000000105371: 55                 push        rbp
  0000000000105372: 48 8B EC           mov         rbp,rsp
  0000000000105375: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  000000000010537F: FF D0              call        rax
  0000000000105381: 5D                 pop         rbp
NP_EXC_HANDLER:
  0000000000105382: F4                 hlt
  0000000000105383: 55                 push        rbp
  0000000000105384: 48 8B EC           mov         rbp,rsp
  0000000000105387: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  0000000000105391: FF D0              call        rax
  0000000000105393: 5D                 pop         rbp
SS_EXC_HANDLER:
  0000000000105394: F4                 hlt
  0000000000105395: 55                 push        rbp
  0000000000105396: 48 8B EC           mov         rbp,rsp
  0000000000105399: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  00000000001053A3: FF D0              call        rax
  00000000001053A5: 5D                 pop         rbp
GP_EXC_HANDLER:
  00000000001053A6: F4                 hlt
  00000000001053A7: 55                 push        rbp
  00000000001053A8: 48 8B EC           mov         rbp,rsp
  00000000001053AB: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  00000000001053B5: FF D0              call        rax
  00000000001053B7: 5D                 pop         rbp
PF_EXC_HANDLER:
  00000000001053B8: F4                 hlt
  00000000001053B9: 55                 push        rbp
  00000000001053BA: 48 8B EC           mov         rbp,rsp
  00000000001053BD: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  00000000001053C7: FF D0              call        rax
  00000000001053C9: 5D                 pop         rbp
MF_ISR_HANDLER:
  00000000001053CA: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  00000000001053D4: FF D0              call        rax
  00000000001053D6: 48 CF              iretq
AC_EXC_HANDLER:
  00000000001053D8: F4                 hlt
  00000000001053D9: 55                 push        rbp
  00000000001053DA: 48 8B EC           mov         rbp,rsp
  00000000001053DD: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  00000000001053E7: FF D0              call        rax
  00000000001053E9: 5D                 pop         rbp
MC_ISR_HANDLER:
  00000000001053EA: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  00000000001053F4: FF D0              call        rax
  00000000001053F6: 48 CF              iretq
XM_ISR_HANDLER:
  00000000001053F8: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105402: FF D0              call        rax
  0000000000105404: 48 CF              iretq
VE_ISR_HANDLER:
  0000000000105406: 48 B8 F0 1C 10 00  mov         rax,offset INTERRUPT_HANDLER
                    00 00 00 00
  0000000000105410: FF D0              call        rax
  0000000000105412: 48 CF              iretq
SX_EXC_HANDLER:
  0000000000105414: F4                 hlt
  0000000000105415: 55                 push        rbp
  0000000000105416: 48 8B EC           mov         rbp,rsp
  0000000000105419: 48 B8 B0 1C 10 00  mov         rax,offset EXCEPTION_HANDLER
                    00 00 00 00
  0000000000105423: FF D0              call        rax
  0000000000105425: 5D                 pop         rbp
  0000000000105426: CC CC CC CC CC CC CC CC CC CC                    лллллллллл
x64_main:
  0000000000105430: 48 8B 2D D1 1B 00  mov         rbp,qword ptr [KERNEL_STACK_STOP]
                    00
  0000000000105437: 48 8B 25 CA 1B 00  mov         rsp,qword ptr [KERNEL_STACK_STOP]
                    00
  000000000010543E: 48 8D 05 EB C8 FF  lea         rax,[main]
                    FF
  0000000000105445: FF E0              jmp         rax
  0000000000105447: CC CC CC CC CC CC CC CC CC                       ллллллллл
x64_ltr:
  0000000000105450: 0F 00 D9           ltr         cx
  0000000000105453: C3                 ret
x64_rflags:
  0000000000105454: 9C                 pushfq
  0000000000105455: 58                 pop         rax
  0000000000105456: C3                 ret
x64_sti:
  0000000000105457: FB                 sti
  0000000000105458: C3                 ret
x64_cli:
  0000000000105459: FB                 sti
  000000000010545A: C3                 ret
x64_update_segments:
  000000000010545B: 66 8E D9           mov         ds,cx
  000000000010545E: 66 8E C1           mov         es,cx
  0000000000105461: 66 8E E1           mov         fs,cx
  0000000000105464: 66 8E E9           mov         gs,cx
  0000000000105467: 66 8E D1           mov         ss,cx
  000000000010546A: 48 0F B7 D2        movzx       rdx,dx
  000000000010546E: 48 8D 0D 03 00 00  lea         rcx,[105478h]
                    00
  0000000000105475: 52                 push        rdx
  0000000000105476: 51                 push        rcx
  0000000000105477: CB                 retf
  0000000000105478: C3                 ret
x64_ReadCS:
  0000000000105479: 66 8C C8           mov         ax,cs
  000000000010547C: C3                 ret
x64_ReadDS:
  000000000010547D: 66 8C D8           mov         ax,ds
  0000000000105480: C3                 ret
x64_WriteDS:
  0000000000105481: 66 8E D9           mov         ds,cx
  0000000000105484: C3                 ret
x64_ReadES:
  0000000000105485: 66 8C C0           mov         ax,es
  0000000000105488: C3                 ret
x64_WriteES:
  0000000000105489: 66 8E C1           mov         es,cx
  000000000010548C: C3                 ret
x64_ReadFS:
  000000000010548D: 66 8C E0           mov         ax,fs
  0000000000105490: C3                 ret
x64_WriteFS:
  0000000000105491: 66 8E E1           mov         fs,cx
  0000000000105494: C3                 ret
x64_ReadGS:
  0000000000105495: 66 8C E8           mov         ax,gs
  0000000000105498: C3                 ret
x64_WriteGS:
  0000000000105499: 66 8E E9           mov         gs,cx
  000000000010549C: C3                 ret
x64_ReadSS:
  000000000010549D: 66 8C D0           mov         ax,ss
  00000000001054A0: C3                 ret
x64_WriteSS:
  00000000001054A1: 66 8E D1           mov         ss,cx
  00000000001054A4: C3                 ret
x64_ReadSP:
  00000000001054A5: 48 8B C4           mov         rax,rsp
  00000000001054A8: C3                 ret
x64_ReadRCX:
  00000000001054A9: 48 8B C1           mov         rax,rcx
  00000000001054AC: C3                 ret
x64_ReadRDX:
  00000000001054AD: 48 8B C2           mov         rax,rdx
  00000000001054B0: C3                 ret

  Summary

        1000 .CRT
      106000 .data
        1000 .pdata
        1000 .rdata
        1000 .reloc
        1000 .rsrc
        5000 .text
