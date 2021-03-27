
/*
 *  EVE_Roles.h
 *      namespace data for Account and Corporation
 *
 *
 */


#ifndef EVE_ROLES_H
#define EVE_ROLES_H

namespace Acct {

    namespace Type {
        enum {
            CCP                 = 13,
            UK                  = 17,
            PBC                 = 20,
            ETC                 = 21,
            Trial               = 23,   // most client tests are >22
            Mammon              = 30,   // this is default player account type
            Media               = 31,
            CDKey               = 33,
            IA                  = 34,
            // not used here
            DustPlayer          = 101,
            DustCCP             = 102,
            DustBattleServer    = 103
        };
    }

    namespace Role {
        enum:int64_t {
            DUST               = 1L,                      // 0x01                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001
            BANNING            = 2L,                      // 0x02                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0010
            MARKET             = 4L,                      // 0x04                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0100
            MARKETH            = 8L,                      // 0x08                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1000
            CSMADMIN           = 16L,                     // 0x010
            CSMDELEGATE        = 32L,                     // 0x020
            EXPOPLAYER         = 64L,                     // 0x040
            PETITIONEE         = 256L,                    // 0x0100
            CENTURION          = 2048L,                   // 0x0800
            // the client requires a module named "dna" for many menu items for the "worldmod" role
            //  we do not have this module, which leads to an error, and an inoperable rclick menu
            //  when WORLDMOD is part of a client's roles
            WORLDMOD           = 4096L,                   // 0x01000
            DBA                = 16384L,
            REMOTESERVICE      = 131072L,
            LEGIONEER          = 262144L,                  // get ALL corp chat channels (and is usually invis)
            TRANSLATION        = 524288L,
            CHTINVISIBLE       = 1048576L,
            CHTADMINISTRATOR   = 2097152L,
            HEALSELF           = 4194304L,
            HEALOTHERS         = 8388608L,
            NEWSREPORTER       = 16777216L,
            TRANSLATIONADMIN   = 134217728L,
            ACCOUNTMANAGEMENT  = 536870912L,
            SPAWN              = 8589934592L,             // 0x0200000000
            IGB                = 2147483648L,
            TRANSLATIONEDITOR  = 4294967296L,
            BATTLESERVER       = 17179869184LL,
            TRANSLATIONTESTER  = 34359738368LL,
            WIKIEDITOR         = 68719476736LL,
            TRANSFER           = 137438953472LL,
            GMS                = 274877906944LL,
            CL                 = 549755813888LL,
            CR                 = 1099511627776LL,
            CM                 = 2199023255552LL,
            BSDADMIN           = 35184372088832LL,
            PROGRAMMER         = 2251799813685248LL,
            QA                 = 4503599627370496LL,
            GMH                = 9007199254740992LL,
            // the client requires a module named "dna" for many menu items for the "gml" role
                    //  we do not have this module, which leads to an error, and an inoperable rclick menu
            //  when GML is part of a client's roles
            //      groove fixed this in a previous "liveupdates" patch
            GML                = 18014398509481984LL,
            CONTENT            = 36028797018963968LL,
            ADMIN              = 72057594037927936LL,
            VIPLOGIN           = 144115188075855872LL,
            ROLEADMIN          = 288230376151711744LL,
            NEWBIE             = 576460752303423488LL,
            SERVICE            = 1152921504606846976LL,        // can use station services without being docked.
            PLAYER             = 2305843009213693952LL, // 0x02
            LOGIN              = 4611686018427387904LL, // 0x04000000000000000  0b100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

            //ANY                = 18446744073709551615LL & ~IGB,     max uint64 invalid.  unsigned types removed from this version.  -allan 10Dec17

            STD                = LOGIN | PLAYER | IGB,
            VIP                = STD | VIPLOGIN | HEALSELF,
            TRANSAM            = VIP | TRANSLATION | TRANSLATIONADMIN | TRANSLATIONEDITOR,
            SLASH              = VIP | GML | LEGIONEER | SPAWN | HEALOTHERS,
            CREATOR            = SLASH | GMH | WORLDMOD | CONTENT,
            DEV                = CREATOR | QA | PROGRAMMER,
            BOSS               = DEV | ADMIN,
            EPLAYER            = VIP | HEALOTHERS,  //VIP+
            VIEW               = ADMIN | CONTENT | GML | GMH | QA
            /*
            * 23:05:28 G   EVEMu: Common Account Roles:
            * 23:05:28 W          DEV:  7131450020691447808(0x62f8000280c41000)
            * 23:05:28 W          STD:  6917529029788565504(0x6000000080000000)
            * 23:05:28 W          VIP:  7061644217868615680(0x6200000080400000)
            * 23:05:28 W         VIP+:  7061644217877004288(0x6200000080c00000)
            * 23:05:28 W         VIEW:  139611588448485376(0x1f0000000000000)
            * 23:05:28 W         BOSS:  7203507614729375744(0x63f8000280c41000)
            * 23:05:28 W        SLASH:  7079658624976683008(0x6240000280c40000)
            * 23:05:28 W      CREATOR:  7124694621250392064(0x62e0000280c41000)
            *
            */
        };
    }
}

namespace Corp {
    namespace Role {
        enum:int64_t {
            Member                          = 0,
            Director                        = 1,        // only role that can lock/unlock bp
            PersonnelManager                = 128,
            Accountant                      = 256,
            SecurityOfficer                 = 512,
            FactoryManager                  = 1024,
            StationManager                  = 2048,
            Auditor                         = 4096,
            HangarCanTake1                  = 8192,
            HangarCanTake2                  = 16384,
            HangarCanTake3                  = 32768,
            HangarCanTake4                  = 65536,
            HangarCanTake5                  = 131072,
            HangarCanTake6                  = 262144,
            HangarCanTake7                  = 524288,
            HangarCanQuery1                 = 1048576,
            HangarCanQuery2                 = 2097152,
            HangarCanQuery3                 = 4194304,
            HangarCanQuery4                 = 8388608,
            HangarCanQuery5                 = 16777216,
            HangarCanQuery6                 = 33554432,
            HangarCanQuery7                 = 67108864,
            AccountCanTake1                 = 134217728,
            AccountCanTake2                 = 268435456,
            AccountCanTake3                 = 536870912,
            AccountCanTake4                 = 1073741824,
            AccountCanTake5                 = 2147483648L,
            AccountCanTake6                 = 4294967296L,
            AccountCanTake7                 = 8589934592L,
            Diplomat                        = 17179869184L,
            EquipmentConfig                 = 2199023255552L,
            ContainerCanTake1               = 4398046511104L,
            ContainerCanTake2               = 8796093022208L,
            ContainerCanTake3               = 17592186044416L,
            ContainerCanTake4               = 35184372088832L,
            ContainerCanTake5               = 70368744177664L,
            ContainerCanTake6               = 140737488355328L,
            ContainerCanTake7               = 281474976710656L,
            CanRentOffice                   = 562949953421312L,
            CanRentFactorySlot              = 1125899906842624L,
            CanRentResearchSlot             = 2251799813685248L,
            JuniorAccountant                = 4503599627370496L,
            StarbaseConfig                  = 9007199254740992L,
            Trader                          = 18014398509481984L,
            ChatManager                     = 36028797018963968L,
            ContractManager                 = 72057594037927936L,
            InfrastructureTacticalOfficer   = 144115188075855872L,
            StarbaseCaretaker               = 288230376151711744L,
            FittingManager                  = 576460752303423488L,
            Missing                         = 3458764513820540928L, //   0x3000000000000000     <-- seen in logs, but not defined in client  ** not used **

            //Some Combos
            AllHangar   = HangarCanTake1|HangarCanTake2|HangarCanTake3|HangarCanTake4|HangarCanTake5|HangarCanTake6|HangarCanTake7|HangarCanQuery1|HangarCanQuery2|HangarCanQuery3|HangarCanQuery4|HangarCanQuery5|HangarCanQuery6|HangarCanQuery7,
            AllAccount  = JuniorAccountant|AccountCanTake1|AccountCanTake2|AccountCanTake3|AccountCanTake4|AccountCanTake5|AccountCanTake6|AccountCanTake7|Accountant,
            AllContainer= ContainerCanTake1|ContainerCanTake2|ContainerCanTake3|ContainerCanTake4|ContainerCanTake5|ContainerCanTake6|ContainerCanTake7,
            AllOffice   = CanRentOffice|CanRentFactorySlot|CanRentResearchSlot,
            AllStarbase = StarbaseCaretaker|StarbaseConfig|InfrastructureTacticalOfficer|EquipmentConfig,
            AllManager  = PersonnelManager|StationManager|FactoryManager|ChatManager|ContractManager|FittingManager,

            All         = AllHangar|AllAccount|AllContainer|AllOffice|AllStarbase|AllManager|Auditor|Diplomat,
            Admin       = All|Trader|SecurityOfficer|Director

            /*
             * 23:05:28 G   EVEMu: Common Corp Roles:
             * 23:05:28 W          Role_All:  1134904941433847168(0xfbffe07fffffd80)
             * 23:05:28 W         Role_Cont:  558551906910208(0x1fc0000000000)
             * 23:05:28 W        Role_Admin:  1152919339943329665(0xffffe07ffffff81)
             * 23:05:28 W       Role_Hangar:  134209536(0x7ffe000)
             * 23:05:28 W      Role_Account:  4503616673022208(0x100003f8000100)
             * 23:05:28 W     Role_Starbase:  441354962505564160(0x620020000000000)
             *
             */

        };
    }
}

#endif  //EVE_ROLES_H