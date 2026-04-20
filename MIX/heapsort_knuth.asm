global heapsort_knuth

section .text

; void heapsort_knuth(int *a, int n)
; System V AMD64 ABI:
; rdi = a
; esi = n

heapsort_knuth:
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov r12, rdi              ; a = rdi
    cmp esi, 1
    jle DONE

H1:
    mov eax, esi
    shr eax, 1
    mov r13d, eax             ; l = n / 2

    mov eax, esi
    dec eax
    mov r14d, eax             ; r = n - 1

H2:
    cmp r13d, 0
    jle H2_ELSE

    dec r13d                  ; l = l - 1
    movsxd rax, r13d
    mov ebx, [r12 + rax*4]    ; R = a[l]
    jmp H3

H2_ELSE:
    movsxd rax, r14d
    mov ebx, [r12 + rax*4]    ; R = a[r]
    mov edx, [r12]            ; tmp = a[0]
    mov [r12 + rax*4], edx    ; a[r] = a[0]
    dec r14d                  ; r = r - 1
    cmp r14d, 0
    jne H3

    mov [r12], ebx            ; a[0] = R
    jmp DONE

H3:
    mov r15d, r13d            ; i = l
    lea ecx, [r15d + r15d + 1] ; j = 2*i + 1

H4:
    cmp ecx, r14d
    jg H8                     ; if (j > r) goto H8

    cmp ecx, r14d
    jl H5                     ; if (j < r) goto H5

    ; j == r
    jmp H6

H5:
    movsxd rax, ecx
    mov edx, [r12 + rax*4]        ; a[j]
    mov edi, [r12 + rax*4 + 4]    ; a[j+1]
    cmp edx, edi
    jge H6
    inc ecx                       ; j = j + 1

H6:
    movsxd rax, ecx
    mov edx, [r12 + rax*4]        ; a[j]
    cmp ebx, edx                  ; R >= a[j] ?
    jge H8

H7:
    movsxd rax, r15d
    mov [r12 + rax*4], edx        ; a[i] = a[j]
    mov r15d, ecx                 ; i = j
    lea ecx, [r15d + r15d + 1]    ; j = 2*i + 1
    jmp H4

H8:
    movsxd rax, r15d
    mov [r12 + rax*4], ebx        ; a[i] = R
    jmp H2

DONE:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
