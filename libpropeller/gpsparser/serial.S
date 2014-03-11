		.section .serial.cog, "ax"
		.global Fds_entry //Line comment
		.compress off
..start
		.org	0
		//----- Setup -----
Fds_entry // Counter A's job is to output data on the TX pin
				neg		PHSA, #1
				mov		CTRA, Ctra_val
				mov		DIRA, Masktx   // may be 0 if the TX pin is undefined
		
				// Counter B's job is to count how many clocks the RX pin is low
				// (specifically for aligning the read to the start bit).  Note:
				// it increments by 2...I want to compare 1/4 the bit period, so
				// instead of dividing the 1/2 bit period by 2, I just increment
				// the logic counter by 2.
		
				mov		CTRB, Ctrb_val
				mov		FRQB, #2
		
				//  period info: load in 1/2 bit period
				rdlong	Step_clocks, Period_ptr
		
				// let the caller know it's OK to proceed (store a 0)
				wrlong	OUTB, PAR
		
				//  start looping
				mov		Timestamp, Step_clocks
				add		Timestamp, CNT
		
/*----- Receive -----

Notes:
* Any hub ops in RX must have a jmpret within 2 instructions: rdbyte nop jmpret
*/
		
Rx_cleanup //  resume after setting my start-bit-edge-detector to 0 
				mov		PHSB, #0
Rx_main
        //  loop (while calling TX co-routines) until we have a start bit
				jmpret	Lockstep_ret, Tx_jump
				mov		Bits_in, PHSB	wz
	if_z 		jmp		#Rx_main
	
				// We have a start bit!              
				// Sync up with the correct place to sample the next bit:
				// If RX was low for a short time (< 1/4 period), wait an
				// extra 1/2 bit (places us before the last 1/4).
				jmpret	Lockstep_ret, Tx_jump
				cmp		Bits_in, Step_clocks	wz, wc
	if_b 		jmpret	Lockstep_ret, Tx_jump
	
				// OK, we have 8 data bits left to read
				mov		Bits_in, #8
		
				// read in 8 bits
Rx_main.get_bit
				jmpret	Lockstep_ret, Tx_jump
				test	Maskrx, INA	wc
				rcr		Data_in, #1
				jmpret	Lockstep_ret, Tx_jump
				djnz	Bits_in, #Rx_main.get_bit
		
				// align the data
				shr		Data_in, #(32 - 8)
		
				// read and verify the stop bit
				jmpret	Lockstep_ret, Tx_jump
				test	Maskrx, INA	wc
	if_nc 		jmp		#Rx_cleanup
	
				// save the byte, increment the pointer
				add		Rx_ptr, Rx_head_ptr
				wrbyte	Data_in, Rx_ptr
				add		Rx_ptr, #1
				jmpret	Lockstep_ret, Tx_jump
		
				// wrap the pointer and re-adjust back to 0-based index
				cmpsub	Rx_ptr, Rx_end_ptr	wc
	if_nc		sub		Rx_ptr, Rx_head_ptr
	
				// update the head pointer
				wrword	Rx_ptr, Update_head_ptr
				tjz		PHSB, #Rx_main	wr
		
/*----- Sync -----*/
Tx_jump // This is the jmpret register that handles switching to the right TX co-routine
				jmp		#Tx_main
Lockstep // This routine cycles through parts of the TX routine, and ends always locked in step
				waitcnt	Timestamp, Step_clocks
				// send out another TX bit (if we have any) right after the lockstep
				ror		PHSA, Write_bit
Lockstep_ret
				// return back to the RX code, also always a fixed time after the lockstep 
				jmp		#(0 - 0)

/*----- Transmit -----

Notes:
* Any hub ops in TX must have a jmpret within 3 instructions: rdbyte nop nop jmpret
* There is no guarantee that a TX co-routine is in sync
* So bits are clocked out in "lockstep".
*/
Tx_main // check for outgoing data
				rdlong	Bytes_out, PAR
				jmpret	Tx_jump, #Lockstep
				//  I need some free time to adjust the clock rate
				rdlong	Step_clocks, Period_ptr
				jmpret	Tx_jump, #Lockstep
				// loop if we have no data to send out
				tjz		Bytes_out, #Tx_main

				// Bottom 16 bits hold the buffer address
				// (no need to mask, prop does it for us)
				mov		Write_ptr, Bytes_out

				// Top 16 bits hold the buffer length - 1
				shr		Bytes_out, #16
				add		Bytes_out, #1

				jmpret	Tx_jump, #Lockstep
		
Tx_main.tx_byte
				// set up for sending out a byte
				rdbyte	Tmp, Write_ptr
				add		Write_ptr, #1

				// force the stop bit
				or		Tmp, #$100

				jmpret	Tx_jump, #Lockstep
				
				//SRLM: Add CTS support: If high, don't transmit byte (ie, wait).
				mov		INA, INA
				and		INA, Maskcts wz, nr
	if_nz		jmp		#Lockstep

				// sign extend the 1 into all upper bits
				shl		Tmp, #(32-9)
				sar		Tmp, #(32-10)
				mov		PHSA, Tmp

				// 10 bits (start + 8 data + stop) makes 20 half-bits
				mov		Half_bits_out, #20

Tx_main.tx_bit // wait 1/2 a bit, toggle the write_bit bit
				jmpret	Tx_jump, #Lockstep
				xor		Write_bit, #1
				djnz	Half_bits_out, #Tx_main.tx_bit

				jmpret	Tx_jump, #Lockstep

				// go back for more?
				djnz	Bytes_out, #Tx_main.tx_byte
				wrlong	Bytes_out, PAR
				jmpret	Tx_jump, #Lockstep
				jmp		#(Tx_main + 1)
		
		
/*----- ASM parameters -----*/
Period_ptr
		.global Period_ptr
		.long	0
Maskrx
		.global Maskrx
		.long	0		
Masktx
		.global Masktx
		.long	0
Rx_head_ptr
		.global Rx_head_ptr
		.long	0
Rx_end_ptr
		.global Rx_end_ptr
		.long	0
Rx_ptr
		.global Rx_ptr
		.long	0
Ctra_val
		.global Ctra_val
		.long	0
Ctrb_val
		.global Ctrb_val
		.long	0
Update_head_ptr
		.global Update_head_ptr
		.long	0
Maskcts //Input to propeller
		.global Maskcts
		.long	0
Write_bit
		.long	0
		
/*----- ASM local variables -----*/
Tmp
		.res	1
Step_clocks
		.res	1
Bits_in
		.res	1
Half_bits_out
		.res	1
Bytes_out
		.res	1
Data_in
		.res	1
Timestamp
		.res	1
Write_ptr
		.res	1
		//  make sure we fit inside the 512-16 instruction / variable limit
		.fit	$1f1
		.compress default
