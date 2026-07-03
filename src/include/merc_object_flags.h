#ifndef MERC_OBJECT_FLAGS_H
#define MERC_OBJECT_FLAGS_H
/*
 * merc_object_flags.h -- object item-type and item/wear/extra flag constants
 * extracted from merc.h (#75). Pure ITEM_ #defines; no behavior change.
 * Included by merc.h.
 */

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define	ITEM_PORTAL		     27
#define	ITEM_GATE		     28
#define	ITEM_SUIT		     29
#define	ITEM_MUNITION		     30
#define	ITEM_ACCESSORY		     31
#define	ITEM_MATERIA		     32


/*
 * Weapon flags.
 * Added in game, goes in v0
 */
#define	WEAP_RUNE_AIR			1
#define	WEAP_RUNE_EARTH			2
#define	WEAP_RUNE_FIRE			4
#define	WEAP_RUNE_WATER			8
#define	WEAP_RUNE_ENERGY		16
#define	WEAP_RUNE_NEGATIVE		32

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1
#define ITEM_HUM		      2
#define ITEM_DARK		      4
#define ITEM_LOCK		      8
#define ITEM_EVIL		     16
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define	ITEM_NO_LOCATE		  16384
#define	ITEM_UNIQUE		  32768
#define	ITEM_ASTRAL		  65536
#define	ITEM_NOLOOT		 131072
#define	ITEM_HARDENED		 262144


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		      1
#define ITEM_WEAR_FINGER	      2
#define ITEM_WEAR_NECK		      4
#define ITEM_WEAR_BODY		      8
#define ITEM_WEAR_HEAD		     16
#define ITEM_WEAR_LEGS		     32
#define ITEM_WEAR_FEET		     64
#define ITEM_WEAR_HANDS		    128 
#define ITEM_WEAR_ARMS		    256
#define ITEM_WEAR_SHIELD	    512
#define ITEM_WEAR_ABOUT		   1024 
#define ITEM_WEAR_WAIST		   2048
#define ITEM_WEAR_WRIST		   4096
#define ITEM_WIELD		   8192
#define ITEM_HOLD		  16384

#endif
