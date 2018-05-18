/* The routine to call the main function in the ezboot. */
.globl _ram_entry
_ram_entry:
	bl	main	   // C 함수의 main함수를 호출한다.
	b	_ram_entry // main 함수의 호출이 끝났다면 재 호출한다.
