; ide.s — ATA PIO LBA sector read
[BITS 32]
global ata_lba_read

; int ata_lba_read(uint32_t lba, uint8_t* buf, uint8_t sectors)
; reads `sectors` 512-byte blocks starting at LBA into buf
ata_lba_read:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push edx
    push ecx
    push ebx

    mov eax, [ebp + 8]      ; LBA
    mov edi, [ebp + 12]     ; buffer pointer
    mov cl,  [ebp + 16]     ; number of sectors

    mov dx, 0x1F2
    mov al, cl
    out dx, al              ; sector count

    mov eax, [ebp + 8]
    mov dx, 0x1F3
    out dx, al              ; LBA low
    shr eax, 8
    mov dx, 0x1F4
    out dx, al              ; LBA mid
    shr eax, 8
    mov dx, 0x1F5
    out dx, al              ; LBA high
    shr eax, 8
    mov dx, 0x1F6
    and al, 0x0F
    or  al, 0xE0            ; master, LBA mode
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al              ; read sectors command

.next_sector:
.wait:
    in al, dx
    test al, 8
    jz .wait

    mov ecx, 256
    mov dx, 0x1F0
.read_word:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .read_word

    dec cl
    jnz .next_sector

    pop ebx
    pop ecx
    pop edx
    pop edi
    pop esi
    leave
    ret
