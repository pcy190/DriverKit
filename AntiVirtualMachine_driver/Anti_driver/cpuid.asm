EXTERN a:DWORD;
EXTERN b:DWORD;
EXTERN c_var:DWORD;
EXTERN d:DWORD;

.CODE
getcpuid PROC
	xor eax,eax
	mov eax,1h
	cpuid
	mov a,eax
	mov b,ebx
	mov c_var,ecx  
	mov d,edx
	ret
getcpuid ENDP
END