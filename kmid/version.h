#ifndef _VERSION_H


#define VERSION_NUM 0.5
#define VERSION_SHORTTXT "0.5"
#define VERSION_TXT "KMid 0.5" 

// Undefine WORKING for release versions
//#define WORKING

// I want to keep some special variables defined at home for personal 
// adjustments
//#define AT_HOME


// Define MODE_DEMO_ONLYVISUAL for kmid to work on systems with no soundcard,
// or when it is not well configured. You will only get visual feedback, that
// is, you will see the letter changing of colors, but no music (of course:-)
//#define MODE_DEMO_ONLYVISUAL


// And now, DEBUG messages :
#ifdef WORKING
//#define KMidDEBUG
//#define KDISPTEXTDEBUG
//#define COLLECTDLGDEBUG
//#define FMOUTDEBUG
//#define MIDFILEDEBUG
//#define MIDIOUTDEBUG
//#define MIDISTATDEBUG
//#define SYNTHOUTDEBUG
//#define VOICEMANDEBUG
#endif

#endif
