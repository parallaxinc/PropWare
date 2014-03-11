                .section .pwm2.cog, "ax"
                .global PWM2_Entry
                .compress off

PWM2_Entry
..start
		.org	0
		mov	T1, PAR
		rdlong	Period, T1
		mov	Periodaddr, T1
		add	T1, #4
		mov	Dutyaaddr, T1
		add	T1, #4
		mov	Pinaaddr, T1
		add	T1, #4
		mov	Ctravaladdr, T1
		add	T1, #4
		mov	Dutybaddr, T1
		add	T1, #4
		mov	Pinbaddr, T1
		add	T1, #4
		mov	Ctrbvaladdr, T1
		mov	FRQA, #1
		mov	FRQB, #1
		mov	Nextcyclestart, CNT
		add	Nextcyclestart, Period
PWM2_Entry.loop
		rdlong	Pina, Pinaaddr
		mov	T1, Pina
		rdlong	Pinb, Pinbaddr
		or	T1, Pinb
		mov	DIRA, T1
		mov	T1, Ctraval
		rdlong	Ctraval, Ctravaladdr
		cmp	T1, Ctraval	wz
	if_nz 	mov	CTRA, Ctraval
		mov	T1, Ctrbval
		rdlong	Ctrbval, Ctrbvaladdr
		cmp	T1, Ctrbval	wz
	if_nz 	mov	CTRB, Ctrbval
		rdlong	Dutya, Dutyaaddr
		rdlong	Dutyb, Dutybaddr
		rdlong	Period, Periodaddr
		waitcnt	Nextcyclestart, Period
		neg	PHSA, Dutya
		neg	PHSB, Dutyb
		jmp	#PWM2_Entry.loop
Ctraval
		.long	0
Ctrbval
		.long	0
Dutya
		.res	1
Dutyb
		.res	1
Dutyaaddr
		.res	1
Dutybaddr
		.res	1
Periodaddr
		.res	1
Pina
		.res	1
Pinb
		.res	1
Pinaaddr
		.res	1
Pinbaddr
		.res	1
Ctravaladdr
		.res	1
Ctrbvaladdr
		.res	1
Period
		.res	1
T1
		.res	1
Nextcyclestart
		.res	1

                .compress default
