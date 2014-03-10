                .section .quadrature_encoder.cog, "ax"
                .global QuadratureEncoder_Entry
                .compress off

// Had to change the relative addressing of some things from long to byte address                        
                        

QuadratureEncoder_Entry 
..start
		.org	0
                        
                mov T1, par
                rdlong Pin, T1
                add T1, #4
                rdlong Totenc, T1
                add T1, #4
                mov Pos, T1
                
QuadratureEncoder
		test	Pin, #$20	wc
		muxc	QuadratureEncoder.pinsrc, #1
		mov	Iposaddr, #((Intpos-..start)/4)
		movd	QuadratureEncoder.iclear, Iposaddr
		mov	Idx, Totenc
QuadratureEncoder.iclear
		mov	0, #0
		add	Iposaddr, #4
		movd	QuadratureEncoder.iclear, Iposaddr
		djnz	Idx, #QuadratureEncoder.iclear
		mov	St2, INA
		shr	St2, Pin
QuadratureEncoder.sample
		mov	Iposaddr, #((Intpos-..start)/4)
		movd	(QuadratureEncoder.ipos), Iposaddr
		movd	(QuadratureEncoder.ipos + 4), Iposaddr
		mov	Mposaddr, Pos
		mov	St1, St2
		mov	T1, St2
		shl	T1, #1
QuadratureEncoder.pinsrc
		mov	St2, INB
		shr	St2, Pin
		xor	St1, St2
		xor	T1, St2
		and	T1, Bmask
		or	T1, Amask
		mov	T2, St1
		and	T2, Amask
		and	St1, Bmask
		shr	St1, #1
		xor	T2, St1
		mov	St1, T2
		shl	St1, #1
		or	St1, T2
		and	St1, T1
		mov	Idx, Totenc
QuadratureEncoder.updatepos
		ror	St1, #2
		mov	Diff, St1
		sar	Diff, #$1e
QuadratureEncoder.ipos
		add	0, Diff
QuadratureEncoder.ipos2
		wrlong	0, Mposaddr
		add	Iposaddr, #4
		movd	(QuadratureEncoder.ipos), Iposaddr
		movd	(QuadratureEncoder.ipos2), Iposaddr
		add	Mposaddr, #4
QuadratureEncoder.next
		djnz	Idx, #QuadratureEncoder.updatepos
		jmp	#QuadratureEncoder.sample
Amask
		.long	$55555555
Bmask
		.long	$aaaaaaaa
Msb
		.long	$80000000
Pos             .res 1                        
Pin
		.res	1
Totenc
		.res	1
Idx
		.res	1
St1
		.res	1
St2
		.res	1
T1
		.res	1
T2
		.res	1
Diff
		.res	1
Iposaddr
		.res	1
Mposaddr
		.res	1
Intpos
		.res	$10
                        
                .compress default