#ifndef MERC_CLASSES_H
#define MERC_CLASSES_H
/*
 * merc_classes.h -- per-class ability/power constants extracted from merc.h (#59).
 * Pure #defines (Saiyan S_, Fist F_, Patryn P_ and RUNE_, Mazoku M_ families),
 * indexing pcdata->powers[] and related. Included by merc.h; no behavior change.
 */

/*
 * Saiyan Powers slots
 */
#define S_POWER			0
#define S_POWER_MAX		1
#define	S_STRENGTH		2
#define S_STRENGTH_MAX		3
#define S_SPEED			4
#define S_SPEED_MAX		5
#define S_AEGIS			6
#define S_AEGIS_MAX		7
#define S_TECH			8

#define S_KIBOLT		1
#define S_KIWAVE		2
#define S_KIBLAST		4
#define S_KIBOMB		8
#define S_KIMOVE		16
#define S_KISENSE		32
#define S_FLIGHT		64
#define S_KAMEHAMEHA		128
#define S_KIKOUHOU		256
#define S_MASENKOUHA		512
#define S_SOLARFIST		1024
#define S_HAWKEYES		2048
#define S_HASHYUKEN		4096
#define S_BATTLESENSE		8192
#define S_ZANZOUKEN		16384
#define S_KAIOUKEN		32768
#define S_KIWALL		65536
#define S_RYUKEN		131072
#define	S_KIAIHOU		262144


/*
 * Fist Powers slots and bits
 */
#define F_KI			0
#define F_KI_MAX		1
#define F_LEARNED		2
#define F_TORSO			3
#define F_ARMS			4
#define F_HANDS			5
#define F_LEGS			6
#define F_DISC			7
#define	F_MASTER		8

#define F_SHINKICK		1300
#define F_JAB			1301
#define F_SPINKICK		1302
#define F_KNEE			1303
#define F_ELBOW			1304
#define F_UPPERCUT		1305
#define F_STOMP			1306
#define F_JUMPKICK		1307
#define F_DEATHTOUCH		1308
#define	F_PALMTHRUST		1309

#define DAM_KIFLAME		1100

/*
 * Patryn bits and such
 */
#define	P_LEARNED		0
#define P_LEARNED_NUM		1
#define P_AIR			2
#define P_EARTH			3
#define P_FIRE			4
#define P_WATER			5
#define P_ENERGY		6
#define P_NEGATIVE		7
#define	P_BITS			8

#define	P_DEFENSES		1

#define TORSO			0
#define LEFTARM			1
#define RIGHTARM		2
#define LEFTLEG			3
#define RIGHTLEG		4

#define DAM_SHOCKSHIELD		1200
#define DAM_LIGHTNING		1201
#define DAM_EARTH		1202
#define DAM_FIRE		1203
#define DAM_WATER		1204
#define DAM_ENERGY		1205
#define DAM_NEGATIVE		1206
/*
 * Runes for Patryns
 */
#define RUNE_NONE		0
#define	RUNE_AIR		1
#define RUNE_EARTH		2
#define RUNE_FIRE		4
#define	RUNE_WATER		8
#define	RUNE_ENERGY		16
#define	RUNE_NEGATIVE		32
#define	RUNE_LIFE		64
#define	RUNE_DEATH		128
#define	RUNE_CREATION		256
#define	RUNE_DESTRUCTION	512
#define	RUNE_PROTECTION		1024
#define RUNE_TRANSFORMATION	2048
#define RUNE_MOVEMENT		4096
#define	RUNE_ALL		8192
#define	RUNE_ABJURATION		16384
/*
 * Bits for Sorcerers
 */
#define SORC_PREP	0
#define	SORC_MYSTIC	1
#define SCHOOL_BLACK	2
#define SCHOOL_EARTH	3
#define SCHOOL_WIND	4
#define SCHOOL_FIRE	5
#define SCHOOL_WATER	6
#define SCHOOL_ASTRAL	7
#define SCHOOL_WHITE	8
#define SORC_SPEC	9

#define DOLPH_ZORK	50
#define RUBYEYE_BLADE	51
#define	LAGUNA_BLADE	52
#define	DAM_FLAME_BREATH	1495


/* Bits for Mazoku */
#define M_LEARNED	0
#define	M_SET		1
#define	M_CTYPE		2
#define	M_CTIME		3
#define	M_EGO           4
#define	M_NIHILISM	5
#define	M_ESSENSE	6
#define	M_MATTER	7
#define	M_ASTRAL	8
#define	M_FOCUS		9

#define	M_HUMAN		1
#define	M_BATTLE	2
#define	M_TRUE		4
#define	M_HANDS		8
#define M_CLAWS		16
#define	M_SPIKES	32
#define	M_BLADES	64
#define	M_TENTACLES	128

#define	M_THIRD		512
#define	M_FOURTH	1024
#define	M_FIFTH		2048
#define	M_SIXTH		4096
#define	M_WINGS		8192
#define	M_EYES		16384
#define	M_TELEPORT	32768
#define	M_CHARGE	65536
#define	M_BLAST		131072
#define	M_BOLT		262144
#define	M_BOMB		524288
#define	M_ASTRIKE	1048576

#endif
