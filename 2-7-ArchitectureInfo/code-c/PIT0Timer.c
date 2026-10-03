// init Intel 8253 Programmable Interval Timer (PIT)
// (channel 0 controlling interrupt 8 needs to be switched
// to mode 2 (Rate Generator) while default mode is 3 (Square Wave Generator))
void initPIT0Timer()
{
	__asm {
		mov   al,00110100b
		out   0x43,al
		xor   al,al  // counter reload value will be zero
		out   0x40,al  // low byte
		out   0x40,al  // high byte
	};
}


void restoreDefaultPIT0Timer()
{
	__asm {
		mov   al,00110110b
		out   0x43,al
		xor   al,al  // counter reload value will be zero
		out   0x40,al  // low byte
		out   0x40,al  // high byte
	};
}


// based on:
// https://brokenthorn.com/Resources/OSDevPit.html
// and other random information from internet
__int64 readPIT0Ticks()
{
#if defined(__WATCOMC__)
	__int64 r = 0;
#else
	__int64 r;
#endif
	__asm {

		// prepare registers
		push  ds
		mov   ax,0x40
		mov   ds,ax

		// read counter 0 on Intel 8253 PIT and BIOS TimerTicks on 0x40:0x6c
	again:
		xor   al,al
		pushf
		cli
		mov   dx,ds:[0x6c]  // read BIOS Timer Ticks low word
		mov   cx,ds:[0x6e]  // read BIOS Timer Ticks high word
		out   0x43,al  // counter 0 latch command
		in    al,0x40  // read counter 0 low byte
		mov   bl,al
		in    al,0x40  // read counter 0 high byte
		popf
		mov   bh,al

		// detect pending IRQ0 interrupt on Intel 8259 PIC
		// (IRQ0 is activated from counter 0 when transitioning from value 1 to 0)
		pushf
		mov   al,0x0a
		cli
		out   0x20,al
		in    al,0x20
		and   al,0x01
		jnz   interrupted

		// detect BIOS Timer Tick change
		cmp   dx,ds:[0x6c]  // read BIOS Timer Ticks low word
		jne   interrupted
		cmp   cx,ds:[0x6e]  // read BIOS Timer Ticks high word
		je    noInterruption

		// if interrupt happened, solve it by returning counter value zero and BIOS Timer Ticks increased by one
	interrupted:
		xor   bx,bx
		add   dx,1
		adc   cx,0
	noInterruption:
		popf

		// compute ticks from counter 0
		dec   bx  // returned value can be 0, 65535, 65534, 65533,....; so, we decrement it by one to skip zero value
		not   bx  // we invert the sequence to 0, 1, 2, 3, ....

		// restore state and store results
		pop   ds
		xor   ax,ax
		mov   word ptr r,bx
		mov   word ptr r+2,dx
		mov   word ptr r+4,cx
		mov   word ptr r+6,ax
	};

	return r;
}
