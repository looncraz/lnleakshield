/*
	A little inferential hackery for the Aquacomputer Leakshield
	Lots of data in the packet (/dev/hidraw#)
*/

#pragma once

#define LEAKSHIELD_DATA_SIZE 820

struct LeakshieldData {
	big_t<8>	cookie;	// assumed.. 72167580647354627, seems stagnant
	big_t<1>	dunno0[14];
	big_t<2>	incby5;	// no idea why, but this just goes up by 5/sec
	big_t<1>	dunno4[7];
	big_t<4>	something; // index 31-34, inc quickly... then not at all... WTF (33... by 10?)
	big_t<4>	time;	// of some sort, 35~39, unknown epoch? seconds only, but overflows left
	big_t<1>	tick;	// one second tick? why?

	big_t<1>	dunno1[11];
	big_t<2>	beatsMe; //  varies around -16115_i / 49400_u
	big_t<2>	beatsMe2; // Assuming 16-bit value, but only ever 10/11
	big_t<2>	beatsMe3; // Assuming 16-bit value
	big_t<2>	beatsMe4; // Assuming 16-bit value
	big_t<1>	dunno8[5];
	big_t<2>	beatsMe5; // Assuming 16-bit value, 67, 68
	big_t<1>	dunno10[15];

	big_t<2>	volChange0; // This approximates 307,308, sometimes out of sync
	big_t<2>	beatsMe6; // changes slowly
	big_t<1>	dunno11[2]; // doesn't seem to change
	big_t<2>	beatsMe7;
	big_t<1>	dunno12[2]; // doesn't seem to change
	big_t<2>	beatsMe8;

	big_t<2>	beatsMe9;
	big_t<2>	beatsMe10;
	big_t<2>	beatsMe11;
	big_t<1>	dunno9[176];

//		big_t<1>	dunno9[190];

	// Theres bound to be some goodness in dunno1

	big_t<2>	press1;	// 275, 276
	big_t<2>	dunno2;	// always zero
	big_t<2>	press2;
	big_t<2>	pumpAverage;
	big_t<2>	correction; // signed pressure value (offset for atmosphere)
	big_t<2>	adjusted; // 285, 286
		big_t<2>	temperature2; // 287, 288
	big_t<1>	dunno5[16];
	big_t<2>	pressChange;	// 305, 306 mbar/min
	big_t<2>	volChange;	// 307, 308, ml/h
	big_t<1>	dunno6[2];
	big_t<2>	reservoirFilled; // in ml
	big_t<2>	reservoirVolume; // in ml, position 313

	// Most of what follows in the packet appears to be a log of some sort

	void PrintToStream()
	{
		printf("\tCookie       : %lu\n", 			cookie.UInt64());
		printf("\tWTF? (+5)    : %i\n", 			incby5.UInt16());
	printf("       *Something    : %lu\n", 			something.UInt64());
		printf("\tTime         : %lu\n", 			time.UInt64());
		printf("\tTick         : %i\n", 			tick.UInt32());

	/*
		Most of these are seemingly pressure related, most jump to 32767 or something when the
		pump is active and then start gaining their typical value ranges
	*/

	printf("       *Beats me     : %i\n", 			beatsMe.Int32());
	printf("       *Beats me 2   : %i\n", 			beatsMe2.Int32());
	printf("       *Beats me 3   : %i\n", 			beatsMe3.Int32());
	printf("       *Beats me 4   : %i\n", 			beatsMe4.Int32());
	printf("       *Beats me 5   : %i\n", 			beatsMe5.Int32());
	printf("       *Beats me 6   : %i\n", 			beatsMe6.Int32());
	printf("       *Beats me 7   : %i\n", 			beatsMe7.Int32());
	printf("       *Beats me 8   : %i\n", 			beatsMe8.Int32());
	printf("       *Beats me 9   : %i\n", 			beatsMe9.Int32());
	printf("       *Beats me 10  : %i\n", 			beatsMe10.Int32());
	printf("       *Beats me 11  : %i\n", 			beatsMe11.Int32());

		printf("\tPress 1      : %.2f mbar\n", 	(float)press1.UInt32() / 100);
		printf("\tPress 2      : %.2f mbar\n", 	(float)press2.UInt32() / 100);
		printf("\tAverage      : %.1f  mbar\n", 	(float)pumpAverage.Int32() / 10);
		printf("\tCorrection   : %.1f  mbar\n", 	(float)correction.Int32() / 10);
		printf("\tAdjusted     : %.1f  mbar\n", 	(float)adjusted.Int32() / 10);

		printf("\tTemperature  : %.2f C\n", 			(float)temperature2.UInt32() / 100);

		printf("\tPress Change : %4.1f ml/min\n",	 (float)pressChange.Int16() / 10);
		printf("\tVolume change: %4i ml/hour\n",	 volChange.Int16());
		printf("\tVolume change: %4i ml/hour (two entries)\n",	 volChange0.Int16());

		printf("\tFilled       : %i ml\n", 		reservoirFilled.Int16());
		printf("\tReservoir    : %i ml\n", 		reservoirVolume.Int16());
	}
};
