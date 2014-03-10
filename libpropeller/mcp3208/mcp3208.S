                .section .mcp3208.cog, "ax"
                .global MCP3208_Entry
                .compress off

MCP3208_Entry
..start
		.org	0
		mov	T1, PAR             // Read parameters

		call	#Param              // Setup DIN/DOUT pin
		mov	Dmask, T2

		call	#Param              // Setup CLK pin
		mov	Cmask, T2

		call	#Param              // Setup CS Pin
		mov	Smask, T2

		call	#Param              // Set mode
		mov	Enables, T3

		call	#Param              // Setup DAC configuration
	if_c 	or	DIRA, T2
	if_c 	movs	CTRA, T3
	if_c 	movi	CTRA, #0b00110000

		shr	T3, #8
		call	#Param2
	if_c 	or	DIRA, T2
	if_c 	movs	CTRB, T3
	if_c 	movi	CTRB, #0b0011000


//
//
// Perform Conversions Continuously
//
		or	DIRA, Cmask         // Output CLK
		or	DIRA, Smask         // Output CS

Main_loop
		mov	Command, #$10       // Init command
		mov	T1, PAR             // Reset sample pointer
		mov	T2, Enables         // Get enables
		mov	T3, #8              // Ready 8 channels

Cloop
		shr	T2, #1	wc          // If channel disabled, skip
	if_nc 	jmp	#Skip

		test	T2, #$80	wc  // Channel enabled, get single/diff mode
		muxnc	Command, #8
		mov	Stream, Command

		or	OUTA, Smask         // CS high
		or	DIRA, Dmask         // make DIN/DOUT output
		mov	Bits, #20           // ready 20 bits (cs+1+diff+ch[3]+0+0+data[12])

Bloop   
		test	Stream, #$20	wc  // update DIN/DOUT
		muxc	OUTA, Dmask

		cmp	Bits, #$e	wz  // if command done, input DIN/DOUT
	if_z 	andn	DIRA, Dmask

		andn	OUTA, Cmask         // CLK low
		mov	T4, PAR             // update DACs between clock transitions
		add	T4, #$14
		rdlong	FRQA, T4
		add	T4, #4
		rdlong	FRQB, T4
		or	OUTA, Cmask         // CLK high

		test	Dmask, INA	wc  // Sample DIN/DOUT
		rcl	Stream, #1

		andn	OUTA, Smask         // CS Low

		djnz	Bits, #Bloop        // Next data bit


		and	Stream, Mask12      // Trim and write sample
		wrword	Stream, T1

Skip
		add	T1, #2              // Advance sample pointer
		add	Command, #1         // Advance command
		djnz	T3, #Cloop          // more channels?

		wrlong	Counter, T1         // Channels done, update counter
		add	Counter, #1

		jmp	#Main_loop          // perform conversions again


//
//
// Get parameter
//
Param
		rdlong	T3, T1              // Get parameter into T3
		add	T1, #4              // Point to next parameter

Param2
		mov	T2, #1              // Make pin mask in T2
		shl	T2, T3

		test	T3, #$80	wc  // Get DAC flag into c

Param2_ret
Param_ret
		ret


Dlsb        .long	(1 << 9)
Mask12      .long	$fff
T1          .res	1
T2          .res	1
T3          .res	1
T4          .res	1
Dmask       .res	1
Cmask       .res	1
Smask       .res	1
Enables     .res	1
Command     .res	1
Stream      .res	1
Bits        .res	1
Counter     .res	1

                .compress default