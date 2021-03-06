;xlock2ld.asm   --password harddisk lock program
;tasm xlock2ld
;tlink /t xlock2ld
;book: HARDDISK PROTECTION TECHNIQUE HANDBOOK
;PAGE 79
;PROGRAMMING ON 2/15/1999
;MODIFY ON 2/15/1999
passmaxlen    equ  15
        .model tiny
	.code
	org    100h
begin:
       push  cs
       pop   ds
       push  cs
       pop   es
       mov   ax,0201h
       mov   bx,offset buffer
       mov   cx,0001h
       mov   dx,0080h
       int   13h
       cld
       mov   si,offset buffer+01beh
       mov   di,offset buffer+01beh
       mov   cx,21h
       repne movsw
       mov   ah,09h
       mov   dx,offset askmsg
       int   21h
       mov   di,offset sector+1beh-16
       call  readstr
       mov   ax,0301h
       mov   bx,offset sector
       mov   cx,0001h
       mov   dx,0080h
       int   13h
       mov   ah,4ch
       int   21h
askmsg   db   'Password:$'
plength  db   0
readstr    proc near
       push  es
       push  di
       push  cs
       pop   es
       inc   di
       mov   plength,0
next:
       cmp   plength,passmaxlen
       jge   @l1
       mov   ah,1
       int   16h
       jz    next
       mov   ax,0
       int   16h
       cmp   al,0
       je    next
       cmp   al,08h
       je    @l2
       cmp   al,13
       je    @l1
       ;sub  al,20
       ;or   al,80h
       stosb
       mov   al,'*'
       mov   bx,0
       mov   ah,0eh
       int   10h
       inc   plength
       jmp   next
@l2:
       cmp   plength,0
       je    next
       dec   di
       mov   al,00h
       stosb
       mov   ah,03h
       mov   bh,00h
       int   10h
       dec   dx
       mov   ah,02h
       int   10h
       mov   al,' '
       mov   ah,0eh
       int   10h
       mov   ah,02h
       int   10h
       dec   plength
       dec   di
       jmp   next
@l1:
       pop   di
       mov   al,byte ptr plength
       mov   byte ptr [di],al
       pop   es
       ret
readstr    endp
buffer     db   512 dup(0)
sector  db  0FAh,33h,0C0h,8Eh,0D0h,0BCh,00h,7Ch,8Bh,0F4h,50h,07h
	db  50h,1Fh,0FBh,0FCh,0BFh,00h,06h,0B9h,00h,01h,0F2h,0A5h
	db  0EAh,1Dh,06h,00h,00h,0BEh,0F3h,06h,0E8h,0EEh,00h,0BFh
	db  00h,08h,0E8h,0F8h,00h,0BEh,00h,08h,0BFh,0AEh,07h,32h
	db  0EDh,8Ah,0Dh,0F3h,0A6h,74h,08h,0BEh,0FDh,06h,0E8h,0D4h
	db  00h,0EBh,0FEh,0BEh,0BEh,07h,0B3h,04h,80h,3Ch,80h,74h
	db  0Eh,80h,3Ch,00h,75h,1Ch,83h,0C6h,10h,0FEh,0CBh,75h
	db  0EFh,0CDh,18h,8Bh,14h,8Bh,4Ch,02h,8Bh,0EEh,83h,0C6h
	db  10h,0FEh,0CBh,74h,0Dh,80h,3Ch,00h,74h,0F4h,0BEh,0A0h
	db  06h,0E8h,0A1h,00h,0EBh,0FEh,0BFh,05h,00h,0BBh,00h,7Ch
	db  0B8h,01h,02h,57h,0CDh,13h,5Fh,73h,0Ch,33h,0C0h,0CDh
	db  13h,4Fh,75h,0EDh,0BEh,0B9h,06h,0EBh,0E0h,0BEh,0D9h,06h
	db  0BFh,0FEh,7Dh,81h,3Dh,55h,0AAh,75h,0D4h,8Bh,0F5h,0EAh
	db  00h,7Ch,00h,00h,49h,6Eh,76h,61h,6Ch,69h,64h,20h
	db  70h,61h,72h,74h,69h,74h,69h,6Fh,6Eh,20h,74h,61h
	db  62h,6Ch,65h,21h,00h,45h,72h,72h,6Fh,72h,20h,6Ch
	db  6Fh,61h,64h,69h,6Eh,67h,20h,6Fh,70h,65h,72h,61h
	db  74h,69h,6Eh,67h,20h,73h,79h,73h,74h,65h,6Dh,21h
	db  00h,4Dh,69h,73h,73h,69h,6Eh,67h,20h,6Fh,70h,65h
	db  72h,61h,74h,69h,6Eh,67h,20h,73h,79h,73h,74h,65h
	db  6Dh,21h,00h,50h,61h,73h,73h,77h,6Fh,72h,64h,3Ah
	db  00h,55h,6Eh,61h,75h,74h,68h,6Fh,72h,69h,7Ah,65h
	db  64h,20h,75h,73h,65h,72h,21h,00h,00h,0ACh,3Ch,00h
	db  74h,0Ah,0B7h,09h,56h,0B4h,0Eh,0CDh,10h,5Eh,0EBh,0F1h
	db  0C3h,06h,57h,0Eh,07h,47h,0C6h,06h,10h,01h,00h,80h
	db  3Eh,10h,01h,0Fh,7Dh,4Eh,0B4h,01h,0CDh,16h,74h,0F3h
	db  0B8h,00h,00h,0CDh,16h,3Ch,00h,74h,0EAh,3Ch,08h,74h
	db  14h,3Ch,0Dh,74h,37h,0AAh,0B0h,2Ah,0BBh,00h,00h,0B4h
	db  0Eh,0CDh,10h,0FEh,06h,10h,01h,0EBh,0D2h,80h,3Eh,10h
	db  01h,00h,74h,0CBh,4Fh,0B0h,00h,0AAh,0B4h,03h,0B7h,00h
	db  0CDh,10h,4Ah,0B4h,02h,0CDh,10h,0B0h,20h,0B4h,0Eh,0CDh
	db  10h,0B4h,02h,0CDh,10h,0FEh,0Eh,10h,01h,4Fh,0EBh,0ABh
	db  5Fh,0A0h,10h,01h,88h,05h,07h,0C3h,00h,00h,00h,00h
	db  00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	db  00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	db  00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,05h,00h
	db  00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h,00h
	db  00h,00h,80h,01h,01h,00h,0Bh,0FEh,3Fh,79h,3Fh,00h
	db  00h,00h,0BBh,0E7h,1Dh,00h,00h,00h,01h,7Ah,82h,0FEh
	db  3Fh,80h,0FAh,0E7h,1Dh,00h,47h,0B7h,01h,00h,00h,00h
	db  01h,81h,83h,0FEh,3Fh,0E6h,41h,9Fh,1Fh,00h,0E6h,00h
	db  19h,00h,00h,00h,01h,0E7h,0Fh,0FEh,0BFh,0Bh,27h,0A0h
	db  38h,00h,0A6h,11h,48h,00h,55h,0AAh
     end begin