//Warning: To change the buffer size you must change it here and in the .h file
#define buffersize 256
#define buffermask (buffersize - 1)

#define Local 523052

                .section .full_duplex_serial.cog, "ax"
                .global FullDuplexSerial_Entry
		.compress off
                .equ	Bufsiz, buffersize
		.equ	Bufmsk, buffermask

//
//
// Entry
//
FullDuplexSerial_Entry

..start
		.org	0
		mov	T1Local, PAR                 //'get structure address
		add	T1Local, #(4 << 2)           //'skip past heads and tails

		rdlong	T2Local, T1Local                  //'get rx_pin
		mov	Rxmask, #1
		shl	Rxmask, T2Local

		add	T1Local, #4                  //'get tx_pin
		rdlong	T2Local, T1Local
		mov	Txmask, #1
		shl	Txmask, T2Local

		add	T1Local, #4                  //'get rxtx_mode
		rdlong	Rxtxmode, T1Local

		add	T1Local, #4                  //'get bit_ticks
		rdlong	Bitticks, T1Local

		add	T1Local, #4                  //'get buffer_ptr
		rdlong	Rxbuff, T1Local
		mov	Txbuff, Rxbuff
		add	Txbuff, #Bufsiz

		test	Rxtxmode, #4	wz      //'init tx pin according to mode
		test	Rxtxmode, #2	wc
	if_z_ne_c 	or	OUTA, Txmask
	if_z 	or	DIRA, Txmask

		mov	Txcode, #((Transmit-..start)/4) //'initialize ping-pong multitasking

Receive
		jmpret	Rxcode, Txcode          //'run a chunk of transmit code, then return

		test	Rxtxmode, #1	wz      //'wait for start bit on rx pin
		test	Rxmask, INA	wc
	if_z_eq_c 	jmp	#Receive

		mov	Rxbits, #9              //'ready to receive byte
		mov	Rxcnt, Bitticks
		shr	Rxcnt, #1
		add	Rxcnt, CNT

Receive.bit
		add	Rxcnt, Bitticks         //'ready next bit period

Receive.wait
		jmpret	Rxcode, Txcode          //'run a chuck of transmit code, then return

		mov	T1Local, Rxcnt               //'check if bit receive period done
		sub	T1Local, CNT
		cmps	T1Local, #0	wc
	if_nc 	jmp	#Receive.wait

		test	Rxmask, INA	wc      //'receive bit on rx pin
		rcr	Rxdata, #1
		djnz	Rxbits, #Receive.bit

		shr	Rxdata, #($20 - 9)      //'justify and trim received byte
		and	Rxdata, #$ff
		test	Rxtxmode, #1	wz      //'if rx inverted, invert byte
	if_nz 	xor	Rxdata, #$ff

		rdlong	T2Local, PAR                 //'save received byte and inc head
		add	T2Local, Rxbuff
		wrbyte	Rxdata, T2Local
		sub	T2Local, Rxbuff
		add	T2Local, #1
		and	T2Local, #Bufmsk
		wrlong	T2Local, PAR

		jmp	#Receive                //'byte done, receive next byte
Transmit
		jmpret	Txcode, Rxcode          //'run a chunk of receive code, then return

		mov	T1Local, PAR                 //'check for head <> tail
		add	T1Local, #(2 << 2)
		rdlong	T2Local, T1Local
		add	T1Local, #(1 << 2)
		rdlong	T3, T1Local
		cmp	T2Local, T3	wz
	if_z 	jmp	#Transmit

		add	T3, Txbuff              //'get byte and inc tail
		rdbyte	Txdata, T3
		sub	T3, Txbuff
		add	T3, #1
		and	T3, #Bufmsk
		wrlong	T3, T1Local

		or	Txdata, #$100           //'ready byte to transmit
		shl	Txdata, #2
		or	Txdata, #1
		mov	Txbits, #$b
		mov	Txcnt, CNT

Transmit.bit
		test	Rxtxmode, #4	wz      //'output bit on tx pin according to mode
		test	Rxtxmode, #2	wc
	if_z_and_c 	xor	Txdata, #1
		shr	Txdata, #1	wc
	if_z 	muxc	OUTA, Txmask
	if_nz 	muxnc	DIRA, Txmask
		add	Txcnt, Bitticks         //'ready next cnt

Transmit.wait
		jmpret	Txcode, Rxcode          //'run a chunk of receive code, then return

		mov	T1Local, Txcnt               //'check if bit transmit period done
		sub	T1Local, CNT
		cmps	T1Local, #0	wc
	if_nc 	jmp	#Transmit.wait

		djnz	Txbits, #Transmit.bit   //'another bit to transmit?

    		jmp	#Transmit               //'byte done, transmit next byte


//
//
// Unitialized Data
//
T1Local          .res	1
T2Local          .res	1
T3          .res	1
Rxtxmode    .res	1
Bitticks    .res	1
Rxmask      .res	1
Rxbuff      .res	1
Rxdata      .res	1
Rxbits      .res	1
Rxcnt       .res	1
Rxcode      .res	1
Txmask      .res	1
Txbuff      .res	1
Txdata      .res	1
Txbits      .res	1
Txcnt       .res	1
Txcode      .res	1

            .compress default
