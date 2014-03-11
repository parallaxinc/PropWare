                .section .pulsewidthreader.cog, "ax"
                .global PulseWidthReader_Entry
                .compress off
                

PulseWidthReader_Entry
..start
		.org	0

		mov	Mask, PAR           // Read Mask data from main RAM
		rdlong	Mask, Mask

		mov	Offset, PAR         // Reset main RAM to '0'
		mov	Temp, #0
		mov	Index, Mask
Init
		shr	Index, #1	wz, wc
	if_c 	wrlong	Temp, Offset        // Set longs of main RAM to 0 for every monitored pin.
	if_c 	add	Offset, #4  
	if_c 	wrlong	Temp, Offset
	if_c 	add	Offset, #4
	if_nz 	jmp	#Init

		mov	Unchanged, Mask     // Set unchanged for all monitored pins 

		mov	State, INA          // Recard initial states and keep a reference.
		mov	Original, State

Waitchange
		mov	Laststate, State    // Remember last state
		waitpne	Laststate, Mask     // Wait for any monitored pin to chaneg state. Ignore non-watched pins.
		mov	State, INA
		and	State, Mask

		mov	Time, CNT           // Update timer

		mov	First, Original     // Check for initial pin transitions, record them, then remove them from Unchanged.
		xor	First, State
		and	First, Unchanged
		xor	Unchanged, First

Decode
		mov	Index, Neg1         // Reset pin counter

		mov	Newstate, State     // State and Mask must be preserved
		mov	Temp, Mask

Again
		shr	Temp, #1	wz, wc  // Try next pin
	if_z_and_nc 	jmp	#Waitchange     // Last monitored pin?
	if_nc 	jmp	#Done                   // Is pin being monitored?

		add	Index, #1               // Increment counter

		test	Laststate, #1	wz      // z = !LastState.0
		test	Newstate, #1	wc      // c = NewState.0
	if_z_ne_c 	jmp	#Done           // If there was no change, check the next pin

		mov	Offset, Index           // Point Offset to the appropriate LastRecord
		add	Offset, #((Lastrecord-..start)/4)

		movs	Readlastrecord, Offset
		movd	Writelastrecord, Offset

		mov	Deltatime, Time         // Subtract old record from current time
Readlastrecord
		sub	Deltatime, Lastrecord

Writelastrecord
		mov	Lastrecord, Time    // Write back current time

		mov	Offset, Index       // Point Offset to the proper RAM location
		shl	Offset, #3
	if_z_and_c 	add	Offset, #4  // If falling edge, store in the previous word
		add	Offset, PAR
		test	First, #1	wz  // Ignore first transition
	if_z 	wrlong	Deltatime, Offset   // Write the appropriate PulseData variable

Done
		shr	Laststate, #1       // Queue in the next pins
		shr	Newstate, #1
		shr	First, #1
		jmp	#Again

Neg1            .long	(-1)
Mask            .res	1                   // Pins that are being monitored

Unchanged       .res	1                   // Pins that have never changed state
Original        .res	1                   // Original state of monitored pins
First           .res	1                   // Pins that just had their first transition

Laststate       .res	1                   // Last state of monitored pins
State           .res	1                   // Current state of monitored pins
Newstate        .res	1                   // Working register to keep State unmodified
Lastrecord      .res	32                 // CNT values for last transition of each pin
Time            .res	1                   // CNT value for most recent transition
Deltatime       .res	1                   // Difference between Time and LastRecord

Offset          .res	1                   // Temporary and working registers
Index           .res	1
Temp            .res	1

                .compress default
