/* The routine to call the main function in the ezboot. */
.globl _ram_entry
_ram_entry:
	bl	main	   // call the main function of the c program
	b	_ram_entry // when it is finished to call the main funciton, recall it
