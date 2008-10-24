
//static int field
#ifndef INT
#define INT(v)
#endif

//dynamic int field
#ifndef INT_DYN
#define INT_DYN(v)
#endif

//static real field
#ifndef REAL
#define REAL(v)
#endif

//dynamic real field
#ifndef REAL_DYN
#define REAL_DYN(v)
#endif

INT_DYN(accessoryID)
INT_DYN(beardID)
INT(costumeID)
INT_DYN(decoID)
INT(eyebrowsID)
INT(eyesID)
INT(hairID)
INT_DYN(lipstickID)
INT_DYN(makeupID)
INT(skinID)
INT(backgroundID)
INT(lightID)
	
REAL(headRotation1)
REAL(headRotation2)
REAL(headRotation3)

REAL(eyeRotation1)
REAL(eyeRotation2)
REAL(eyeRotation3)

REAL(camPos1)
REAL(camPos2)
REAL(camPos3)

REAL_DYN(morph1e)
REAL_DYN(morph1n)
REAL_DYN(morph1s)
REAL_DYN(morph1w)
REAL_DYN(morph2e)
REAL_DYN(morph2n)
REAL_DYN(morph2s)
REAL_DYN(morph2w)
REAL_DYN(morph3e)
REAL_DYN(morph3n)
REAL_DYN(morph3s)
REAL_DYN(morph3w)
REAL_DYN(morph4e)
REAL_DYN(morph4n)
REAL_DYN(morph4s)
REAL_DYN(morph4w)

#undef INT
#undef INT_DYN
#undef REAL
#undef REAL_DYN

