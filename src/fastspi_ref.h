#pragma once

/// @file fastspi_ref.h
/// Example of a hardware SPI support class.
/// @note Example for developers. Not a functional part of the library.

#ifndef __INC_FASTSPI_ARM_SAM_H
#define __INC_FASTSPI_ARM_SAM_H

#if FASTLED_DOXYGEN // guard against the arduino ide idiotically including every header file
#include "FastLED.h"
#include "fl/int.h"

FASTLED_NAMESPACE_BEGIN

/// A skeletal implementation of hardware SPI support.  Fill in the necessary code for init, waiting, and writing.  The rest of
/// the method implementations should provide a starting point, even if they're not the most efficient to start with
template <fl::u8 _DATA_PIN, fl::u8 _CLOCK_PIN, fl::u32 _SPI_CLOCK_DIVIDER>
class REFHardwareSPIOutput {
	Selectable *m_pSelect;

public:
	/// Default Constructor
	SAMHardwareSPIOutput() { m_pSelect = NULL; }

	/// Constructor with selectable
	SAMHArdwareSPIOutput(Selectable *pSelect) { m_pSelect = pSelect; }

	/// set the object representing the selectable
	void setSelect(Selectable *pSelect) { /* TODO */ }

	/// initialize the SPI subssytem
	void init() { /* TODO */ }

	/// latch the CS select
	void inline select() __attribute__((always_inline)) { if(m_pSelect != NULL) { m_pSelect->select(); } }

	/// release the CS select
	void inline release() __attribute__((always_inline)) { if(m_pSelect != NULL) { m_pSelect->release(); } }

	/// wait until all queued up data has been written
	static void waitFully() { /* TODO */ }

	/// write a byte out via SPI (returns immediately on writing register)
	static void writeByte(fl::u8 b) { /* TODO */ }

	/// write a word out via SPI (returns immediately on writing register)
	static void writeWord(uint16_t w) { /* TODO */ }

	/// A raw set of writing byte values, assumes setup/init/waiting done elsewhere
	static void writeBytesValueRaw(fl::u8 value, int len) {
		while(len--) { writeByte(value); }
	}

	/// A full cycle of writing a value for len bytes, including select, release, and waiting
	void writeBytesValue(fl::u8 value, int len) {
		select(); writeBytesValueRaw(value, len); release();
	}

	/// A full cycle of writing a value for len bytes, including select, release, and waiting
	template <class D> void writeBytes(FASTLED_REGISTER fl::u8 *data, int len) {
		fl::u8 *end = data + len;
		select();
		// could be optimized to write 16bit words out instead of 8bit bytes
		while(data != end) {
			writeByte(D::adjust(*data++));
		}
		D::postBlock(len);
		waitFully();
		release();
	}

	/// A full cycle of writing a value for len bytes, including select, release, and waiting
	void writeBytes(FASTLED_REGISTER fl::u8 *data, int len) { writeBytes<DATA_NOP>(data, len); }

	/// write a single bit out, which bit from the passed in byte is determined by template parameter
	template <fl::u8 BIT> inline static void writeBit(fl::u8 b) { /* TODO */ }

	/// write a block of uint8_ts out in groups of three.  len is the total number of uint8_ts to write out.  The template
	/// parameters indicate how many uint8_ts to skip at the beginning and/or end of each grouping
	template <fl::u8 FLAGS, class D, EOrder RGB_ORDER> void writePixels(PixelController<RGB_ORDER> pixels, void* context = NULL) {
		select();
		while(data != end) {
			if(FLAGS & FLAG_START_BIT) {
				writeBit<0>(1);
			}
			writeByte(D::adjust(pixels.loadAndScale0()));
			writeByte(D::adjust(pixels.loadAndScale1()));
			writeByte(D::adjust(pixels.loadAndScale2()));

			pixels.advanceData();
			pixels.stepDithering();
			data += (3+skip);
		}
		D::postBlock(len);
		release();
	}

};

FASTLED_NAMESPACE_END

#endif

#endif
