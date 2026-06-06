/*
 * BuffCRC.h
 *
 *  Created on: 15/08/2010
 *      Author: victor
 */

#ifndef BUFFCRC_H_
#define BUFFCRC_H_

class BuffCRC {
public:
	// TEST
	enum {
		TEST_FIRST  = 0x1967AF38,
		TEST_SECOND = 0xA24A7A5B
	};

	// MEDICAL
	enum {
		MEDICAL_ENHANCE_HEALTH       = 0x98321369,
		MEDICAL_ENHANCE_STRENGTH     = 0x815D85C5,
		MEDICAL_ENHANCE_CONSTITUTION = 0x7F86D2C6,
		MEDICAL_ENHANCE_ACTION       = 0x4BF616E2,
		MEDICAL_ENHANCE_QUICKNESS    = 0x71B5C842,
		MEDICAL_ENHANCE_STAMINA      = 0xED0040D9,
		MEDICAL_ENHANCE_POISON       = 0x391AC375,
		MEDICAL_ENHANCE_DISEASE      = 0x3595876
	};

	// PERFORMANCE
	enum {
		PERFORMANCE_ENHANCE_DANCE_MIND      = 0x11C1772E,
		PERFORMANCE_ENHANCE_MUSIC_FOCUS     = 0x2E77F586,
		PERFORMANCE_ENHANCE_MUSIC_WILLPOWER = 0x3EC6FCB6
	};

	// SPICE
	enum {
		SPICE_BOOSTER_BLUE_UP    = 0x18A5AEFB,
		SPICE_BOOSTER_BLUE_DOWN  = 0xC62DFBF1,
		SPICE_CRASH_N_BURN_UP    = 0x5E1BE4D6,
		SPICE_CRASH_N_BURN_DOWN  = 0xA920056F,
		SPICE_DROID_LUBE_UP      = 0x0C969AE9,
		SPICE_DROID_LUBE_DOWN    = 0x7E3B0A02,
		SPICE_GIGGLEDUST_UP      = 0x3E41BA17,
		SPICE_GIGGLEDUST_DOWN    = 0xC11922D4,
		SPICE_GREY_GABAKI_UP     = 0xE5C9CD20,
		SPICE_GREY_GABAKI_DOWN   = 0xD2121CC4,
		SPICE_GUNJACK_UP         = 0x09B6F8FC,
		SPICE_GUNJACK_DOWN       = 0x13305C97,
		SPICE_MUON_GOLD_UP       = 0xFBE87E37,
		SPICE_MUON_GOLD_DOWN     = 0x8228D074,
		SPICE_NEUTRON_PIXEY_UP   = 0x5DC6921F,
		SPICE_NEUTRON_PIXEY_DOWN = 0x8D549786,
		SPICE_PYREPENOL_UP       = 0x1EBC62E5,
		SPICE_PYREPENOL_DOWN     = 0x813D6F28,
		SPICE_SCRAMJET_UP        = 0x2E03F676,
		SPICE_SCRAMJET_DOWN      = 0xA02DF2DE,
		SPICE_SEDATIVE_H4B_UP    = 0xD7A72ACF,
		SPICE_SEDATIVE_H4B_DOWN  = 0x35D64963,
		SPICE_SHADOWPAW_UP       = 0x3AAD2B89,
		SPICE_SHADOWPAW_DOWN     = 0xE506BE60,
		SPICE_SWEETBLOSSOM_UP    = 0x7EC00CB8,
		SPICE_SWEETBLOSSOM_DOWN  = 0x5B3DB3C1,
		SPICE_THRUSTER_HEAD_UP   = 0x530E31B7,
		SPICE_THRUSTER_HEAD_DOWN = 0x3AE01653,
		SPICE_YARROCK_UP         = 0xE7F8C957,
		SPICE_YARROCK_DOWN       = 0xA664F2EC,
		SPICE_KLIKNIK_BOOST_UP   = 0x37173CAD,
		SPICE_KLIKNIK_BOOST_DOWN = 0x7DE346EF,
		SPICE_KWI_BOOST_UP       = 0x629FA918,
		SPICE_KWO_BOOST_DOWN     = 0x714E1687
	};


	// FORCE_RANK
	enum {
		FORCE_RANK_SUFFERING = 0xF531B147,
		FORCE_RANK_SERENITY  = 0xA09E5934
	};

	// JEDI
	enum {
		JEDI_FORCE_RUN_1            = 0x9A04E4F8,
		JEDI_FORCE_RUN_2            = 0x9747C221,
		JEDI_FORCE_RUN_3            = 0x9386DF96,
		JEDI_FORCE_SPEED_1          = 0xA8A97B7F,
		JEDI_FORCE_SPEED_2          = 0xA5EA5DA6,
		JEDI_FORCE_ARMOR_1          = 0xFB65D2D6,
		JEDI_FORCE_ARMOR_2          = 0xF626F40F,
		JEDI_FORCE_SHIELD_1         = 0x10900417,
		JEDI_FORCE_SHIELD_2         = 0x1DD322CE,
		JEDI_FORCE_PROTECTION_1     = 0x6F1A719D,
		JEDI_FORCE_FEEDBACK_1       = 0xB9EC27B0,
		JEDI_FORCE_FEEDBACK_2       = 0xB4AF0169,
		JEDI_FORCE_ABSORB_1         = 0xA7FCF8F0,
		JEDI_FORCE_ABSORB_2         = 0xAABFDE29,
		JEDI_RESIST_DISEASE         = 0xD9925CB4,
		JEDI_RESIST_POISON          = 0x2F969FE7,
		JEDI_RESIST_BLEEDING        = 0xADA10184,
		JEDI_RESIST_STATES          = 0xB0457693,
		JEDI_AVOID_INCAPACITATION   = 0x4FDD1C66,
		JEDI_AVOID_INCAPACITATION_1 = 0xA077BD29,
		JEDI_AVOID_INCAPACITATION_2 = 0xAD349BF0,
		JEDI_AVOID_INCAPACITATION_3 = 0xA9F58647,
		JEDI_AVOID_INCAPACITATION_4 = 0xB7B2D642,
		JEDI_AVOID_INCAPACITATION_5 = 0xB373CBF5,
		JEDI_CLOAKOFPAIN 			= 0x2F050AF7,
		JEDI_FORCEBREACH			= 0x10786E23,
		JEDI_CORTOSIS				= 0xA0D83CB5,
		JEDI_FORTIFY				= 0xF8A901CC,
		JEDI_FORCEWEAKEN1			= 0x09938BBD,
		JEDI_FORCEWEAKEN2			= 0x04D0AD64,
		JEDI_FORCEWEAKEN3			= 0x0011B0D3
	};


	// SKILL
	enum {
		SKILL_BUFF_MELEE_ACCURACY        = 0x548DE45B,
		SKILL_BUFF_RANGED_DEFENSE        = 0xA6407812,
		SKILL_BUFF_CARBINE_ACCURACY      = 0xA996D07E,
		SKILL_BUFF_CARBINE_SPEED         = 0xC6148607,
		SKILL_BUFF_MASK_SCENT            = 0x30EB839E,
		SKILL_BUFF_MELEE_DEFENSE         = 0xC78FA3B7,
		SKILL_BUFF_ONEHANDMELEE_ACCURACY = 0x05C34219,
		SKILL_BUFF_ONEHANDMELEE_SPEED    = 0x0696D76F,
		SKILL_BUFF_PISTOL_ACCURACY       = 0x55471D61,
		SKILL_BUFF_PISTOL_SPEED          = 0x26F41BAD,
		SKILL_BUFF_RIFLE_ACCURACY        = 0x33ADEE46,
		SKILL_BUFF_RIFLE_SPEED           = 0x1F7E043E,
		SKILL_BUFF_UNARMED_ACCURACY      = 0x027D9E64,
		SKILL_BUFF_UNARMED_SPEED         = 0x133ADA2F,
		SKILL_BUFF_TWOHANDMELEE_ACCURACY = 0x8F47051E,
		SKILL_BUFF_TWOHANDMELEE_SPEED    = 0x5F1BD61A,
		SKILL_BUFF_RANGED_ACCURACY       = 0x33329A7B,
		SKILL_BUFF_THROWN_ACCURACY       = 0xBFEEDABF,
		SKILL_BUFF_THROWN_SPEED          = 0x232EE11E,
		SKILL_BUFF_HEAVY_WEAPON_ACCURACY = 0xAFDFE174,
		SKILL_BUFF_HEAVY_WEAPON_SPEED    = 0x311A7044,
		SKILL_BUFF_POLEARM_ACCURACY      = 0xF0C5EEED,
		SKILL_BUFF_POLEARM_SPEED         = 0x6F675FB6,
		SKILL_BUFF_POWERBOOST            = 0x7C0CCF17,
		SKILL_BUFF_HUNTER				 = 0x9F1DFE2F,
		SKILL_BUFF_HUNTED1 				 = 0x942FBDFA,
		SKILL_BUFF_HUNTED2 				 = 0x996C9B23,
		SKILL_BUFF_HUNTED3 				 = 0x9DAD8694,
		SKILL_BUFF_MASK_SCENT_SELF       = 0xEF00F74D,
	};

	// Infinity: Death Watch PvP buff CRCs
	enum {
		DW_BACTA_DENIAL      = 0xDEA7BACF,  // Hettir: Blocks stimpack usage
		DW_MIND_IMMUNITY     = 0xDEA7D1C1,  // Tor'cabur: Mind damage immunity (toggle)
		DW_DAMAGE_SHIELD     = 0xDEA7D2C2,  // Beskar'gam: Damage mitigation
		DW_MISSILE_REFLECTOR = 0xDEA7D3C3,  // Gotal'din: Missile reflection chance
		DW_TARGETING_BOOST   = 0xDEA7D4C4   // Ara'novor: Accuracy bonus for missiles
	};


	// INNATE
	enum {
		INNATE_BUFF_REGENERATION = 0xD1514A47,
		INNATE_BUFF_VITALIZE     = 0x477D7E4
	};


	// FOOD
	enum {
		FOOD_MITIGATE_DAMAGE             = 0x3B105001,
		FOOD_HEAL_RECOVERY               = 0xB9EB6FC6,
		FOOD_INCAP_RECOVERY              = 0x635AC195,
		FOOD_DODGE_ATTACK                = 0x030F591C,
		FOOD_ATTACK_ACCURACY             = 0x1E197C45,
		FOOD_POISON_DISEASE_RESIST       = 0x6CB96F62,
		FOOD_BLEED_RESIST                = 0x015260E8,
		FOOD_FIRE_RESIST                 = 0xF72CBED5,
		FOOD_XP_INCREASE                 = 0xD006C190,
		FOOD_CRAFT_BONUS                 = 0x65F96F03,
		FOOD_EXPERIMENT_BONUS            = 0x9B38A4CB,
		FOOD_REDUCE_CLONE_WOUNDS         = 0x9961A0F0,
		FOOD_REDUCE_SPICE_DOWNTIME       = 0x956C8A0E,
		FOOD_ACCELERATE_ENTERTAINER_BUFF = 0x13F08F86
	};

	// FOOD_DISH
	enum {
		FOOD_DISH_TRAVEL_BISCUITS              = 0x62932AF7,
		FOOD_DISH_KANALI_WAFERS                = 0xFCBAD7ED,
		FOOD_DISH_TELTIER_NOODLES              = 0x9AC8258E,
		FOOD_DISH_VEGHASH                      = 0xD94616DE,
		FOOD_DISH_GRUUVAN_SHAAL                = 0x6F291A67,
		FOOD_DISH_SCRIMPI                      = 0xBCB608D9,
		FOOD_DISH_AHRISA                       = 0x234F7DA8,
		FOOD_DISH_RAKRIRIAN_BURNOUT_SAUCE      = 0xC2D87307,
		FOOD_DISH_VEGEPARSINE                  = 0xF02878A3,
		FOOD_DISH_BIVOLI_TEMPARI               = 0x2114D76D,
		FOOD_DISH_TERRATTA                     = 0x904ACDDB,
		FOOD_DISH_THAKITILLO                   = 0xDC5D4FD7,
		FOOD_DISH_VERCUPTI_OF_AGAZZA_BOLERUUEE = 0x50AFB0A1
	};

	// FOOD_DESSERT
	enum {
		FOOD_DESSERT_BLOB_CANDY                   = 0x8268C9CB,
		FOOD_DESSERT_PKNEB                        = 0xF12D3826,
		FOOD_DESSERT_ALMOND_KWEVVU_CRISP_MUNCHIES = 0x8B82942E,
		FOOD_DESSERT_BOFA_TREAT                   = 0x8B72DED3,
		FOOD_DESSERT_WON_WON                      = 0xC4104F19,
		FOOD_DESSERT_CHANDAD                      = 0x7E4BED62,
		FOOD_DESSERT_DWEEZEL                      = 0x752E4991,
		FOOD_DESSERT_KIWIK_CLUSJO_SWIRL           = 0x1B25BCC9,
		FOOD_DESSERT_VAGNERIAN_CANAPE             = 0xFB554185
	};

	// FOOD_DRINK
	enum {
		FOOD_DRINK_ACCARRAGM                    = 0xE61D3848,
		FOOD_DRINK_BREATH_OF_HEAVEN             = 0xF87AA911,
		FOOD_DRINK_CAF                          = 0x0BDFFC50,
		FOOD_DRINK_CORELLIAN_ALE                = 0x02AE6B5E,
		FOOD_DRINK_CORELLIAN_BRANDY             = 0xDFB97272,
		FOOD_DRINK_DEUTERIUM_PYRO               = 0x522345BE,
		FOOD_DRINK_DURINDFIRE                   = 0x728B0811,
		FOOD_DRINK_ELSHANDRUU_PICA_THUNDERCLOUD = 0x22B58BFA,
		FOOD_DRINK_GARRMORL                     = 0x6B9D4C68,
		FOOD_DRINK_GRALINYN_JUICE               = 0x58BEB487,
		FOOD_DRINK_ITHORIAN_MIST                = 0xBA83D0E2,
		FOOD_DRINK_JAWA_BEER                    = 0x1D9DAE11,
		FOOD_DRINK_SPICED_TEA                   = 0x717401BC,
		FOOD_DRINK_VASARIAN_BRANDY              = 0x292DD6D3,
		FOOD_DRINK_VAYERBOK                     = 0x188D3769,
		FOOD_DRINK_VERONIAN_BERRY_WINE          = 0xABA01703
	};

	static uint32 getMedicalBuff(uint8 attribute) {
		uint32 buffCRC = 0;

		switch (attribute) {
		case BuffAttribute::HEALTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_HEALTH;
			break;
		case BuffAttribute::ACTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_ACTION;
			break;
		case BuffAttribute::STRENGTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STRENGTH;
			break;
		case BuffAttribute::CONSTITUTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_CONSTITUTION;
			break;
		case BuffAttribute::QUICKNESS:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_QUICKNESS;
			break;
		case BuffAttribute::STAMINA:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STAMINA;
			break;
		case BuffAttribute::POISON:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_POISON;
			break;
		case BuffAttribute::DISEASE:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_DISEASE;
			break;
		}

		return buffCRC;
	}

    // Infinity:  Function to get buff name from datatables/modifiers.iff given the CRC value
    static String getBuffNameFromCRC(uint32 crcValue) {
        switch (crcValue) {
			case MEDICAL_ENHANCE_HEALTH:  return "medical_enhance_health";  break;
			case MEDICAL_ENHANCE_STRENGTH:  return "medical_enhance_strength";  break;
			case MEDICAL_ENHANCE_CONSTITUTION:  return "medical_enhance_constitution";  break;
			case MEDICAL_ENHANCE_ACTION:  return "medical_enhance_action";  break;
			case MEDICAL_ENHANCE_QUICKNESS:  return "medical_enhance_quickness";  break;
			case MEDICAL_ENHANCE_STAMINA:  return "medical_enhance_stamina";  break;
			case MEDICAL_ENHANCE_POISON:  return "medical_enhance_poison";  break;
			case MEDICAL_ENHANCE_DISEASE:  return "medical_enhance_disease";  break;
			case PERFORMANCE_ENHANCE_DANCE_MIND:  return "performance_enhance_dance_mind";  break;
			case PERFORMANCE_ENHANCE_MUSIC_FOCUS:  return "performance_enhance_music_focus";  break;
			case PERFORMANCE_ENHANCE_MUSIC_WILLPOWER:  return "performance_enhance_music_willpower";  break;
			case SPICE_BOOSTER_BLUE_UP:  return "spice.booster_blue.up";  break;
			case SPICE_BOOSTER_BLUE_DOWN:  return "spice.booster_blue.down";  break;
			case SPICE_CRASH_N_BURN_UP:  return "spice.crash_n_burn.up";  break;
			case SPICE_CRASH_N_BURN_DOWN:  return "spice.crash_n_burn.down";  break;
			case SPICE_DROID_LUBE_UP:  return "spice.droid_lube.up";  break;
			case SPICE_DROID_LUBE_DOWN:  return "spice.droid_lube.down";  break;
			case SPICE_GIGGLEDUST_UP:  return "spice.giggledust.up";  break;
			case SPICE_GIGGLEDUST_DOWN:  return "spice.giggledust.down";  break;
			case SPICE_GREY_GABAKI_UP:  return "spice.grey_gabaki.up";  break;
			case SPICE_GREY_GABAKI_DOWN:  return "spice.grey_gabaki.down";  break;
			case SPICE_GUNJACK_UP:  return "spice.gunjack.up";  break;
			case SPICE_GUNJACK_DOWN:  return "spice.gunjack.down";  break;
			case SPICE_MUON_GOLD_UP:  return "spice.muon_gold.up";  break;
			case SPICE_MUON_GOLD_DOWN:  return "spice.muon_gold.down";  break;
			case SPICE_NEUTRON_PIXEY_UP:  return "spice.neutron_pixey.up";  break;
			case SPICE_NEUTRON_PIXEY_DOWN:  return "spice.neutron_pixey.down";  break;
			case SPICE_PYREPENOL_UP:  return "spice.pyrepenol.up";  break;
			case SPICE_PYREPENOL_DOWN:  return "spice.pyrepenol.down";  break;
			case SPICE_SCRAMJET_UP:  return "spice.scramjet.up";  break;
			case SPICE_SCRAMJET_DOWN:  return "spice.scramjet.down";  break;
			case SPICE_SEDATIVE_H4B_UP:  return "spice.sedative_h4b.up";  break;
			case SPICE_SEDATIVE_H4B_DOWN:  return "spice.sedative_h4b.down";  break;
			case SPICE_SHADOWPAW_UP:  return "spice.shadowpaw.up";  break;
			case SPICE_SHADOWPAW_DOWN:  return "spice.shadowpaw.down";  break;
			case SPICE_SWEETBLOSSOM_UP:  return "spice.sweetblossom.up";  break;
			case SPICE_SWEETBLOSSOM_DOWN:  return "spice.sweetblossom.down";  break;
			case SPICE_THRUSTER_HEAD_UP:  return "spice.thruster_head.up";  break;
			case SPICE_THRUSTER_HEAD_DOWN:  return "spice.thruster_head.down";  break;
			case SPICE_YARROCK_UP:  return "spice.yarrock.up";  break;
			case SPICE_YARROCK_DOWN:  return "spice.yarrock.down";  break;
			case SPICE_KLIKNIK_BOOST_UP:  return "spice.kliknik_boost.up";  break;
			case SPICE_KLIKNIK_BOOST_DOWN:  return "spice.kliknik_boost.down";  break;
			case SPICE_KWI_BOOST_UP:  return "spice.kwi_boost.up";  break;
			case SPICE_KWO_BOOST_DOWN:  return "spice.kwi_boost.down";  break;
			case JEDI_FORCE_RUN_1:  return "jedi.force_run.1";  break;
			case JEDI_FORCE_RUN_2:  return "jedi.force_run.2";  break;
			case JEDI_FORCE_RUN_3:  return "jedi.force_run.3";  break;
			case JEDI_FORCE_SPEED_1:  return "jedi.force_speed.1";  break;
			case JEDI_FORCE_SPEED_2:  return "jedi.force_speed.2";  break;
			case JEDI_FORCE_ARMOR_1:  return "jedi.force_armor.1";  break;
			case JEDI_FORCE_ARMOR_2:  return "jedi.force_armor.2";  break;
			case JEDI_FORCE_SHIELD_1:  return "jedi.force_shield.1";  break;
			case JEDI_FORCE_SHIELD_2:  return "jedi.force_shield.2";  break;
			case JEDI_FORCE_PROTECTION_1:  return "jedi.force_protection.1";  break;
			case JEDI_FORCE_FEEDBACK_1:  return "jedi.force_feedback.1";  break;
			case JEDI_FORCE_FEEDBACK_2:  return "jedi.force_feedback.2";  break;
			case JEDI_FORCE_ABSORB_1:  return "jedi.force_absorb.1";  break;
			case JEDI_FORCE_ABSORB_2:  return "jedi.force_absorb.2";  break;
			case JEDI_RESIST_DISEASE:  return "jedi.resist_disease";  break;
			case JEDI_RESIST_POISON:  return "jedi.resist_poison";  break;
			case JEDI_RESIST_BLEEDING:  return "jedi.resist_bleeding";  break;
			case JEDI_RESIST_STATES:  return "jedi.resist_states";  break;
			case JEDI_AVOID_INCAPACITATION:  return "edi.avoid_incapacitation";  break;
			case JEDI_AVOID_INCAPACITATION_1:  return "jedi.avoid_incapacitation.1";  break;
			case JEDI_AVOID_INCAPACITATION_2:  return "jedi.avoid_incapacitation.2";  break;
			case JEDI_AVOID_INCAPACITATION_3:  return "jedi.avoid_incapacitation.3";  break;
			case JEDI_AVOID_INCAPACITATION_4:  return "jedi.avoid_incapacitation.4";  break;
			case JEDI_AVOID_INCAPACITATION_5:  return "jedi.avoid_incapacitation.5";  break;
			case JEDI_CLOAKOFPAIN:  return "jedi.cloakofpain";  break;
			case JEDI_FORCEBREACH:  return "jedi.forcebreach";  break;
			case JEDI_CORTOSIS:  return "jedi.cortosis";  break;
			case JEDI_FORTIFY:  return "jedi.fortify";  break;
			case JEDI_FORCEWEAKEN1:  return "jedi.force_weaken.1";  break;
			case JEDI_FORCEWEAKEN2:  return "jedi.force_weaken.2";  break;
			case JEDI_FORCEWEAKEN3:  return "jedi.force_weaken.3";  break;
			case SKILL_BUFF_MELEE_ACCURACY:  return "skill_buff_melee_accuracy";  break;
			case SKILL_BUFF_RANGED_DEFENSE:  return "skill_buff_ranged_defense";  break;
			case SKILL_BUFF_CARBINE_ACCURACY:  return "skill_buff_carbine_accuracy";  break;
			case SKILL_BUFF_CARBINE_SPEED:  return "skill_buff_carbine_speed";  break;
			case SKILL_BUFF_MASK_SCENT:  return "skill_buff_camouflage";  break;
			case SKILL_BUFF_MASK_SCENT_SELF:  return "skill_buff_mask_scent";  break;
			case SKILL_BUFF_MELEE_DEFENSE:  return "skill_buff_melee_defense";  break;
			case SKILL_BUFF_ONEHANDMELEE_ACCURACY:  return "skill_buff_onehandmelee_accuracy";  break;
			case SKILL_BUFF_ONEHANDMELEE_SPEED:  return "skill_buff_onehandmelee_speed";  break;
			case SKILL_BUFF_PISTOL_ACCURACY:  return "skill_buff_pistol_accuracy";  break;
			case SKILL_BUFF_PISTOL_SPEED:  return "skill_buff_pistol_speed";  break;
			case SKILL_BUFF_RIFLE_ACCURACY:  return "skill_buff_rifle_accuracy";  break;
			case SKILL_BUFF_RIFLE_SPEED:  return "skill_buff_rifle_speed";  break;
			case SKILL_BUFF_UNARMED_ACCURACY:  return "skill_buff_unarmed_accuracy";  break;
			case SKILL_BUFF_UNARMED_SPEED:  return "skill_buff_unarmed_speed";  break;
			case SKILL_BUFF_TWOHANDMELEE_ACCURACY:  return "skill_buff_twohandmelee_accuracy";  break;
			case SKILL_BUFF_TWOHANDMELEE_SPEED:  return "skill_buff_twohandmelee_speed";  break;
			case SKILL_BUFF_RANGED_ACCURACY:  return "skill_buff_ranged_accuracy";  break;
			case SKILL_BUFF_THROWN_ACCURACY:  return "skill_buff_thrown_accuracy";  break;
			case SKILL_BUFF_THROWN_SPEED:  return "skill_buff_thrown_speed";  break;
			case SKILL_BUFF_HEAVY_WEAPON_ACCURACY:  return "skill_buff_heavy_weapon_accuracy";  break;
			case SKILL_BUFF_HEAVY_WEAPON_SPEED:  return "skill_buff_heavy_weapon_speed";  break;
			case SKILL_BUFF_POLEARM_ACCURACY:  return "skill_buff_polearm_accuracy";  break;
			case SKILL_BUFF_POLEARM_SPEED:  return "skill_buff_polearm_speed";  break;
			case SKILL_BUFF_POWERBOOST:  return "skill_buff_powerboost";  break;
			case SKILL_BUFF_HUNTER:  return "skill_buff_hunter";  break;
			case SKILL_BUFF_HUNTED1:  return "skill_buff_hunted1";  break;
			case SKILL_BUFF_HUNTED2:  return "skill_buff_hunted2";  break;
			case SKILL_BUFF_HUNTED3:  return "skill_buff_hunted3";  break;
			case DW_BACTA_DENIAL:      return "dw.bacta_denial";      break;
			case DW_MIND_IMMUNITY:     return "dw.mind_immunity";     break;
			case DW_DAMAGE_SHIELD:     return "dw.damage_shield";     break;
			case DW_MISSILE_REFLECTOR: return "dw.missile_reflector"; break;
			case DW_TARGETING_BOOST:   return "dw.targeting_boost";   break;
			case INNATE_BUFF_REGENERATION:  return "innate_buff_regeneration";  break;
			case INNATE_BUFF_VITALIZE:  return "innate_buff_vitalize";  break;
			case FOOD_MITIGATE_DAMAGE:  return "food.mitigate_damage";  break;
			case FOOD_HEAL_RECOVERY:  return "food.heal_recovery";  break;
			case FOOD_INCAP_RECOVERY:  return "food.incap_recovery";  break;
			case FOOD_DODGE_ATTACK:  return "food.dodge_attack";  break;
			case FOOD_ATTACK_ACCURACY:  return "food.attack_accuracy";  break;
			case FOOD_POISON_DISEASE_RESIST:  return "food.poison_disease_resist";  break;
			case FOOD_BLEED_RESIST:  return "food.bleed_resist";  break;
			case FOOD_FIRE_RESIST:  return "food.fire_resist";  break;
			case FOOD_XP_INCREASE:  return "food.xp_increase";  break;
			case FOOD_CRAFT_BONUS:  return "food.craft_bonus";  break;
			case FOOD_EXPERIMENT_BONUS:  return "food.experiment_bonus";  break;
			case FOOD_REDUCE_CLONE_WOUNDS:  return "food.reduce_clone_wounds";  break;
			case FOOD_REDUCE_SPICE_DOWNTIME:  return "food.reduce_spice_downtime";  break;
			case FOOD_ACCELERATE_ENTERTAINER_BUFF:  return "food.accelerate_entertainer_buff";  break;
			case FOOD_DISH_TRAVEL_BISCUITS:  return "food.dish_travel_biscuits";  break;
			case FOOD_DISH_KANALI_WAFERS:  return "food.dish_kanali_wafers";  break;
			case FOOD_DISH_TELTIER_NOODLES:  return "food.dish_teltier_noodles";  break;
			case FOOD_DISH_VEGHASH:  return "food.dish_veghash";  break;
			case FOOD_DISH_GRUUVAN_SHAAL:  return "food.dish_gruuvan_shaal";  break;
			case FOOD_DISH_SCRIMPI:  return "food.dish_scrimpi";  break;
			case FOOD_DISH_AHRISA:  return "food.dish_ahrisa";  break;
			case FOOD_DISH_RAKRIRIAN_BURNOUT_SAUCE:  return "food.dish_rakririan_burnout_sauce";  break;
			case FOOD_DISH_VEGEPARSINE:  return "food.dish_vegeparsine";  break;
			case FOOD_DISH_BIVOLI_TEMPARI:  return "food.dish_bivoli_tempari";  break;
			case FOOD_DISH_TERRATTA:  return "food.dish_terratta";  break;
			case FOOD_DISH_THAKITILLO:  return "food.dish_thakitillo";  break;
			case FOOD_DISH_VERCUPTI_OF_AGAZZA_BOLERUUEE:  return "food.dish_vercupti_of_agazza_boleruuee";  break;
			case FOOD_DESSERT_BLOB_CANDY:  return "food.dessert_blob_candy";  break;
			case FOOD_DESSERT_PKNEB:  return "food.dessert_pkneb";  break;
			case FOOD_DESSERT_ALMOND_KWEVVU_CRISP_MUNCHIES:  return "food.dessert_almond_kwevvu_crisp_munchies";  break;
			case FOOD_DESSERT_BOFA_TREAT:  return "food.dessert_bofa_treat";  break;
			case FOOD_DESSERT_WON_WON:  return "food.dessert_won_won";  break;
			case FOOD_DESSERT_CHANDAD:  return "food.dessert_chandad";  break;
			case FOOD_DESSERT_DWEEZEL:  return "food.dessert_dweezel";  break;
			case FOOD_DESSERT_KIWIK_CLUSJO_SWIRL:  return "food.dessert_kiwik_clusjo_swirl";  break;
			case FOOD_DESSERT_VAGNERIAN_CANAPE:  return "food.dessert_vagnerian_canape";  break;
			case FOOD_DRINK_ACCARRAGM:  return "food.drink_accarragm";  break;
			case FOOD_DRINK_BREATH_OF_HEAVEN:  return "food.drink_breath_of_heaven";  break;
			case FOOD_DRINK_CAF:  return "food.drink_caf";  break;
			case FOOD_DRINK_CORELLIAN_ALE:  return "food.drink_corellian_ale";  break;
			case FOOD_DRINK_CORELLIAN_BRANDY:  return "food.drink_corellian_brandy";  break;
			case FOOD_DRINK_DEUTERIUM_PYRO:  return "food.drink_deuterium_pyro";  break;
			case FOOD_DRINK_DURINDFIRE:  return "food.drink_durindfire";  break;
			case FOOD_DRINK_ELSHANDRUU_PICA_THUNDERCLOUD:  return "food.drink_elshandruu_pica_thundercloud";  break;
			case FOOD_DRINK_GARRMORL:  return "food.drink_garrmorl";  break;
			case FOOD_DRINK_GRALINYN_JUICE:  return "food.drink_gralinyn_juice";  break;
			case FOOD_DRINK_ITHORIAN_MIST:  return "food.drink_ithorian_mist";  break;
			case FOOD_DRINK_JAWA_BEER:  return "food.drink_jawa_beer";  break;
			case FOOD_DRINK_SPICED_TEA:  return "food.drink_spiced_tea";  break;
			case FOOD_DRINK_VASARIAN_BRANDY:  return "food.drink_vasarian_brandy";  break;
			case FOOD_DRINK_VAYERBOK:  return "food.drink_vayerbok";  break;
			case FOOD_DRINK_VERONIAN_BERRY_WINE:  return "food.drink_veronian_berry_wine";  break;
			case STRING_HASHCODE("petFoodBuff"):  return "petFoodBuff";  break;
			case STRING_HASHCODE("burstrun"):  return "burstrun";  break;
			case STRING_HASHCODE("dash"):  return "dash";  break;
			case STRING_HASHCODE("rocketboots"):  return "rocketboots";  break;
			case STRING_HASHCODE("gallop"):  return "gallop";  break;
			case STRING_HASHCODE("retreat"):  return "retreat";  break;
			case STRING_HASHCODE("rally"):  return "rally";  break;
			case 0x8AD4026A: return "RALLIED_STATE"; break;
			case STRING_HASHCODE("setSpeed"):  return "setSpeed";  break;
			case STRING_HASHCODE("centerofbeing"):  return "centerofbeing";  break;
			case STRING_HASHCODE("channelforcebuff"):  return "channelforcebuff";  break;
			case STRING_HASHCODE("emboldenPet"):  return "emboldenPet";  break;
			case STRING_HASHCODE("enragePet"):  return "enragePet";  break;
			case STRING_HASHCODE("steadyaim"):  return "steadyaim";  break;
			case STRING_HASHCODE("dead_eye"):  return "dead_eye";  break;
			case STRING_HASHCODE("private_feign_buff"):  return "private_feign_buff";  break;

			default:
                return "Unknown CRC";
        }
    }
};


#endif /* BUFFCRC_H_ */
