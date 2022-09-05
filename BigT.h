/*
	(C) 2022 looncraz <looncraz@looncraz.net>
	MIT License, really I don't care.. if you find a bug (you will) or improve it, I'd not mind an
	email :p

	Warning: This shit ain't thoroughly tested at all, I wrote in 30 minutes and found no bugs
	first try... that's BAD, not a brag!
*/

#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <string>

enum BigT_ByteSize {
	BIG_16 = 2,
	BIG_32 = 4,
	BIG_64 = 8,
	BIG_FLOAT = 4,
	BIG_DOUBLE = 8
};

// The data given to this is always assumed to be big_endian
// If necessary, such as on x86, the conversions to little_endian will occur
// byteSize  - move semantics not supported.
template <std::size_t byteSize>
class big_t {
public:
			big_t	();
			// No swapping!  Bytes are used as RAW!
			big_t	(std::uint8_t);
			big_t	(std::uint16_t);
			big_t	(std::uint32_t);
			big_t	(std::uint64_t);
			big_t	(float);
			big_t	(double);
			big_t	(void* raw); // must be >= byteSize!

			template<std::size_t Size2>
			big_t	(const big_t<Size2>& other);	// allow any size!

			// TODO: move/copy ctors!

			~big_t	(){}

			std::int16_t	Int16()		const;
			std::int32_t	Int32()		const;
			std::int64_t	Int64()		const;
			std::uint16_t	UInt16()	const;
			std::uint32_t	UInt32()	const;
			std::uint64_t	UInt64()	const;
			float			Float()		const;
			double			Double()	const;

			void*			Raw()		const {return &fData[0];}

			int 			BitSize()	const {return byteSize * 8;}
			std::size_t		Size()		const {return byteSize;};

			void			PrintToStream(FILE* = stdout) const;

	/* 			Operators are life!  		*/

		// Assignment
		big_t<byteSize>&	operator = (std::uint8_t);
		big_t<byteSize>&	operator = (std::uint16_t);
		big_t<byteSize>&	operator = (std::uint32_t);
		big_t<byteSize>&	operator = (std::uint64_t);
		big_t<byteSize>&	operator = (float);
		big_t<byteSize>&	operator = (double);
		big_t<byteSize>&	operator = (void*);
		template<std::size_t Size2>
		big_t<byteSize>&  	operator = (const big_t<Size2>& other);	// allow any size!

private:
		std::uint8_t		fData[byteSize];
};


#pragma mark Implementation------------------------------------------------------------------------

// We support 1~8 bytes in length only (yes, 8, 16, 24, 32, 40, 48, 56, & 64 bit support!)
#define BIG_T_BASE_INIT 	static_assert(byteSize > 0, "big_t cannot be zero bytes in size!");	\
	static_assert(byteSize <= 8, "big_t cannot be more than 8 bytes in size!");

// Constructors - the real work is done in the assignment operators
template <std::size_t byteSize>	big_t<byteSize>::big_t	() { BIG_T_BASE_INIT; }
#define BIG_T_BASE_CON(T) template <std::size_t byteSize>	big_t<byteSize>::big_t	(T in)	\
	{	BIG_T_BASE_INIT;	*this = in; }
BIG_T_BASE_CON(std::uint8_t);
BIG_T_BASE_CON(std::uint16_t);
BIG_T_BASE_CON(std::uint32_t);
BIG_T_BASE_CON(std::uint64_t);
BIG_T_BASE_CON(float);
BIG_T_BASE_CON(double);
BIG_T_BASE_CON(void*);

// now ain't that some shit?
template <std::size_t byteSize>
template <std::size_t Size2>
big_t<byteSize>::big_t	(const big_t<Size2>& in)
{
	BIG_T_BASE_INIT;
	*this = in;
}

		// Assignment

// Assignments are assumed to be from casting to a big-endian buffer, nothing is swapped on assignment!
#define BIG_T_ASSIGNMENT(T) template <std::size_t byteSize>	big_t<byteSize>& \
	big_t<byteSize>::operator=(T in) { 	memset(&fData[0], 0, byteSize);	\
	memcpy(&fData[0], &in, std::min(byteSize, sizeof(in)));	return *this; }

BIG_T_ASSIGNMENT(uint8_t);
BIG_T_ASSIGNMENT(uint16_t);
BIG_T_ASSIGNMENT(uint32_t);
BIG_T_ASSIGNMENT(uint64_t);
BIG_T_ASSIGNMENT(float);
BIG_T_ASSIGNMENT(double);
//	Allow conversion from any size to any size big_t
template<std::size_t byteSize>
template<std::size_t Size2>
big_t<byteSize>&
big_t<byteSize>::operator = (const big_t<Size2>& other)
{
	memset(&fData[0], 0, byteSize);
	memcpy(&fData[0], &other.fData[0], std::min(byteSize, Size2));

	return *this;
}



// Swap out-conversion.. crude, probably slow, but it works, so fuck it...


#define BIG_OUT_CONV(N, T) template <std::size_t byteSize>	T \
big_t<byteSize>:: N ()		const \
{	uint8_t result[sizeof(T)];			\
	memset(&result[0], 0, sizeof(result));	\
	int pos = sizeof(result) - 1;	\
	for (int i = 0; i < byteSize && pos >= 0; i++, pos--)	\
		result[pos] = fData[i];	\
		\
	int shift = (sizeof(T) - byteSize) * 8;	\
	if (shift < 0) shift = 0;	\
	\
	return (*(T*)&result[0]) >> shift; \
}

BIG_OUT_CONV(Int16, std::int16_t);
BIG_OUT_CONV(Int32, std::int32_t);
BIG_OUT_CONV(Int64, std::int64_t);
BIG_OUT_CONV(UInt16, std::uint16_t);
BIG_OUT_CONV(UInt32, std::uint32_t);
BIG_OUT_CONV(UInt64, std::uint64_t);

template <std::size_t byteSize>
float
big_t<byteSize>::Float ()		const
{
	uint8_t result[sizeof(float)];
	memset(&result[0], 0, sizeof(result));
	int pos = sizeof(result) - 1;
	for (int i = 0; i < byteSize && pos >= 0; i++, pos--)
		result[pos] = fData[i];

	int shift = (sizeof(float) - byteSize) * 8;
	if (shift < 0) shift = 0;

	return (float)((*(uint32_t*)&result[0]) >> shift);
}

template <std::size_t byteSize>
double
big_t<byteSize>::Double ()		const
{
	uint8_t result[sizeof(double)];
	memset(&result[0], 0, sizeof(result));
	int pos = sizeof(result) - 1;
	for (int i = 0; i < byteSize && pos >= 0; i++, pos--)
		result[pos] = fData[i];

	int shift = (sizeof(double) - byteSize) * 8;
	if (shift < 0) shift = 0;

	return (double)((*(uint64_t*)&result[0]) >> shift);
}


template <std::size_t byteSize> void
big_t<byteSize>::PrintToStream (FILE* f) const
{
	fprintf(f, "big_t<%lu> {\n", byteSize);
	fprintf(f, "\tSigned  : %li\n", Int64());
	fprintf(f, "\tUnsigned: %li\n", UInt64());
	fprintf(f, "\tData    : ");
	for (int i = 0; i < byteSize; ++i)
		fprintf(f, "%i ", fData[i]);

	fprintf(f, "\n}\n");
}
