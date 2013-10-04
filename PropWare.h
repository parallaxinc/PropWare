/* File:    PropWare.h
 *
 * Author:  David Zemon
 */

#ifndef PROPWARE_H
#define PROPWARE_H

#ifdef DAREDEVIL
#define checkErrors(x)			x
#else
#define checkErrors(x)			if ((err = x)) return err
#endif

#define SECOND		((unsigned long long) CLKFREQ)
#define MILLISECOND	((unsigned long long) (CLKFREQ / 1000))
#define MICROSECOND ((unsigned long long) (MILLISECOND / 1000))

#define BIT_0				0x00000001
#define BIT_1				0x00000002
#define BIT_2				0x00000004
#define BIT_3				0x00000008
#define BIT_4				0x00000010
#define BIT_5				0x00000020
#define BIT_6				0x00000040
#define BIT_7				0x00000080
#define BIT_8				0x00000100
#define BIT_9				0x00000200
#define BIT_10				0x00000400
#define BIT_11				0x00000800
#define BIT_12				0x00001000
#define BIT_13				0x00002000
#define BIT_14				0x00004000
#define BIT_15				0x00008000
#define BIT_16				0x00010000
#define BIT_17				0x00020000
#define BIT_18				0x00040000
#define BIT_19				0x00080000
#define BIT_20				0x00100000
#define BIT_21				0x00200000
#define BIT_22				0x00400000
#define BIT_23				0x00800000
#define BIT_24				0x01000000
#define BIT_25				0x02000000
#define BIT_26				0x04000000
#define BIT_27				0x08000000
#define BIT_28				0x10000000
#define BIT_29				0x20000000
#define BIT_30				0x40000000
#define BIT_31				0x80000000

#define NIBBLE_0			0xf
#define NIBBLE_1			0xf0
#define NIBBLE_2			0xf00
#define NIBBLE_3			0xf000
#define NIBBLE_4			0xf0000
#define NIBBLE_5			0xf00000
#define NIBBLE_6			0xf000000
#define NIBBLE_7			0xf0000000

#define BYTE_0				0xff
#define BYTE_1				0xff00
#define BYTE_2				0xff0000
#define BYTE_3				0xff000000

#define WORD_0				0xffff
#define WORD_1				0xffff0000

#endif /* PROPWARE_H */
