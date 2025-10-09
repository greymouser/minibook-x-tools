/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20241212 (64-bit version)
 * Copyright (c) 2000 - 2023 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of ssdt9.dat
 *
 * Original Table Header:
 *     Signature        "SSDT"
 *     Length           0x00007A5F (31327)
 *     Revision         0x02
 *     Checksum         0xC4
 *     OEM ID           "DptfTb"
 *     OEM Table ID     "DptfTabl"
 *     OEM Revision     0x00001000 (4096)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20200717 (538969879)
 */
DefinitionBlock ("", "SSDT", 2, "DptfTb", "DptfTabl", 0x00001000)
{
    External (_SB_.AAC0, FieldUnitObj)
    External (_SB_.ACRT, FieldUnitObj)
    External (_SB_.APSV, FieldUnitObj)
    External (_SB_.CBMI, FieldUnitObj)
    External (_SB_.CFGD, FieldUnitObj)
    External (_SB_.CLVL, FieldUnitObj)
    External (_SB_.CPPC, FieldUnitObj)
    External (_SB_.CTC0, FieldUnitObj)
    External (_SB_.CTC1, FieldUnitObj)
    External (_SB_.CTC2, FieldUnitObj)
    External (_SB_.OSCP, IntObj)
    External (_SB_.PAGD, DeviceObj)
    External (_SB_.PAGD._PUR, PkgObj)
    External (_SB_.PAGD._STA, MethodObj)    // 0 Arguments
    External (_SB_.PC00, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.ACUR, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.AP01, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.AP02, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.AP10, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.ARTG, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.AVOL, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.B1FC, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.B1RC, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.BICC, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.BMAX, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CFAN, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CFSP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CHGR, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CHRG, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.CMDR, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CMPP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CPUP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.CTYP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.DFSP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.DGPU, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.ECAV, IntObj)
    External (_SB_.PC00.LPCB.H_EC.ECF2, OpRegionObj)
    External (_SB_.PC00.LPCB.H_EC.ECMD, MethodObj)    // 1 Arguments
    External (_SB_.PC00.LPCB.H_EC.ECRD, MethodObj)    // 1 Arguments
    External (_SB_.PC00.LPCB.H_EC.ECWT, MethodObj)    // 2 Arguments
    External (_SB_.PC00.LPCB.H_EC.FCHG, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.GFSP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.HYST, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PBOK, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PBSS, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PECH, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PENV, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PINV, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PLMX, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PMAX, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PPSH, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PPSL, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PPWR, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PROP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PSOC, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PSTP, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.PWRT, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.RBHF, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.SEN2, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.SEN3, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.SEN4, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.SEN5, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.TFN1, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.TFN2, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.TFN3, DeviceObj)
    External (_SB_.PC00.LPCB.H_EC.TSHT, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSI_, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSLT, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSR1, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSR2, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSR3, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSR4, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSR5, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.TSSR, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.UVTH, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.VBNL, FieldUnitObj)
    External (_SB_.PC00.LPCB.H_EC.VMIN, FieldUnitObj)
    External (_SB_.PC00.MC__.MHBR, FieldUnitObj)
    External (_SB_.PC00.TCPU, DeviceObj)
    External (_SB_.PL10, FieldUnitObj)
    External (_SB_.PL11, FieldUnitObj)
    External (_SB_.PL12, FieldUnitObj)
    External (_SB_.PL20, FieldUnitObj)
    External (_SB_.PL21, FieldUnitObj)
    External (_SB_.PL22, FieldUnitObj)
    External (_SB_.PLW0, FieldUnitObj)
    External (_SB_.PLW1, FieldUnitObj)
    External (_SB_.PLW2, FieldUnitObj)
    External (_SB_.PR00, ProcessorObj)
    External (_SB_.PR00._PSS, MethodObj)    // 0 Arguments
    External (_SB_.PR00._TPC, IntObj)
    External (_SB_.PR00._TSD, MethodObj)    // 0 Arguments
    External (_SB_.PR00._TSS, MethodObj)    // 0 Arguments
    External (_SB_.PR00.LPSS, PkgObj)
    External (_SB_.PR00.TPSS, PkgObj)
    External (_SB_.PR00.TSMC, PkgObj)
    External (_SB_.PR00.TSMF, PkgObj)
    External (_SB_.PR01, ProcessorObj)
    External (_SB_.PR02, ProcessorObj)
    External (_SB_.PR03, ProcessorObj)
    External (_SB_.PR04, ProcessorObj)
    External (_SB_.PR05, ProcessorObj)
    External (_SB_.PR06, ProcessorObj)
    External (_SB_.PR07, ProcessorObj)
    External (_SB_.PR08, ProcessorObj)
    External (_SB_.PR09, ProcessorObj)
    External (_SB_.PR10, ProcessorObj)
    External (_SB_.PR11, ProcessorObj)
    External (_SB_.PR12, ProcessorObj)
    External (_SB_.PR13, ProcessorObj)
    External (_SB_.PR14, ProcessorObj)
    External (_SB_.PR15, ProcessorObj)
    External (_SB_.PR16, ProcessorObj)
    External (_SB_.PR17, ProcessorObj)
    External (_SB_.PR18, ProcessorObj)
    External (_SB_.PR19, ProcessorObj)
    External (_SB_.PR20, ProcessorObj)
    External (_SB_.PR21, ProcessorObj)
    External (_SB_.PR22, ProcessorObj)
    External (_SB_.PR23, ProcessorObj)
    External (_SB_.PR24, ProcessorObj)
    External (_SB_.PR25, ProcessorObj)
    External (_SB_.PR26, ProcessorObj)
    External (_SB_.PR27, ProcessorObj)
    External (_SB_.PR28, ProcessorObj)
    External (_SB_.PR29, ProcessorObj)
    External (_SB_.PR30, ProcessorObj)
    External (_SB_.PR31, ProcessorObj)
    External (_SB_.SLPB, DeviceObj)
    External (_SB_.TAR0, FieldUnitObj)
    External (_SB_.TAR1, FieldUnitObj)
    External (_SB_.TAR2, FieldUnitObj)
    External (_SB_.TPWR, DeviceObj)
    External (_TZ_.ETMD, IntObj)
    External (_TZ_.TZ00, ThermalZoneObj)
    External (ACTT, IntObj)
    External (ATPC, IntObj)
    External (BATR, IntObj)
    External (CHGE, IntObj)
    External (CIDE, IntObj)
    External (CRTT, IntObj)
    External (DCFE, IntObj)
    External (DPTF, IntObj)
    External (ECON, IntObj)
    External (FND1, IntObj)
    External (FND2, IntObj)
    External (FND3, IntObj)
    External (HIDW, MethodObj)    // 4 Arguments
    External (HIWC, MethodObj)    // 1 Arguments
    External (IN34, IntObj)
    External (IPCS, MethodObj)    // 7 Arguments
    External (ODV0, IntObj)
    External (ODV1, IntObj)
    External (ODV2, IntObj)
    External (ODV3, IntObj)
    External (ODV4, IntObj)
    External (ODV5, IntObj)
    External (PCHE, FieldUnitObj)
    External (PF00, IntObj)
    External (PLID, IntObj)
    External (PNHM, IntObj)
    External (PPPR, IntObj)
    External (PPSZ, IntObj)
    External (PSVT, IntObj)
    External (PTPC, IntObj)
    External (PWRE, IntObj)
    External (PWRS, IntObj)
    External (S1DE, IntObj)
    External (S2DE, IntObj)
    External (S3DE, IntObj)
    External (S4DE, IntObj)
    External (S5DE, IntObj)
    External (S6DE, IntObj)
    External (S6P2, IntObj)
    External (SADE, IntObj)
    External (SSP1, IntObj)
    External (SSP2, IntObj)
    External (SSP3, IntObj)
    External (SSP4, IntObj)
    External (SSP5, IntObj)
    External (TCNT, IntObj)
    External (TSOD, IntObj)

    Scope (\_SB)
    {
        Device (IETM)
        {
            Method (GHID, 1, Serialized)
            {
                If ((Arg0 == "IETM"))
                {
                    Return ("INTC1041")
                }

                If ((Arg0 == "SEN1"))
                {
                    Return ("INTC1046")
                }

                If ((Arg0 == "SEN2"))
                {
                    Return ("INTC1046")
                }

                If ((Arg0 == "SEN3"))
                {
                    Return ("INTC1046")
                }

                If ((Arg0 == "SEN4"))
                {
                    Return ("INTC1046")
                }

                If ((Arg0 == "SEN5"))
                {
                    Return ("INTC1046")
                }

                If ((Arg0 == "TPCH"))
                {
                    Return ("INTC1049")
                }

                If ((Arg0 == "TFN1"))
                {
                    Return ("INTC1048")
                }

                If ((Arg0 == "TFN2"))
                {
                    Return ("INTC1048")
                }

                If ((Arg0 == "TFN3"))
                {
                    Return ("INTC1048")
                }

                If ((Arg0 == "TPWR"))
                {
                    Return ("INTC1060")
                }

                If ((Arg0 == "1"))
                {
                    Return ("INTC1061")
                }

                If ((Arg0 == "CHRG"))
                {
                    Return ("INTC1046")
                }

                Return ("XXXX9999")
            }

            Name (_UID, "IETM")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Method (_DSM, 4, Serialized)  // _DSM: Device-Specific Method
            {
                If (CondRefOf (HIWC))
                {
                    If (HIWC (Arg0))
                    {
                        If (CondRefOf (HIDW))
                        {
                            Return (HIDW (Arg0, Arg1, Arg2, Arg3))
                        }
                    }
                }

                Return (Buffer (One)
                {
                     0x00                                             // .
                })
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If (((\DPTF == One) && (\IN34 == One)))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Name (PTRP, Zero)
            Name (PSEM, Zero)
            Name (ATRP, Zero)
            Name (ASEM, Zero)
            Name (YTRP, Zero)
            Name (YSEM, Zero)
            Method (_OSC, 4, Serialized)  // _OSC: Operating System Capabilities
            {
                CreateDWordField (Arg3, Zero, STS1)
                CreateDWordField (Arg3, 0x04, CAP1)
                If ((Arg1 != One))
                {
                    STS1 &= 0xFFFFFF00
                    STS1 |= 0x0A
                    Return (Arg3)
                }

                If ((Arg2 != 0x02))
                {
                    STS1 &= 0xFFFFFF00
                    STS1 |= 0x02
                    Return (Arg3)
                }

                If (CondRefOf (\_SB.APSV))
                {
                    If ((PSEM == Zero))
                    {
                        PSEM = One
                        PTRP = \_SB.APSV /* External reference */
                    }
                }

                If (CondRefOf (\_SB.AAC0))
                {
                    If ((ASEM == Zero))
                    {
                        ASEM = One
                        ATRP = \_SB.AAC0 /* External reference */
                    }
                }

                If (CondRefOf (\_SB.ACRT))
                {
                    If ((YSEM == Zero))
                    {
                        YSEM = One
                        YTRP = \_SB.ACRT /* External reference */
                    }
                }

                If ((Arg0 == ToUUID ("b23ba85d-c8b7-3542-88de-8de2ffcfd698") /* Unknown UUID */))
                {
                    If (~(STS1 & One))
                    {
                        If ((CAP1 & One))
                        {
                            If ((CAP1 & 0x02))
                            {
                                \_SB.AAC0 = 0x6E
                                \_TZ.ETMD = Zero
                            }
                            Else
                            {
                                \_SB.AAC0 = ATRP /* \_SB_.IETM.ATRP */
                                \_TZ.ETMD = One
                            }

                            If ((CAP1 & 0x04))
                            {
                                \_SB.APSV = 0x6E
                            }
                            Else
                            {
                                \_SB.APSV = PTRP /* \_SB_.IETM.PTRP */
                            }

                            If ((CAP1 & 0x08))
                            {
                                \_SB.ACRT = 0xD2
                            }
                            Else
                            {
                                \_SB.ACRT = YTRP /* \_SB_.IETM.YTRP */
                            }

                            If (CondRefOf (\_TZ.TZ00))
                            {
                                Notify (\_TZ.TZ00, 0x81) // Information Change
                            }
                        }
                        Else
                        {
                            \_SB.ACRT = YTRP /* \_SB_.IETM.YTRP */
                            \_SB.APSV = PTRP /* \_SB_.IETM.PTRP */
                            \_SB.AAC0 = ATRP /* \_SB_.IETM.ATRP */
                            \_TZ.ETMD = One
                        }

                        If (CondRefOf (\_TZ.TZ00))
                        {
                            Notify (\_TZ.TZ00, 0x81) // Information Change
                        }
                    }

                    Return (Arg3)
                }

                Return (Arg3)
            }

            Method (DCFG, 0, NotSerialized)
            {
                Return (\DCFE) /* External reference */
            }

            Name (ODVX, Package (0x06)
            {
                Zero, 
                Zero, 
                Zero, 
                Zero, 
                Zero, 
                Zero
            })
            Method (ODVP, 0, Serialized)
            {
                ODVX [Zero] = \ODV0 /* External reference */
                ODVX [One] = \ODV1 /* External reference */
                ODVX [0x02] = \ODV2 /* External reference */
                ODVX [0x03] = \ODV3 /* External reference */
                ODVX [0x04] = \ODV4 /* External reference */
                ODVX [0x05] = \ODV5 /* External reference */
                Return (ODVX) /* \_SB_.IETM.ODVX */
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Mutex (PATM, 0x00)
        Method (_QF1, 0, NotSerialized)  // _Qxx: EC Query, xx=0x00-0xFF
        {
            Notify (\_SB.PC00.LPCB.H_EC.SEN3, 0x90) // Device-Specific
        }
    }

    Scope (\_SB.IETM)
    {
        Method (KTOC, 1, Serialized)
        {
            If ((Arg0 > 0x0AAC))
            {
                Return (((Arg0 - 0x0AAC) / 0x0A))
            }
            Else
            {
                Return (Zero)
            }
        }

        Method (CTOK, 1, Serialized)
        {
            Return (((Arg0 * 0x0A) + 0x0AAC))
        }

        Method (C10K, 1, Serialized)
        {
            Name (TMP1, Buffer (0x10)
            {
                 0x00                                             // .
            })
            CreateByteField (TMP1, Zero, TMPL)
            CreateByteField (TMP1, One, TMPH)
            Local0 = (Arg0 + 0x0AAC)
            TMPL = (Local0 & 0xFF)
            TMPH = ((Local0 & 0xFF00) >> 0x08)
            ToInteger (TMP1, Local1)
            Return (Local1)
        }

        Method (K10C, 1, Serialized)
        {
            If ((Arg0 > 0x0AAC))
            {
                Return ((Arg0 - 0x0AAC))
            }
            Else
            {
                Return (Zero)
            }
        }
    }

    Scope (\_SB.PC00.TCPU)
    {
        Name (PFLG, Zero)
        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
            If ((\SADE == One))
            {
                Return (0x0F)
            }
            Else
            {
                Return (Zero)
            }
        }

        OperationRegion (CPWR, SystemMemory, ((\_SB.PC00.MC.MHBR << 0x0F) + 0x5000), 0x1000)
        Field (CPWR, ByteAcc, NoLock, Preserve)
        {
            Offset (0x930), 
            PTDP,   15, 
            Offset (0x932), 
            PMIN,   15, 
            Offset (0x934), 
            PMAX,   15, 
            Offset (0x936), 
            TMAX,   7, 
            Offset (0x938), 
            PWRU,   4, 
            Offset (0x939), 
            EGYU,   5, 
            Offset (0x93A), 
            TIMU,   4, 
            Offset (0x958), 
            Offset (0x95C), 
            LPMS,   1, 
            CTNL,   2, 
            Offset (0x978), 
            PCTP,   8, 
            Offset (0x998), 
            RP0C,   8, 
            RP1C,   8, 
            RPNC,   8, 
            Offset (0xF3C), 
            TRAT,   8, 
            Offset (0xF40), 
            PTD1,   15, 
            Offset (0xF42), 
            TRA1,   8, 
            Offset (0xF44), 
            PMX1,   15, 
            Offset (0xF46), 
            PMN1,   15, 
            Offset (0xF48), 
            PTD2,   15, 
            Offset (0xF4A), 
            TRA2,   8, 
            Offset (0xF4C), 
            PMX2,   15, 
            Offset (0xF4E), 
            PMN2,   15, 
            Offset (0xF50), 
            CTCL,   2, 
                ,   29, 
            CLCK,   1, 
            MNTR,   8
        }

        Name (XPCC, Zero)
        Method (PPCC, 0, Serialized)
        {
            If (((XPCC == Zero) && CondRefOf (\_SB.CBMI)))
            {
                Switch (ToInteger (\_SB.CBMI))
                {
                    Case (Zero)
                    {
                        If (((\_SB.CLVL >= One) && (\_SB.CLVL <= 0x03)))
                        {
                            CPL0 ()
                            XPCC = One
                        }
                    }
                    Case (One)
                    {
                        If (((\_SB.CLVL == 0x02) || (\_SB.CLVL == 0x03)))
                        {
                            CPL1 ()
                            XPCC = One
                        }
                    }
                    Case (0x02)
                    {
                        If ((\_SB.CLVL == 0x03))
                        {
                            CPL2 ()
                            XPCC = One
                        }
                    }

                }
            }

            Return (NPCC) /* \_SB_.PC00.TCPU.NPCC */
        }

        Name (NPCC, Package (0x03)
        {
            0x02, 
            Package (0x06)
            {
                Zero, 
                0x88B8, 
                0xAFC8, 
                0x6D60, 
                0x7D00, 
                0x03E8
            }, 

            Package (0x06)
            {
                One, 
                0xDBBA, 
                0xDBBA, 
                Zero, 
                Zero, 
                0x03E8
            }
        })
        Method (CPNU, 2, Serialized)
        {
            Name (CNVT, Zero)
            Name (PPUU, Zero)
            Name (RMDR, Zero)
            If ((PWRU == Zero))
            {
                PPUU = One
            }
            Else
            {
                PPUU = (PWRU-- << 0x02)
            }

            Divide (Arg0, PPUU, RMDR, CNVT) /* \_SB_.PC00.TCPU.CPNU.CNVT */
            If ((Arg1 == Zero))
            {
                Return (CNVT) /* \_SB_.PC00.TCPU.CPNU.CNVT */
            }
            Else
            {
                CNVT *= 0x03E8
                RMDR *= 0x03E8
                RMDR /= PPUU
                CNVT += RMDR /* \_SB_.PC00.TCPU.CPNU.RMDR */
                Return (CNVT) /* \_SB_.PC00.TCPU.CPNU.CNVT */
            }
        }

        Method (CPL0, 0, NotSerialized)
        {
            \_SB.PC00.TCPU.NPCC [Zero] = 0x02
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [Zero] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [One] = 0x7D
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x02] = CPNU (\_SB.PL10, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x03] = (\_SB.PLW0 * 0x03E8)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x04] = ((\_SB.PLW0 * 0x03E8
                ) + 0x0FA0)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x05] = PPSZ /* External reference */
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [Zero] = One
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [One] = CPNU (\_SB.PL20, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x02] = CPNU (\_SB.PL20, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x03] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x04] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x05] = PPSZ /* External reference */
        }

        Method (CPL1, 0, NotSerialized)
        {
            \_SB.PC00.TCPU.NPCC [Zero] = 0x02
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [Zero] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [One] = 0x7D
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x02] = CPNU (\_SB.PL11, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x03] = (\_SB.PLW1 * 0x03E8)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x04] = ((\_SB.PLW1 * 0x03E8
                ) + 0x0FA0)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x05] = PPSZ /* External reference */
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [Zero] = One
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [One] = CPNU (\_SB.PL21, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x02] = CPNU (\_SB.PL21, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x03] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x04] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x05] = PPSZ /* External reference */
        }

        Method (CPL2, 0, NotSerialized)
        {
            \_SB.PC00.TCPU.NPCC [Zero] = 0x02
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [Zero] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [One] = 0x7D
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x02] = CPNU (\_SB.PL12, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x03] = (\_SB.PLW2 * 0x03E8)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x04] = ((\_SB.PLW2 * 0x03E8
                ) + 0x0FA0)
            DerefOf (\_SB.PC00.TCPU.NPCC [One]) [0x05] = PPSZ /* External reference */
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [Zero] = One
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [One] = CPNU (\_SB.PL22, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x02] = CPNU (\_SB.PL22, One)
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x03] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x04] = Zero
            DerefOf (\_SB.PC00.TCPU.NPCC [0x02]) [0x05] = PPSZ /* External reference */
        }

        Name (LSTM, Zero)
        Name (_PPC, Zero)  // _PPC: Performance Present Capabilities
        Method (SPPC, 1, Serialized)
        {
            If (CondRefOf (\_SB.CPPC))
            {
                \_SB.CPPC = Arg0
            }

            If ((ToInteger (\TCNT) > Zero))
            {
                Notify (\_SB.PR00, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > One))
            {
                Notify (\_SB.PR01, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x02))
            {
                Notify (\_SB.PR02, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x03))
            {
                Notify (\_SB.PR03, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x04))
            {
                Notify (\_SB.PR04, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x05))
            {
                Notify (\_SB.PR05, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x06))
            {
                Notify (\_SB.PR06, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x07))
            {
                Notify (\_SB.PR07, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x08))
            {
                Notify (\_SB.PR08, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x09))
            {
                Notify (\_SB.PR09, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0A))
            {
                Notify (\_SB.PR10, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0B))
            {
                Notify (\_SB.PR11, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0C))
            {
                Notify (\_SB.PR12, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0D))
            {
                Notify (\_SB.PR13, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0E))
            {
                Notify (\_SB.PR14, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x0F))
            {
                Notify (\_SB.PR15, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x10))
            {
                Notify (\_SB.PR16, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x11))
            {
                Notify (\_SB.PR17, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x12))
            {
                Notify (\_SB.PR18, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x13))
            {
                Notify (\_SB.PR19, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x14))
            {
                Notify (\_SB.PR20, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x15))
            {
                Notify (\_SB.PR21, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x16))
            {
                Notify (\_SB.PR22, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x17))
            {
                Notify (\_SB.PR23, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x18))
            {
                Notify (\_SB.PR24, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x19))
            {
                Notify (\_SB.PR25, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1A))
            {
                Notify (\_SB.PR26, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1B))
            {
                Notify (\_SB.PR27, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1C))
            {
                Notify (\_SB.PR28, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1D))
            {
                Notify (\_SB.PR29, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1E))
            {
                Notify (\_SB.PR30, 0x80) // Status Change
            }

            If ((ToInteger (\TCNT) > 0x1F))
            {
                Notify (\_SB.PR31, 0x80) // Status Change
            }
        }

        Method (SPUR, 1, NotSerialized)
        {
            If ((Arg0 <= \TCNT))
            {
                If ((\_SB.PAGD._STA () == 0x0F))
                {
                    \_SB.PAGD._PUR [One] = Arg0
                    Notify (\_SB.PAGD, 0x80) // Status Change
                }
            }
        }

        Method (PCCC, 0, Serialized)
        {
            PCCX [Zero] = One
            Switch (ToInteger (CPNU (PTDP, Zero)))
            {
                Case (0x39)
                {
                    DerefOf (PCCX [One]) [Zero] = 0xA7F8
                    DerefOf (PCCX [One]) [One] = 0x00017318
                }
                Case (0x2F)
                {
                    DerefOf (PCCX [One]) [Zero] = 0x9858
                    DerefOf (PCCX [One]) [One] = 0x00014C08
                }
                Case (0x25)
                {
                    DerefOf (PCCX [One]) [Zero] = 0x7148
                    DerefOf (PCCX [One]) [One] = 0xD6D8
                }
                Case (0x19)
                {
                    DerefOf (PCCX [One]) [Zero] = 0x3E80
                    DerefOf (PCCX [One]) [One] = 0x7D00
                }
                Case (0x0F)
                {
                    DerefOf (PCCX [One]) [Zero] = 0x36B0
                    DerefOf (PCCX [One]) [One] = 0x7D00
                }
                Case (0x0B)
                {
                    DerefOf (PCCX [One]) [Zero] = 0x36B0
                    DerefOf (PCCX [One]) [One] = 0x61A8
                }
                Default
                {
                    DerefOf (PCCX [One]) [Zero] = 0xFF
                    DerefOf (PCCX [One]) [One] = 0xFF
                }

            }

            Return (PCCX) /* \_SB_.PC00.TCPU.PCCX */
        }

        Name (PCCX, Package (0x02)
        {
            0x80000000, 
            Package (0x02)
            {
                0x80000000, 
                0x80000000
            }
        })
        Name (KEFF, Package (0x1E)
        {
            Package (0x02)
            {
                0x01BC, 
                Zero
            }, 

            Package (0x02)
            {
                0x01CF, 
                0x27
            }, 

            Package (0x02)
            {
                0x01E1, 
                0x4B
            }, 

            Package (0x02)
            {
                0x01F3, 
                0x6C
            }, 

            Package (0x02)
            {
                0x0206, 
                0x8B
            }, 

            Package (0x02)
            {
                0x0218, 
                0xA8
            }, 

            Package (0x02)
            {
                0x022A, 
                0xC3
            }, 

            Package (0x02)
            {
                0x023D, 
                0xDD
            }, 

            Package (0x02)
            {
                0x024F, 
                0xF4
            }, 

            Package (0x02)
            {
                0x0261, 
                0x010B
            }, 

            Package (0x02)
            {
                0x0274, 
                0x011F
            }, 

            Package (0x02)
            {
                0x032C, 
                0x01BD
            }, 

            Package (0x02)
            {
                0x03D7, 
                0x0227
            }, 

            Package (0x02)
            {
                0x048B, 
                0x026D
            }, 

            Package (0x02)
            {
                0x053E, 
                0x02A1
            }, 

            Package (0x02)
            {
                0x05F7, 
                0x02C6
            }, 

            Package (0x02)
            {
                0x06A8, 
                0x02E6
            }, 

            Package (0x02)
            {
                0x075D, 
                0x02FF
            }, 

            Package (0x02)
            {
                0x0818, 
                0x0311
            }, 

            Package (0x02)
            {
                0x08CF, 
                0x0322
            }, 

            Package (0x02)
            {
                0x179C, 
                0x0381
            }, 

            Package (0x02)
            {
                0x2DDC, 
                0x039C
            }, 

            Package (0x02)
            {
                0x44A8, 
                0x039E
            }, 

            Package (0x02)
            {
                0x5C35, 
                0x0397
            }, 

            Package (0x02)
            {
                0x747D, 
                0x038D
            }, 

            Package (0x02)
            {
                0x8D7F, 
                0x0382
            }, 

            Package (0x02)
            {
                0xA768, 
                0x0376
            }, 

            Package (0x02)
            {
                0xC23B, 
                0x0369
            }, 

            Package (0x02)
            {
                0xDE26, 
                0x035A
            }, 

            Package (0x02)
            {
                0xFB7C, 
                0x034A
            }
        })
        Name (CEUP, Package (0x06)
        {
            0x80000000, 
            0x80000000, 
            0x80000000, 
            0x80000000, 
            0x80000000, 
            0x80000000
        })
        Method (TMPX, 0, Serialized)
        {
            Return (\_SB.IETM.CTOK (PCTP))
        }

        Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
        {
            LSTM = Arg0
            Notify (\_SB.PC00.TCPU, 0x91) // Device-Specific
        }

        Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
        {
            Return (0x0ADE)
        }

        Name (PTYP, Zero)
        Method (_PSS, 0, NotSerialized)  // _PSS: Performance Supported States
        {
            If (CondRefOf (\_SB.PR00._PSS))
            {
                Return (\_SB.PR00._PSS ())
            }
            Else
            {
                Return (Package (0x02)
                {
                    Package (0x06)
                    {
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero
                    }, 

                    Package (0x06)
                    {
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero
                    }
                })
            }
        }

        Method (_TSS, 0, NotSerialized)  // _TSS: Throttling Supported States
        {
            If (CondRefOf (\_SB.PR00._TSS))
            {
                Return (\_SB.PR00._TSS ())
            }
            Else
            {
                Return (Package (0x01)
                {
                    Package (0x05)
                    {
                        One, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero
                    }
                })
            }
        }

        Method (_TPC, 0, NotSerialized)  // _TPC: Throttling Present Capabilities
        {
            If (CondRefOf (\_SB.PR00._TPC))
            {
                Return (\_SB.PR00._TPC) /* External reference */
            }
            Else
            {
                Return (Zero)
            }
        }

        Method (_PTC, 0, NotSerialized)  // _PTC: Processor Throttling Control
        {
            If ((CondRefOf (\PF00) && (\PF00 != 0x80000000)))
            {
                If ((\PF00 & 0x04))
                {
                    Return (Package (0x02)
                    {
                        ResourceTemplate ()
                        {
                            Register (FFixedHW, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (FFixedHW, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }
                    })
                }
                Else
                {
                    Return (Package (0x02)
                    {
                        ResourceTemplate ()
                        {
                            Register (SystemIO, 
                                0x05,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000001810, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (SystemIO, 
                                0x05,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000001810, // Address
                                ,)
                        }
                    })
                }
            }
            Else
            {
                Return (Package (0x02)
                {
                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }, 

                    ResourceTemplate ()
                    {
                        Register (FFixedHW, 
                            0x00,               // Bit Width
                            0x00,               // Bit Offset
                            0x0000000000000000, // Address
                            ,)
                    }
                })
            }
        }

        Method (_TSD, 0, NotSerialized)  // _TSD: Throttling State Dependencies
        {
            If (CondRefOf (\_SB.PR00._TSD))
            {
                Return (\_SB.PR00._TSD ())
            }
            Else
            {
                Return (Package (0x01)
                {
                    Package (0x05)
                    {
                        0x05, 
                        Zero, 
                        Zero, 
                        0xFC, 
                        Zero
                    }
                })
            }
        }

        Method (_TDL, 0, NotSerialized)  // _TDL: T-State Depth Limit
        {
            If ((CondRefOf (\_SB.PR00._TSS) && CondRefOf (\_SB.CFGD)))
            {
                If ((\_SB.CFGD & 0x2000))
                {
                    Return ((SizeOf (\_SB.PR00.TSMF) - One))
                }
                Else
                {
                    Return ((SizeOf (\_SB.PR00.TSMC) - One))
                }
            }
            Else
            {
                Return (Zero)
            }
        }

        Method (_PDL, 0, NotSerialized)  // _PDL: P-state Depth Limit
        {
            If (CondRefOf (\_SB.PR00._PSS))
            {
                If ((\_SB.OSCP & 0x0400))
                {
                    Return ((SizeOf (\_SB.PR00.TPSS) - One))
                }
                Else
                {
                    Return ((SizeOf (\_SB.PR00.LPSS) - One))
                }
            }
            Else
            {
                Return (Zero)
            }
        }

        Name (TJMX, 0x6E)
        Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
        {
            Return (Zero)
        }

        Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
        {
            Local1 = \_SB.IETM.CTOK (TJMX)
            Local1 -= 0x0A
            If ((LSTM >= Local1))
            {
                Return ((Local1 - 0x14))
            }
            Else
            {
                Return (Local1)
            }
        }

        Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
        {
            Local1 = \_SB.IETM.CTOK (TJMX)
            Local1 -= 0x1E
            If ((LSTM >= Local1))
            {
                Return ((Local1 - 0x14))
            }
            Else
            {
                Return (Local1)
            }
        }

        Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
        {
            Local1 = \_SB.IETM.CTOK (TJMX)
            Local1 -= 0x28
            If ((LSTM >= Local1))
            {
                Return ((Local1 - 0x14))
            }
            Else
            {
                Return (Local1)
            }
        }

        Method (_AC3, 0, Serialized)  // _ACx: Active Cooling, x=0-9
        {
            Local1 = \_SB.IETM.CTOK (TJMX)
            Local1 -= 0x37
            If ((LSTM >= Local1))
            {
                Return ((Local1 - 0x14))
            }
            Else
            {
                Return (Local1)
            }
        }

        Method (_AC4, 0, Serialized)  // _ACx: Active Cooling, x=0-9
        {
            Local1 = \_SB.IETM.CTOK (TJMX)
            Local1 -= 0x46
            If ((LSTM >= Local1))
            {
                Return ((Local1 - 0x14))
            }
            Else
            {
                Return (Local1)
            }
        }

        Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
        {
            Return (\_SB.IETM.CTOK (TJMX))
        }

        Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
        {
            Return (\_SB.IETM.CTOK (TJMX))
        }

        Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
        {
            Return (\_SB.IETM.CTOK (TJMX))
        }

        Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
        {
            Return (\_SB.IETM.CTOK (TJMX))
        }

        Method (UVTH, 1, Serialized)
        {
            If (((\ECON == One) && (\_SB.PC00.LPCB.H_EC.ECAV == One)))
            {
                \_SB.PC00.LPCB.H_EC.ECWT (Arg0, RefOf (\_SB.PC00.LPCB.H_EC.UVTH))
                \_SB.PC00.LPCB.H_EC.ECMD (0x17)
            }
        }
    }

    Scope (\_SB.IETM)
    {
        Name (CTSP, Package (0x01)
        {
            ToUUID ("e145970a-e4c1-4d73-900e-c9c5a69dd067") /* Unknown UUID */
        })
    }

    Scope (\_SB.PC00.TCPU)
    {
        Method (TDPL, 0, Serialized)
        {
            Name (AAAA, Zero)
            Name (BBBB, Zero)
            Name (CCCC, Zero)
            Local0 = CTNL /* \_SB_.PC00.TCPU.CTNL */
            If (((Local0 == One) || (Local0 == 0x02)))
            {
                Local0 = \_SB.CLVL /* External reference */
            }
            Else
            {
                Return (Package (0x01)
                {
                    Zero
                })
            }

            If ((CLCK == One))
            {
                Local0 = One
            }

            AAAA = CPNU (\_SB.PL10, One)
            BBBB = CPNU (\_SB.PL11, One)
            CCCC = CPNU (\_SB.PL12, One)
            Name (TMP1, Package (0x01)
            {
                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }
            })
            Name (TMP2, Package (0x02)
            {
                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }, 

                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }
            })
            Name (TMP3, Package (0x03)
            {
                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }, 

                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }, 

                Package (0x05)
                {
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000, 
                    0x80000000
                }
            })
            If ((Local0 == 0x03))
            {
                If ((AAAA > BBBB))
                {
                    If ((AAAA > CCCC))
                    {
                        If ((BBBB > CCCC))
                        {
                            Local3 = Zero
                            LEV0 = Zero
                            Local4 = One
                            LEV1 = One
                            Local5 = 0x02
                            LEV2 = 0x02
                        }
                        Else
                        {
                            Local3 = Zero
                            LEV0 = Zero
                            Local5 = One
                            LEV1 = 0x02
                            Local4 = 0x02
                            LEV2 = One
                        }
                    }
                    Else
                    {
                        Local5 = Zero
                        LEV0 = 0x02
                        Local3 = One
                        LEV1 = Zero
                        Local4 = 0x02
                        LEV2 = One
                    }
                }
                ElseIf ((BBBB > CCCC))
                {
                    If ((AAAA > CCCC))
                    {
                        Local4 = Zero
                        LEV0 = One
                        Local3 = One
                        LEV1 = Zero
                        Local5 = 0x02
                        LEV2 = 0x02
                    }
                    Else
                    {
                        Local4 = Zero
                        LEV0 = One
                        Local5 = One
                        LEV1 = 0x02
                        Local3 = 0x02
                        LEV2 = Zero
                    }
                }
                Else
                {
                    Local5 = Zero
                    LEV0 = 0x02
                    Local4 = One
                    LEV1 = One
                    Local3 = 0x02
                    LEV2 = Zero
                }

                Local1 = (\_SB.TAR0 + One)
                Local2 = (Local1 * 0x64)
                DerefOf (TMP3 [Local3]) [Zero] = AAAA /* \_SB_.PC00.TCPU.TDPL.AAAA */
                DerefOf (TMP3 [Local3]) [One] = Local2
                DerefOf (TMP3 [Local3]) [0x02] = \_SB.CTC0 /* External reference */
                DerefOf (TMP3 [Local3]) [0x03] = Local1
                DerefOf (TMP3 [Local3]) [0x04] = Zero
                Local1 = (\_SB.TAR1 + One)
                Local2 = (Local1 * 0x64)
                DerefOf (TMP3 [Local4]) [Zero] = BBBB /* \_SB_.PC00.TCPU.TDPL.BBBB */
                DerefOf (TMP3 [Local4]) [One] = Local2
                DerefOf (TMP3 [Local4]) [0x02] = \_SB.CTC1 /* External reference */
                DerefOf (TMP3 [Local4]) [0x03] = Local1
                DerefOf (TMP3 [Local4]) [0x04] = Zero
                Local1 = (\_SB.TAR2 + One)
                Local2 = (Local1 * 0x64)
                DerefOf (TMP3 [Local5]) [Zero] = CCCC /* \_SB_.PC00.TCPU.TDPL.CCCC */
                DerefOf (TMP3 [Local5]) [One] = Local2
                DerefOf (TMP3 [Local5]) [0x02] = \_SB.CTC2 /* External reference */
                DerefOf (TMP3 [Local5]) [0x03] = Local1
                DerefOf (TMP3 [Local5]) [0x04] = Zero
                Return (TMP3) /* \_SB_.PC00.TCPU.TDPL.TMP3 */
            }

            If ((Local0 == 0x02))
            {
                If ((AAAA > BBBB))
                {
                    Local3 = Zero
                    Local4 = One
                    LEV0 = Zero
                    LEV1 = One
                    LEV2 = Zero
                }
                Else
                {
                    Local4 = Zero
                    Local3 = One
                    LEV0 = One
                    LEV1 = Zero
                    LEV2 = Zero
                }

                Local1 = (\_SB.TAR0 + One)
                Local2 = (Local1 * 0x64)
                DerefOf (TMP2 [Local3]) [Zero] = AAAA /* \_SB_.PC00.TCPU.TDPL.AAAA */
                DerefOf (TMP2 [Local3]) [One] = Local2
                DerefOf (TMP2 [Local3]) [0x02] = \_SB.CTC0 /* External reference */
                DerefOf (TMP2 [Local3]) [0x03] = Local1
                DerefOf (TMP2 [Local3]) [0x04] = Zero
                Local1 = (\_SB.TAR1 + One)
                Local2 = (Local1 * 0x64)
                DerefOf (TMP2 [Local4]) [Zero] = BBBB /* \_SB_.PC00.TCPU.TDPL.BBBB */
                DerefOf (TMP2 [Local4]) [One] = Local2
                DerefOf (TMP2 [Local4]) [0x02] = \_SB.CTC1 /* External reference */
                DerefOf (TMP2 [Local4]) [0x03] = Local1
                DerefOf (TMP2 [Local4]) [0x04] = Zero
                Return (TMP2) /* \_SB_.PC00.TCPU.TDPL.TMP2 */
            }

            If ((Local0 == One))
            {
                Switch (ToInteger (\_SB.CBMI))
                {
                    Case (Zero)
                    {
                        Local1 = (\_SB.TAR0 + One)
                        Local2 = (Local1 * 0x64)
                        DerefOf (TMP1 [Zero]) [Zero] = AAAA /* \_SB_.PC00.TCPU.TDPL.AAAA */
                        DerefOf (TMP1 [Zero]) [One] = Local2
                        DerefOf (TMP1 [Zero]) [0x02] = \_SB.CTC0 /* External reference */
                        DerefOf (TMP1 [Zero]) [0x03] = Local1
                        DerefOf (TMP1 [Zero]) [0x04] = Zero
                        LEV0 = Zero
                        LEV1 = Zero
                        LEV2 = Zero
                    }
                    Case (One)
                    {
                        Local1 = (\_SB.TAR1 + One)
                        Local2 = (Local1 * 0x64)
                        DerefOf (TMP1 [Zero]) [Zero] = BBBB /* \_SB_.PC00.TCPU.TDPL.BBBB */
                        DerefOf (TMP1 [Zero]) [One] = Local2
                        DerefOf (TMP1 [Zero]) [0x02] = \_SB.CTC1 /* External reference */
                        DerefOf (TMP1 [Zero]) [0x03] = Local1
                        DerefOf (TMP1 [Zero]) [0x04] = Zero
                        LEV0 = One
                        LEV1 = One
                        LEV2 = One
                    }
                    Case (0x02)
                    {
                        Local1 = (\_SB.TAR2 + One)
                        Local2 = (Local1 * 0x64)
                        DerefOf (TMP1 [Zero]) [Zero] = CCCC /* \_SB_.PC00.TCPU.TDPL.CCCC */
                        DerefOf (TMP1 [Zero]) [One] = Local2
                        DerefOf (TMP1 [Zero]) [0x02] = \_SB.CTC2 /* External reference */
                        DerefOf (TMP1 [Zero]) [0x03] = Local1
                        DerefOf (TMP1 [Zero]) [0x04] = Zero
                        LEV0 = 0x02
                        LEV1 = 0x02
                        LEV2 = 0x02
                    }

                }

                Return (TMP1) /* \_SB_.PC00.TCPU.TDPL.TMP1 */
            }

            Return (Zero)
        }

        Name (MAXT, Zero)
        Method (TDPC, 0, NotSerialized)
        {
            Return (MAXT) /* \_SB_.PC00.TCPU.MAXT */
        }

        Name (LEV0, Zero)
        Name (LEV1, Zero)
        Name (LEV2, Zero)
        Method (STDP, 1, Serialized)
        {
            If ((Arg0 >= \_SB.CLVL))
            {
                Return (Zero)
            }

            Switch (ToInteger (Arg0))
            {
                Case (Zero)
                {
                    Local0 = LEV0 /* \_SB_.PC00.TCPU.LEV0 */
                }
                Case (One)
                {
                    Local0 = LEV1 /* \_SB_.PC00.TCPU.LEV1 */
                }
                Case (0x02)
                {
                    Local0 = LEV2 /* \_SB_.PC00.TCPU.LEV2 */
                }

            }

            Switch (ToInteger (Local0))
            {
                Case (Zero)
                {
                    CPL0 ()
                }
                Case (One)
                {
                    CPL1 ()
                }
                Case (0x02)
                {
                    CPL2 ()
                }

            }

            Notify (\_SB.PC00.TCPU, 0x83) // Device-Specific Change
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (TFN1)
        {
            Name (_UID, "TFN1")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Fan 1"))  // _STR: Description String
            Name (PTYP, 0x04)
            Name (FON, One)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((FND1 == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_FIF, 0, NotSerialized)  // _FIF: Fan Information
            {
                Return (Package (0x04)
                {
                    Zero, 
                    One, 
                    0x02, 
                    Zero
                })
            }

            Method (_FPS, 0, NotSerialized)  // _FPS: Fan Performance States
            {
                Return (Package (0x0D)
                {
                    Zero, 
                    Package (0x05)
                    {
                        0x64, 
                        0xFFFFFFFF, 
                        0x2EE0, 
                        0x01F4, 
                        0x1388
                    }, 

                    Package (0x05)
                    {
                        0x5F, 
                        0xFFFFFFFF, 
                        0x2D50, 
                        0x01DB, 
                        0x128E
                    }, 

                    Package (0x05)
                    {
                        0x5A, 
                        0xFFFFFFFF, 
                        0x2BC0, 
                        0x01C2, 
                        0x1194
                    }, 

                    Package (0x05)
                    {
                        0x55, 
                        0xFFFFFFFF, 
                        0x2904, 
                        0x01A9, 
                        0x109A
                    }, 

                    Package (0x05)
                    {
                        0x50, 
                        0xFFFFFFFF, 
                        0x2648, 
                        0x0190, 
                        0x0FA0
                    }, 

                    Package (0x05)
                    {
                        0x46, 
                        0xFFFFFFFF, 
                        0x2454, 
                        0x015E, 
                        0x0DAC
                    }, 

                    Package (0x05)
                    {
                        0x3C, 
                        0xFFFFFFFF, 
                        0x1CE8, 
                        0x012C, 
                        0x0BB8
                    }, 

                    Package (0x05)
                    {
                        0x32, 
                        0xFFFFFFFF, 
                        0x189C, 
                        0xFA, 
                        0x09C4
                    }, 

                    Package (0x05)
                    {
                        0x28, 
                        0xFFFFFFFF, 
                        0x13EC, 
                        0xC8, 
                        0x07D0
                    }, 

                    Package (0x05)
                    {
                        0x1E, 
                        0xFFFFFFFF, 
                        0x0ED8, 
                        0x96, 
                        0x05DC
                    }, 

                    Package (0x05)
                    {
                        0x19, 
                        0xFFFFFFFF, 
                        0x0C80, 
                        0x7D, 
                        0x04E2
                    }, 

                    Package (0x05)
                    {
                        Zero, 
                        0xFFFFFFFF, 
                        Zero, 
                        Zero, 
                        Zero
                    }
                })
            }

            Name (FSLV, Zero)
            Method (_FSL, 1, Serialized)  // _FSL: Fan Set Level
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    If ((Arg0 != \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV))))
                    {
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.PPSL))
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.PPSH))
                        \_SB.PC00.LPCB.H_EC.ECWT (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV)), RefOf (\_SB.PC00.LPCB.H_EC.PINV))
                        \_SB.PC00.LPCB.H_EC.ECWT (Arg0, RefOf (\_SB.PC00.LPCB.H_EC.PENV))
                        FSLV = Arg0
                        \_SB.PC00.LPCB.H_EC.ECWT (0x64, RefOf (\_SB.PC00.LPCB.H_EC.PSTP))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x1A)
                    }
                }
            }

            Name (TFST, Package (0x03)
            {
                Zero, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            })
            Method (_FST, 0, Serialized)  // _FST: Fan Status
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    TFST [One] = FSLV /* \_SB_.PC00.LPCB.H_EC.TFN1.FSLV */
                    TFST [0x02] = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.CFSP))
                }

                Return (TFST) /* \_SB_.PC00.LPCB.H_EC.TFN1.TFST */
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (TFN2)
        {
            Name (_HID, "INTC1048")  // _HID: Hardware ID
            Name (_UID, "TFN2")  // _UID: Unique ID
            Name (_STR, Unicode ("DDR Fan"))  // _STR: Description String
            Name (PTYP, 0x04)
            Name (FON, One)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((FND2 == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_FIF, 0, NotSerialized)  // _FIF: Fan Information
            {
                Return (Package (0x04)
                {
                    Zero, 
                    One, 
                    0x02, 
                    Zero
                })
            }

            Method (_FPS, 0, NotSerialized)  // _FPS: Fan Performance States
            {
                Return (Package (0x0D)
                {
                    Zero, 
                    Package (0x05)
                    {
                        0x64, 
                        0xFFFFFFFF, 
                        0x2EE0, 
                        0x01F4, 
                        0x1388
                    }, 

                    Package (0x05)
                    {
                        0x5F, 
                        0xFFFFFFFF, 
                        0x2D50, 
                        0x01DB, 
                        0x128E
                    }, 

                    Package (0x05)
                    {
                        0x5A, 
                        0xFFFFFFFF, 
                        0x2BC0, 
                        0x01C2, 
                        0x1194
                    }, 

                    Package (0x05)
                    {
                        0x55, 
                        0xFFFFFFFF, 
                        0x2904, 
                        0x01A9, 
                        0x109A
                    }, 

                    Package (0x05)
                    {
                        0x50, 
                        0xFFFFFFFF, 
                        0x2648, 
                        0x0190, 
                        0x0FA0
                    }, 

                    Package (0x05)
                    {
                        0x46, 
                        0xFFFFFFFF, 
                        0x2454, 
                        0x015E, 
                        0x0DAC
                    }, 

                    Package (0x05)
                    {
                        0x3C, 
                        0xFFFFFFFF, 
                        0x1CE8, 
                        0x012C, 
                        0x0BB8
                    }, 

                    Package (0x05)
                    {
                        0x32, 
                        0xFFFFFFFF, 
                        0x189C, 
                        0xFA, 
                        0x09C4
                    }, 

                    Package (0x05)
                    {
                        0x28, 
                        0xFFFFFFFF, 
                        0x13EC, 
                        0xC8, 
                        0x07D0
                    }, 

                    Package (0x05)
                    {
                        0x1E, 
                        0xFFFFFFFF, 
                        0x0ED8, 
                        0x96, 
                        0x05DC
                    }, 

                    Package (0x05)
                    {
                        0x19, 
                        0xFFFFFFFF, 
                        0x0C80, 
                        0x7D, 
                        0x04E2
                    }, 

                    Package (0x05)
                    {
                        Zero, 
                        0xFFFFFFFF, 
                        Zero, 
                        Zero, 
                        Zero
                    }
                })
            }

            Name (FSLV, Zero)
            Method (_FSL, 1, Serialized)  // _FSL: Fan Set Level
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    If ((Arg0 != \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV))))
                    {
                        \_SB.PC00.LPCB.H_EC.ECWT (One, RefOf (\_SB.PC00.LPCB.H_EC.PPSL))
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.PPSH))
                        \_SB.PC00.LPCB.H_EC.ECWT (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV)), RefOf (\_SB.PC00.LPCB.H_EC.PINV))
                        \_SB.PC00.LPCB.H_EC.ECWT (Arg0, RefOf (\_SB.PC00.LPCB.H_EC.PENV))
                        FSLV = Arg0
                        \_SB.PC00.LPCB.H_EC.ECWT (0x64, RefOf (\_SB.PC00.LPCB.H_EC.PSTP))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x1A)
                    }
                }
            }

            Name (TFST, Package (0x03)
            {
                Zero, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            })
            Method (_FST, 0, Serialized)  // _FST: Fan Status
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    TFST [One] = FSLV /* \_SB_.PC00.LPCB.H_EC.TFN2.FSLV */
                    TFST [0x02] = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.DFSP))
                }

                Return (TFST) /* \_SB_.PC00.LPCB.H_EC.TFN2.TFST */
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (TFN3)
        {
            Name (_HID, "INTC1048")  // _HID: Hardware ID
            Name (_UID, "TFN3")  // _UID: Unique ID
            Name (_STR, Unicode ("GFX Fan"))  // _STR: Description String
            Name (PTYP, 0x04)
            Name (FON, One)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((FND3 == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_FIF, 0, NotSerialized)  // _FIF: Fan Information
            {
                Return (Package (0x04)
                {
                    Zero, 
                    One, 
                    0x02, 
                    Zero
                })
            }

            Method (_FPS, 0, NotSerialized)  // _FPS: Fan Performance States
            {
                Return (Package (0x0D)
                {
                    Zero, 
                    Package (0x05)
                    {
                        0x64, 
                        0xFFFFFFFF, 
                        0x2EE0, 
                        0x01F4, 
                        0x1388
                    }, 

                    Package (0x05)
                    {
                        0x5F, 
                        0xFFFFFFFF, 
                        0x2D50, 
                        0x01DB, 
                        0x128E
                    }, 

                    Package (0x05)
                    {
                        0x5A, 
                        0xFFFFFFFF, 
                        0x2BC0, 
                        0x01C2, 
                        0x1194
                    }, 

                    Package (0x05)
                    {
                        0x55, 
                        0xFFFFFFFF, 
                        0x2904, 
                        0x01A9, 
                        0x109A
                    }, 

                    Package (0x05)
                    {
                        0x50, 
                        0xFFFFFFFF, 
                        0x2648, 
                        0x0190, 
                        0x0FA0
                    }, 

                    Package (0x05)
                    {
                        0x46, 
                        0xFFFFFFFF, 
                        0x2454, 
                        0x015E, 
                        0x0DAC
                    }, 

                    Package (0x05)
                    {
                        0x3C, 
                        0xFFFFFFFF, 
                        0x1CE8, 
                        0x012C, 
                        0x0BB8
                    }, 

                    Package (0x05)
                    {
                        0x32, 
                        0xFFFFFFFF, 
                        0x189C, 
                        0xFA, 
                        0x09C4
                    }, 

                    Package (0x05)
                    {
                        0x28, 
                        0xFFFFFFFF, 
                        0x13EC, 
                        0xC8, 
                        0x07D0
                    }, 

                    Package (0x05)
                    {
                        0x1E, 
                        0xFFFFFFFF, 
                        0x0ED8, 
                        0x96, 
                        0x05DC
                    }, 

                    Package (0x05)
                    {
                        0x19, 
                        0xFFFFFFFF, 
                        0x0C80, 
                        0x7D, 
                        0x04E2
                    }, 

                    Package (0x05)
                    {
                        Zero, 
                        0xFFFFFFFF, 
                        Zero, 
                        Zero, 
                        Zero
                    }
                })
            }

            Name (FSLV, Zero)
            Method (_FSL, 1, Serialized)  // _FSL: Fan Set Level
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    If ((Arg0 != \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV))))
                    {
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.PPSL))
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.PPSH))
                        \_SB.PC00.LPCB.H_EC.ECWT (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PENV)), RefOf (\_SB.PC00.LPCB.H_EC.PINV))
                        \_SB.PC00.LPCB.H_EC.ECWT (Arg0, RefOf (\_SB.PC00.LPCB.H_EC.PENV))
                        FSLV = Arg0
                        \_SB.PC00.LPCB.H_EC.ECWT (0x64, RefOf (\_SB.PC00.LPCB.H_EC.PSTP))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x1A)
                    }
                }
            }

            Name (TFST, Package (0x03)
            {
                Zero, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            })
            Method (_FST, 0, Serialized)  // _FST: Fan Status
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    TFST [One] = FSLV /* \_SB_.PC00.LPCB.H_EC.TFN3.FSLV */
                    TFST [0x02] = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.GFSP))
                }

                Return (TFST) /* \_SB_.PC00.LPCB.H_EC.TFN3.TFST */
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (CHRG)
        {
            Name (_UID, "CHRG")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Charger"))  // _STR: Description String
            Name (PTYP, 0x0B)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\CHGE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Name (PSSS, Zero)
            Name (PPPS, Zero)
            Name (PPS1, Package (0x08)
            {
                Package (0x08)
                {
                    0x64, 
                    Zero, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x0DAC, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x55, 
                    Zero, 
                    Zero, 
                    Zero, 
                    One, 
                    0x0BB8, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x47, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x02, 
                    0x09C4, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x39, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x03, 
                    0x07D0, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x2A, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x04, 
                    0x05DC, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x1C, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x05, 
                    0x03E8, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x0E, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x06, 
                    0x01F4, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    Zero, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x07, 
                    Zero, 
                    "MilliAmps", 
                    Zero
                }
            })
            Name (PPS2, Package (0x0A)
            {
                Package (0x08)
                {
                    0x64, 
                    Zero, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x1194, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x58, 
                    Zero, 
                    Zero, 
                    Zero, 
                    One, 
                    0x0FA0, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x4D, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x02, 
                    0x0DAC, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x42, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x03, 
                    0x0BB8, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x37, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x04, 
                    0x09C4, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x2C, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x05, 
                    0x07D0, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x21, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x06, 
                    0x05DC, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x16, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x07, 
                    0x03E8, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    0x0B, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x08, 
                    0x01F4, 
                    "MilliAmps", 
                    Zero
                }, 

                Package (0x08)
                {
                    Zero, 
                    Zero, 
                    Zero, 
                    Zero, 
                    0x09, 
                    Zero, 
                    "MilliAmps", 
                    Zero
                }
            })
            Method (PPSS, 0, Serialized)
            {
                If ((ECRD (RefOf (FCHG)) == One))
                {
                    Return (PPS2) /* \_SB_.PC00.LPCB.H_EC.CHRG.PPS2 */
                }
                Else
                {
                    Return (PPS1) /* \_SB_.PC00.LPCB.H_EC.CHRG.PPS1 */
                }
            }

            Method (PCAL, 0, Serialized)
            {
                If ((ECRD (RefOf (FCHG)) == One))
                {
                    PSSS = SizeOf (PPS2)
                }
                Else
                {
                    PSSS = SizeOf (PPS1)
                }
            }

            Method (PPPC, 0, NotSerialized)
            {
                Return (PPPS) /* \_SB_.PC00.LPCB.H_EC.CHRG.PPPS */
            }

            Method (SPPC, 1, Serialized)
            {
                PCAL ()
                If ((ToInteger (Arg0) <= (PSSS - One)))
                {
                    If ((ECRD (RefOf (FCHG)) == One))
                    {
                        Local1 = DerefOf (DerefOf (PPS2 [Arg0]) [0x05])
                        PPPS = DerefOf (DerefOf (PPS2 [Arg0]) [0x04])
                    }
                    Else
                    {
                        Local1 = DerefOf (DerefOf (PPS1 [Arg0]) [0x05])
                        PPPS = DerefOf (DerefOf (PPS1 [Arg0]) [0x04])
                    }

                    \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.CHGR))
                    \_SB.PC00.LPCB.H_EC.ECMD (0x37)
                }
            }

            Method (PPDL, 0, NotSerialized)
            {
                PCAL ()
                Return ((PSSS - One))
            }
        }
    }

    Scope (\_SB)
    {
        Device (TPWR)
        {
            Name (_UID, "TPWR")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Platform Power"))  // _STR: Description String
            Name (PTYP, 0x11)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\PWRE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (PSOC, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == Zero))
                {
                    Return (Zero)
                }

                If ((\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC)) == Zero))
                {
                    Return (Zero)
                }

                If ((\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1RC)) > \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC))))
                {
                    Return (Zero)
                }

                If ((\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1RC)) == \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC))))
                {
                    Return (0x64)
                }

                If ((\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1RC)) < \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC))))
                {
                    Local0 = (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1RC)) * 0x64)
                    Divide (Local0, \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC)), Local2, Local1)
                    Local2 /= 0x64
                    Local3 = (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.B1FC)) / 0xC8)
                    If ((Local2 >= Local3))
                    {
                        Local1 += One
                    }

                    Return (Local1)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (PSRC, 0, Serialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == Zero))
                {
                    Return (Zero)
                }
                Else
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PWRT))
                    Local1 = (Local0 & 0xF0)
                }

                Switch (ToInteger ((ToInteger (Local0) & 0x07)))
                {
                    Case (Zero)
                    {
                        Local1 |= Zero
                    }
                    Case (One)
                    {
                        Local1 |= One
                    }
                    Case (0x02)
                    {
                        Local1 |= 0x02
                    }
                    Default
                    {
                        Local1 |= Zero
                    }

                }

                Return (Local1)
            }

            Method (ARTG, 0, NotSerialized)
            {
                If (((PSRC () & 0x07) == One))
                {
                    If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                    {
                        Local0 = (\_SB.PC00.LPCB.H_EC.ARTG * 0x0A)
                        Return (Local0)
                    }
                    Else
                    {
                        Return (0x00015F90)
                    }
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (PROP, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = (\_SB.PC00.LPCB.H_EC.PROP * 0x03E8)
                    Return (Local0)
                }
                Else
                {
                    Return (0x61A8)
                }
            }

            Method (PBOK, 1, Serialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = (Arg0 & 0x0F)
                    \_SB.PC00.LPCB.H_EC.ECWT (Local0, RefOf (\_SB.PC00.LPCB.H_EC.PBOK))
                    \_SB.PC00.LPCB.H_EC.ECMD (0x15)
                }
            }
        }
    }

    Scope (\_SB)
    {
        Device (TPCH)
        {
            Name (_UID, "TPCH")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Intel PCH FIVR Participant"))  // _STR: Description String
            Name (PTYP, 0x05)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\PCHE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (RFC0, 1, Serialized)
            {
                IPCS (0xA3, One, 0x08, Zero, Arg0, Zero, Zero)
                Return (Package (0x01)
                {
                    Zero
                })
            }

            Method (RFC1, 1, Serialized)
            {
                IPCS (0xA3, One, 0x08, One, Arg0, Zero, Zero)
                Return (Package (0x01)
                {
                    Zero
                })
            }

            Method (SEMI, 1, Serialized)
            {
                IPCS (0xA3, One, 0x08, 0x02, Arg0, Zero, Zero)
                Return (Package (0x01)
                {
                    Zero
                })
            }

            Method (PKGC, 1, Serialized)
            {
                Name (PPKG, Package (0x02)
                {
                    Zero, 
                    Zero
                })
                PPKG [Zero] = DerefOf (Arg0 [Zero])
                PPKG [One] = DerefOf (Arg0 [One])
                Return (PPKG) /* \_SB_.TPCH.PKGC.PPKG */
            }

            Method (GFC0, 0, Serialized)
            {
                Local0 = IPCS (0xA3, Zero, 0x08, Zero, Zero, Zero, Zero)
                Local1 = \_SB.TPCH.PKGC (Local0)
                Return (Local1)
            }

            Method (GFC1, 0, Serialized)
            {
                Local0 = IPCS (0xA3, Zero, 0x08, One, Zero, Zero, Zero)
                Local1 = \_SB.TPCH.PKGC (Local0)
                Return (Local1)
            }

            Method (GEMI, 0, Serialized)
            {
                Local0 = IPCS (0xA3, Zero, 0x08, 0x02, Zero, Zero, Zero)
                Local1 = \_SB.TPCH.PKGC (Local0)
                Return (Local1)
            }

            Method (GFFS, 0, Serialized)
            {
                Local0 = IPCS (0xA3, Zero, 0x08, 0x03, Zero, Zero, Zero)
                Local1 = \_SB.TPCH.PKGC (Local0)
                Return (Local1)
            }

            Method (GFCS, 0, Serialized)
            {
                Local0 = IPCS (0xA3, Zero, 0x08, 0x04, Zero, Zero, Zero)
                Local1 = \_SB.TPCH.PKGC (Local0)
                Return (Local1)
            }
        }
    }

    Scope (\_SB)
    {
        Device (BAT1)
        {
            Name (_UID, "1")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Battery 1 Participant"))  // _STR: Description String
            Name (PTYP, 0x0C)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\BATR == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (PMAX, 0, Serialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.BMAX))
                    If (Local0)
                    {
                        Local0 = ~Local0 |= 0xFFFF0000
                        Local0 = (Local0 += One * 0x0A)
                    }

                    Return (Local0)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (CTYP, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Return (\_SB.PC00.LPCB.H_EC.CTYP) /* External reference */
                }
                Else
                {
                    Return (0x03)
                }
            }

            Method (PBSS, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.PBSS))
                    Return (Local0)
                }

                Return (0x64)
            }

            Method (DPSP, 0, Serialized)
            {
                Return (\PPPR) /* External reference */
            }

            Method (RBHF, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.RBHF))
                    Return (Local0)
                }

                Return (0xFFFFFFFF)
            }

            Method (VBNL, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.VBNL))
                    Return (Local0)
                }

                Return (0xFFFFFFFF)
            }

            Method (CMPP, 0, NotSerialized)
            {
                If ((\_SB.PC00.LPCB.H_EC.ECAV == One))
                {
                    Local0 = \_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.CMPP))
                    Return (Local0)
                }

                Return (0xFFFFFFFF)
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (SEN1)
        {
            Name (_UID, "SEN1")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Thermistor PCH VR"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S1DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.C10K (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR1))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.SEN1, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S1AC, 0x3C)
            Name (S1A1, 0x32)
            Name (S1A2, 0x28)
            Name (S1PV, 0x41)
            Name (S1CC, 0x50)
            Name (S1C3, 0x46)
            Name (S1HP, 0x4B)
            Name (SSP1, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (SSP1) /* \_SB_.PC00.LPCB.H_EC.SEN1.SSP1 */
            }

            Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S1AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S1A1))
            }

            Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S1A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S1PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S1CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S1C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S1HP))
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (SEN2)
        {
            Name (_UID, "SEN2")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Thermistor GT VR"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S2DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.C10K (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR2))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (One, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (One, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.SEN2, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S2AC, 0x3C)
            Name (S2A1, 0x32)
            Name (S2A2, 0x28)
            Name (S2PV, 0x41)
            Name (S2CC, 0x50)
            Name (S2C3, 0x46)
            Name (S2HP, 0x4B)
            Name (SSP2, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (SSP2) /* \_SB_.PC00.LPCB.H_EC.SEN2.SSP2 */
            }

            Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S2AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S2A1))
            }

            Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S2A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S2PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S2CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S2C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S2HP))
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (SEN3)
        {
            Name (_UID, "SEN3")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Thermistor Ambient"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S3DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.CTOK (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR3))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT ((Local1 / 0x0A), RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT ((Local1 / 0x0A), RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.SEN3, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S3AC, 0x3C)
            Name (S3A1, 0x32)
            Name (S3A2, 0x28)
            Name (S3PV, 0x41)
            Name (S3CC, 0x50)
            Name (S3C3, 0x46)
            Name (S3HP, 0x4B)
            Name (SSP3, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (SSP3) /* \_SB_.PC00.LPCB.H_EC.SEN3.SSP3 */
            }

            Method (_AC3, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S3AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC4, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S3A1))
            }

            Method (_AC5, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S3A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S3PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S3CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S3C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S3HP))
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (SEN4)
        {
            Name (_UID, "SEN4")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Thermistor Battery Charger"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S4DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.C10K (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR4))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (0x03, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (0x03, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.SEN4, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S4AC, 0x3C)
            Name (S4A1, 0x32)
            Name (S4A2, 0x28)
            Name (S4PV, 0x41)
            Name (S4CC, 0x50)
            Name (S4C3, 0x46)
            Name (S4HP, 0x4B)
            Name (SSP4, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (SSP4) /* \_SB_.PC00.LPCB.H_EC.SEN4.SSP4 */
            }

            Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S4AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S4A1))
            }

            Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S4A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S4PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S4CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S4C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S4HP))
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (SEN5)
        {
            Name (_UID, "SEN5")  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return (\_SB.IETM.GHID (_UID))
            }

            Name (_STR, Unicode ("Thermistor Memory"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S5DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.C10K (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR5))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (0x04, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (0x04, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.SEN5, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S5AC, 0x3C)
            Name (S5A1, 0x32)
            Name (S5A2, 0x28)
            Name (S5PV, 0x41)
            Name (S5CC, 0x50)
            Name (S5C3, 0x46)
            Name (S5HP, 0x4B)
            Name (SSP5, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (SSP5) /* \_SB_.PC00.LPCB.H_EC.SEN5.SSP5 */
            }

            Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S5AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S5A1))
            }

            Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S5A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S5PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S5CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S5C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S5HP))
            }
        }
    }

    Scope (\_SB.PC00.LPCB.H_EC)
    {
        Device (DGPU)
        {
            Name (_HID, "INTC1046")  // _HID: Hardware ID
            Name (_UID, "DGPU")  // _UID: Unique ID
            Name (_STR, Unicode ("Discrete Gpu Sensor"))  // _STR: Description String
            Name (PTYP, 0x03)
            Name (CTYP, Zero)
            Name (PFLG, Zero)
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((\S6DE == One))
                {
                    Return (0x0F)
                }
                Else
                {
                    Return (Zero)
                }
            }

            Method (_TMP, 0, Serialized)  // _TMP: Temperature
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Return (\_SB.IETM.C10K (\_SB.PC00.LPCB.H_EC.ECRD (RefOf (\_SB.PC00.LPCB.H_EC.TSR1))))
                }
                Else
                {
                    Return (0x0BB8)
                }
            }

            Name (PATC, 0x02)
            Method (PAT0, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSLT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Method (PAT1, 1, Serialized)
            {
                If (\_SB.PC00.LPCB.H_EC.ECAV)
                {
                    Local0 = Acquire (\_SB.PC00.LPCB.H_EC.PATM, 0x0064)
                    If ((Local0 == Zero))
                    {
                        Local1 = \_SB.IETM.K10C (Arg0)
                        \_SB.PC00.LPCB.H_EC.ECWT (Zero, RefOf (\_SB.PC00.LPCB.H_EC.TSI))
                        \_SB.PC00.LPCB.H_EC.ECWT (0x02, RefOf (\_SB.PC00.LPCB.H_EC.HYST))
                        \_SB.PC00.LPCB.H_EC.ECWT (Local1, RefOf (\_SB.PC00.LPCB.H_EC.TSHT))
                        \_SB.PC00.LPCB.H_EC.ECMD (0x4A)
                        Release (\_SB.PC00.LPCB.H_EC.PATM)
                    }
                }
            }

            Name (GTSH, 0x14)
            Name (LSTM, Zero)
            Method (_DTI, 1, NotSerialized)  // _DTI: Device Temperature Indication
            {
                LSTM = Arg0
                Notify (\_SB.PC00.LPCB.H_EC.DGPU, 0x91) // Device-Specific
            }

            Method (_NTT, 0, NotSerialized)  // _NTT: Notification Temperature Threshold
            {
                Return (0x0ADE)
            }

            Name (S6AC, 0x3C)
            Name (S6A1, 0x32)
            Name (S6A2, 0x28)
            Name (S6PV, 0x41)
            Name (S6CC, 0x50)
            Name (S6C3, 0x46)
            Name (S6HP, 0x4B)
            Name (S6P2, Zero)
            Method (_TSP, 0, Serialized)  // _TSP: Thermal Sampling Period
            {
                Return (S6P2) /* \_SB_.PC00.LPCB.H_EC.DGPU.S6P2 */
            }

            Method (_AC0, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Local1 = \_SB.IETM.CTOK (S6AC)
                If ((LSTM >= Local1))
                {
                    Return ((Local1 - 0x14))
                }
                Else
                {
                    Return (Local1)
                }
            }

            Method (_AC1, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S6A1))
            }

            Method (_AC2, 0, Serialized)  // _ACx: Active Cooling, x=0-9
            {
                Return (\_SB.IETM.CTOK (S6A2))
            }

            Method (_PSV, 0, Serialized)  // _PSV: Passive Temperature
            {
                Return (\_SB.IETM.CTOK (S6PV))
            }

            Method (_CRT, 0, Serialized)  // _CRT: Critical Temperature
            {
                Return (\_SB.IETM.CTOK (S6CC))
            }

            Method (_CR3, 0, Serialized)  // _CR3: Warm/Standby Temperature
            {
                Return (\_SB.IETM.CTOK (S6C3))
            }

            Method (_HOT, 0, Serialized)  // _HOT: Hot Temperature
            {
                Return (\_SB.IETM.CTOK (S6HP))
            }
        }
    }

    Scope (\_SB.IETM)
    {
        Name (TRT0, Package (0x02)
        {
            Package (0x08)
            {
                \_SB.PC00.TCPU, 
                \_SB.PC00.LPCB.H_EC.SEN2, 
                0x28, 
                0x64, 
                Zero, 
                Zero, 
                Zero, 
                Zero
            }, 

            Package (0x08)
            {
                \_SB.PC00.LPCB.H_EC.CHRG, 
                \_SB.PC00.LPCB.H_EC.SEN4, 
                0x14, 
                0xC8, 
                Zero, 
                Zero, 
                Zero, 
                Zero
            }
        })
        Method (_TRT, 0, NotSerialized)  // _TRT: Thermal Relationship Table
        {
            Return (TRT0) /* \_SB_.IETM.TRT0 */
        }
    }

    Scope (\_SB.IETM)
    {
        Name (PTTL, 0x14)
        Name (PSVT, Package (0x05)
        {
            0x02, 
            Package (0x0C)
            {
                \_SB.PC00.LPCB.H_EC.CHRG, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                One, 
                0xC8, 
                0x0C6E, 
                0x0E, 
                0x000A0000, 
                "MAX", 
                One, 
                0x0A, 
                0x0A, 
                Zero
            }, 

            Package (0x0C)
            {
                \_SB.PC00.LPCB.H_EC.CHRG, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                One, 
                0xC8, 
                0x0CA0, 
                0x0E, 
                0x000A0000, 
                One, 
                One, 
                0x0A, 
                0x0A, 
                Zero
            }, 

            Package (0x0C)
            {
                \_SB.PC00.LPCB.H_EC.CHRG, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                One, 
                0xC8, 
                0x0CD2, 
                0x0E, 
                0x000A0000, 
                0x02, 
                One, 
                0x0A, 
                0x0A, 
                Zero
            }, 

            Package (0x0C)
            {
                \_SB.PC00.LPCB.H_EC.CHRG, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                One, 
                0xC8, 
                0x0D36, 
                0x0E, 
                0x000A0000, 
                "MIN", 
                One, 
                0x0A, 
                0x0A, 
                Zero
            }
        })
    }

    Scope (\_SB.IETM)
    {
        Name (ART1, Package (0x06)
        {
            Zero, 
            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.TCPU, 
                0x64, 
                0x50, 
                0x3C, 
                0x28, 
                0x1E, 
                0x14, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN2, 
                0x64, 
                0x50, 
                0x3C, 
                0x1E, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                0x64, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0x50, 
                0x3C, 
                0x1E, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN4, 
                0x64, 
                0x50, 
                0x3C, 
                0x1E, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN5, 
                0x64, 
                0x50, 
                0x3C, 
                0x1E, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }
        })
        Name (ART0, Package (0x06)
        {
            Zero, 
            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.TCPU, 
                0x64, 
                0x64, 
                0x50, 
                0x32, 
                0x28, 
                0x1E, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN2, 
                0x64, 
                0x50, 
                0x32, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN3, 
                0x64, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0x64, 
                0x50, 
                0x32, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN4, 
                0x64, 
                0x64, 
                0x50, 
                0x32, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }, 

            Package (0x0D)
            {
                \_SB.PC00.LPCB.H_EC.TFN1, 
                \_SB.PC00.LPCB.H_EC.SEN5, 
                0x64, 
                0x64, 
                0x50, 
                0x32, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF, 
                0xFFFFFFFF
            }
        })
        Method (_ART, 0, NotSerialized)  // _ART: Active Cooling Relationship Table
        {
            If (\_SB.PC00.LPCB.H_EC.SEN3.CTYP)
            {
                Return (ART1) /* \_SB_.IETM.ART1 */
            }
            Else
            {
                Return (ART0) /* \_SB_.IETM.ART0 */
            }
        }
    }

    Scope (\_SB.IETM)
    {
        Name (DP2P, Package (0x01)
        {
            ToUUID ("9e04115a-ae87-4d1c-9500-0f3e340bfe75") /* Unknown UUID */
        })
        Name (DPSP, Package (0x01)
        {
            ToUUID ("42a441d6-ae6a-462b-a84b-4a8ce79027d3") /* Unknown UUID */
        })
        Name (DASP, Package (0x01)
        {
            ToUUID ("3a95c389-e4b8-4629-a526-c52c88626bae") /* Unknown UUID */
        })
        Name (DA2P, Package (0x01)
        {
            ToUUID ("0e56fab6-bdfc-4e8c-8246-40ecfd4d74ea") /* Unknown UUID */
        })
        Name (DCSP, Package (0x01)
        {
            ToUUID ("97c68ae7-15fa-499c-b8c9-5da81d606e0a") /* Unknown UUID */
        })
        Name (RFIP, Package (0x01)
        {
            ToUUID ("c4ce1849-243a-49f3-b8d5-f97002f38e6a") /* Unknown UUID */
        })
        Name (POBP, Package (0x01)
        {
            ToUUID ("f5a35014-c209-46a4-993a-eb56de7530a1") /* Unknown UUID */
        })
        Name (DAPP, Package (0x01)
        {
            ToUUID ("63be270f-1c11-48fd-a6f7-3af253ff3e2d") /* Unknown UUID */
        })
        Name (DVSP, Package (0x01)
        {
            ToUUID ("6ed722a7-9240-48a5-b479-31eef723d7cf") /* Unknown UUID */
        })
        Name (DPID, Package (0x01)
        {
            ToUUID ("42496e14-bc1b-46e8-a798-ca915464426f") /* Unknown UUID */
        })
    }

    Scope (\_SB.IETM)
    {
        Method (TEVT, 2, Serialized)
        {
            Switch (ToInteger (Arg0))
            {
                Case ("IETM")
                {
                    Notify (\_SB.IETM, Arg1)
                }
                Case ("TCPU")
                {
                    Notify (\_SB.PC00.TCPU, Arg1)
                }
                Case ("TPCH")
                {
                    Notify (\_SB.TPCH, Arg1)
                }

            }

            If (\ECON)
            {
                Switch (ToInteger (Arg0))
                {
                    Case ("CHRG")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.CHRG, Arg1)
                    }
                    Case ("DGPU")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.DGPU, Arg1)
                    }
                    Case ("SEN2")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.SEN2, Arg1)
                    }
                    Case ("SEN3")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.SEN3, Arg1)
                    }
                    Case ("SEN4")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.SEN4, Arg1)
                    }
                    Case ("SEN5")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.SEN5, Arg1)
                    }
                    Case ("TFN1")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.TFN1, Arg1)
                    }
                    Case ("TFN2")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.TFN2, Arg1)
                    }
                    Case ("TFN3")
                    {
                        Notify (\_SB.PC00.LPCB.H_EC.TFN3, Arg1)
                    }
                    Case ("TPWR")
                    {
                        Notify (\_SB.TPWR, Arg1)
                    }

                }
            }
        }
    }

    Scope (\_SB.IETM)
    {
        Method (GDDV, 0, Serialized)
        {
            Switch (ToInteger (CIDE))
            {
                Case (0x70)
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x0300)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0xBF, 0x91, 0x27, 0x06,  // ......'.
                            /* 0070 */  0xEA, 0x3A, 0xCC, 0x52, 0x85, 0xA2, 0x50, 0x87,  // .:.R..P.
                            /* 0078 */  0x55, 0xA7, 0xDE, 0x1D, 0x52, 0xF0, 0x27, 0x39,  // U...R.'9
                            /* 0080 */  0x4F, 0x34, 0xA2, 0xE4, 0x87, 0xEA, 0xE1, 0x50,  // O4.....P
                            /* 0088 */  0x1A, 0xB7, 0x4A, 0xC2, 0x6C, 0x02, 0x00, 0x00,  // ..J.l...
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x40, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00,  // .@......
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x43, 0xED, 0x82,  // u8...C..
                            /* 00D0 */  0xF5, 0xD8, 0x08, 0x03, 0x53, 0x11, 0x4F, 0x34,  // ....S.O4
                            /* 00D8 */  0x76, 0xA3, 0x42, 0x65, 0x95, 0x70, 0xA6, 0x69,  // v.Be.p.i
                            /* 00E0 */  0x09, 0xA5, 0x2D, 0x26, 0xAC, 0x1B, 0x79, 0x3B,  // ..-&..y;
                            /* 00E8 */  0x2D, 0x44, 0x3F, 0xE1, 0x47, 0xF6, 0xF2, 0x2F,  // -D?.G../
                            /* 00F0 */  0x3D, 0x53, 0x79, 0xC1, 0x4A, 0x91, 0x5E, 0xBA,  // =Sy.J.^.
                            /* 00F8 */  0x11, 0xFD, 0x91, 0x72, 0x99, 0x68, 0x61, 0x1D,  // ...r.ha.
                            /* 0100 */  0xDD, 0xE9, 0xEA, 0xB1, 0xCA, 0x4F, 0xA5, 0xED,  // .....O..
                            /* 0108 */  0xFE, 0x19, 0xFA, 0xA7, 0x18, 0xBC, 0x6B, 0x71,  // ......kq
                            /* 0110 */  0xE6, 0x62, 0xCC, 0x4B, 0x81, 0xCF, 0xD1, 0x53,  // .b.K...S
                            /* 0118 */  0x7D, 0x73, 0x54, 0x37, 0x72, 0x11, 0xA1, 0x33,  // }sT7r..3
                            /* 0120 */  0x26, 0x6B, 0x30, 0xC9, 0xD4, 0x17, 0xC4, 0xFF,  // &k0.....
                            /* 0128 */  0x26, 0x86, 0x19, 0xE5, 0x0F, 0x33, 0x2E, 0xF2,  // &....3..
                            /* 0130 */  0x16, 0x5F, 0xD9, 0x53, 0xB4, 0x63, 0xBE, 0x72,  // ._.S.c.r
                            /* 0138 */  0xF7, 0x73, 0xE8, 0x21, 0x0E, 0xC2, 0x62, 0xD7,  // .s.!..b.
                            /* 0140 */  0xCB, 0xF4, 0xA9, 0x95, 0x5D, 0x01, 0x78, 0xB3,  // ....].x.
                            /* 0148 */  0x83, 0xDE, 0x2E, 0x7B, 0x85, 0xE0, 0xED, 0xDC,  // ...{....
                            /* 0150 */  0xE3, 0x3B, 0xD0, 0x95, 0x25, 0x5F, 0x47, 0x77,  // .;..%_Gw
                            /* 0158 */  0x60, 0x65, 0xE9, 0xDD, 0xEB, 0x4E, 0x02, 0x53,  // `e...N.S
                            /* 0160 */  0xB1, 0xA5, 0xAF, 0x7B, 0xBA, 0xF7, 0x6B, 0x39,  // ...{..k9
                            /* 0168 */  0x32, 0xCB, 0xD7, 0x03, 0x66, 0xC5, 0x79, 0xFC,  // 2...f.y.
                            /* 0170 */  0x3B, 0x88, 0x56, 0x5E, 0x59, 0xE1, 0x16, 0x8C,  // ;.V^Y...
                            /* 0178 */  0xBB, 0x2B, 0x3B, 0x83, 0xD3, 0xC8, 0x5D, 0x8F,  // .+;...].
                            /* 0180 */  0x7B, 0xFE, 0x69, 0x19, 0xC9, 0x3C, 0x9D, 0xA0,  // {.i..<..
                            /* 0188 */  0x8B, 0xA0, 0x43, 0x22, 0xC3, 0xBE, 0x4C, 0x14,  // ..C"..L.
                            /* 0190 */  0x88, 0x41, 0x44, 0x5E, 0x18, 0xFA, 0x17, 0xC1,  // .AD^....
                            /* 0198 */  0xDA, 0x17, 0xB3, 0x42, 0x81, 0x7E, 0xA1, 0x7E,  // ...B.~.~
                            /* 01A0 */  0xA4, 0x42, 0xB6, 0x56, 0x3B, 0xCF, 0xB2, 0x08,  // .B.V;...
                            /* 01A8 */  0xAA, 0xCB, 0x42, 0x25, 0x82, 0x45, 0xF4, 0xBF,  // ..B%.E..
                            /* 01B0 */  0xD5, 0xF7, 0xEE, 0x1E, 0xC4, 0x8B, 0xA7, 0x04,  // ........
                            /* 01B8 */  0xB2, 0x0C, 0xE9, 0x18, 0x1A, 0x91, 0xFB, 0xD8,  // ........
                            /* 01C0 */  0x46, 0xAB, 0x39, 0xDB, 0xDD, 0x36, 0x35, 0x13,  // F.9..65.
                            /* 01C8 */  0x68, 0x02, 0x7A, 0x5A, 0xCA, 0x45, 0xEC, 0xFF,  // h.zZ.E..
                            /* 01D0 */  0x19, 0x22, 0x22, 0x48, 0xF0, 0xF8, 0x99, 0xE3,  // .""H....
                            /* 01D8 */  0xDA, 0x30, 0x72, 0x9F, 0x02, 0x8A, 0x63, 0x54,  // .0r...cT
                            /* 01E0 */  0x13, 0x00, 0xAF, 0x22, 0x14, 0xD6, 0x39, 0x55,  // ..."..9U
                            /* 01E8 */  0xA2, 0x61, 0x24, 0x8E, 0x51, 0xBC, 0xA0, 0xF4,  // .a$.Q...
                            /* 01F0 */  0x7B, 0x9F, 0x17, 0xBE, 0xBF, 0x99, 0x5E, 0xB9,  // {.....^.
                            /* 01F8 */  0x58, 0x8A, 0x66, 0x16, 0xC4, 0x51, 0x8F, 0xE3,  // X.f..Q..
                            /* 0200 */  0xFB, 0x82, 0xA8, 0x70, 0x89, 0x18, 0xCB, 0x10,  // ...p....
                            /* 0208 */  0x52, 0xB4, 0x6C, 0xE7, 0xC3, 0x4E, 0x94, 0xF2,  // R.l..N..
                            /* 0210 */  0x86, 0x0C, 0xD9, 0xCD, 0x71, 0x7B, 0xEC, 0x50,  // ....q{.P
                            /* 0218 */  0x3C, 0x88, 0x93, 0xD7, 0xA1, 0x60, 0x5D, 0x18,  // <....`].
                            /* 0220 */  0xE1, 0x10, 0xF4, 0xE8, 0x77, 0x00, 0xEC, 0xB2,  // ....w...
                            /* 0228 */  0xD2, 0xC1, 0x5D, 0x6C, 0xCA, 0x14, 0xD3, 0xD4,  // ..]l....
                            /* 0230 */  0x1C, 0x45, 0xFF, 0xBB, 0x93, 0x85, 0xC8, 0x92,  // .E......
                            /* 0238 */  0x55, 0xFB, 0x48, 0x74, 0x0B, 0xD7, 0x32, 0x8C,  // U.Ht..2.
                            /* 0240 */  0xA8, 0x98, 0x88, 0xE3, 0x55, 0x25, 0x9D, 0xAF,  // ....U%..
                            /* 0248 */  0xAC, 0x72, 0xA6, 0x4F, 0x63, 0xA7, 0x33, 0xD9,  // .r.Oc.3.
                            /* 0250 */  0x67, 0xEE, 0x4C, 0x98, 0xB0, 0xEF, 0x26, 0x41,  // g.L...&A
                            /* 0258 */  0xC4, 0x00, 0x98, 0xAB, 0x91, 0xC8, 0x42, 0x11,  // ......B.
                            /* 0260 */  0x6D, 0x47, 0x5A, 0x77, 0xBB, 0x91, 0x4C, 0x71,  // mGZw..Lq
                            /* 0268 */  0x50, 0x80, 0x8F, 0x43, 0x57, 0x9F, 0xFD, 0xC6,  // P..CW...
                            /* 0270 */  0x3C, 0x3A, 0x85, 0x92, 0xF1, 0x36, 0xCE, 0x00,  // <:...6..
                            /* 0278 */  0xC1, 0x02, 0x07, 0x0A, 0xD6, 0x5E, 0x89, 0x0A,  // .....^..
                            /* 0280 */  0x5A, 0xA8, 0xC6, 0x98, 0x91, 0x27, 0xB2, 0x98,  // Z....'..
                            /* 0288 */  0x81, 0x73, 0x70, 0xB2, 0x28, 0x2A, 0xB8, 0xCF,  // .sp.(*..
                            /* 0290 */  0x5C, 0xC1, 0x0F, 0x7D, 0x35, 0x96, 0x82, 0xDC,  // \..}5...
                            /* 0298 */  0x6F, 0xA3, 0x18, 0x90, 0x14, 0xFE, 0xD4, 0xEB,  // o.......
                            /* 02A0 */  0xB0, 0xEB, 0x0A, 0x32, 0x18, 0x27, 0xD7, 0x68,  // ...2.'.h
                            /* 02A8 */  0x26, 0x84, 0xBE, 0x99, 0x8B, 0x96, 0x0C, 0x6F,  // &......o
                            /* 02B0 */  0x6C, 0x07, 0x1F, 0xE9, 0x2B, 0xC6, 0xC0, 0xB3,  // l...+...
                            /* 02B8 */  0xE6, 0x41, 0x17, 0xCF, 0x49, 0xBC, 0x55, 0xF5,  // .A..I.U.
                            /* 02C0 */  0x95, 0x3F, 0xFC, 0x68, 0xF0, 0xCB, 0xB2, 0x08,  // .?.h....
                            /* 02C8 */  0xA5, 0x16, 0x3C, 0xAB, 0x34, 0xF0, 0xAD, 0x52,  // ..<.4..R
                            /* 02D0 */  0x63, 0xD9, 0x52, 0x4E, 0x8F, 0x2F, 0x3A, 0x24,  // c.RN./:$
                            /* 02D8 */  0x4F, 0x5C, 0xB5, 0x68, 0x5D, 0x29, 0xE5, 0xE3,  // O\.h])..
                            /* 02E0 */  0x0F, 0xF0, 0xCB, 0x2A, 0xEF, 0x76, 0xEA, 0x0A,  // ...*.v..
                            /* 02E8 */  0xE4, 0x41, 0xFF, 0xA3, 0x22, 0xEE, 0xC4, 0x58,  // .A.."..X
                            /* 02F0 */  0x89, 0x77, 0x85, 0x84, 0xDE, 0xBC, 0xB3, 0x3C,  // .w.....<
                            /* 02F8 */  0x94, 0xAA, 0xBA, 0x01, 0xFC, 0xDB, 0x37, 0x00   // ......7.
                        }
                    })
                }
                Case (0x74)
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x0379)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0x36, 0x8E, 0xB1, 0xB5,  // ....6...
                            /* 0070 */  0xFE, 0x5A, 0x71, 0x99, 0xA1, 0xB3, 0x8C, 0xF1,  // .Zq.....
                            /* 0078 */  0x43, 0x9D, 0x76, 0xE1, 0x4C, 0x84, 0xF4, 0xEE,  // C.v.L...
                            /* 0080 */  0x4F, 0x79, 0xDD, 0x18, 0xFF, 0x83, 0x10, 0xE7,  // Oy......
                            /* 0088 */  0x54, 0x99, 0xAF, 0xA4, 0xE5, 0x02, 0x00, 0x00,  // T.......
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x84, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x40, 0xDF, 0x5C,  // u8...@.\
                            /* 00D0 */  0x4F, 0x89, 0xF0, 0xDC, 0xC2, 0x03, 0xE4, 0x33,  // O......3
                            /* 00D8 */  0x41, 0xDE, 0x14, 0x4A, 0x70, 0x1C, 0xD7, 0xB5,  // A..Jp...
                            /* 00E0 */  0xA2, 0xD1, 0xFE, 0x4C, 0xAC, 0x05, 0xED, 0xE7,  // ...L....
                            /* 00E8 */  0x60, 0x6C, 0xF9, 0xAB, 0x9C, 0x07, 0x0E, 0x87,  // `l......
                            /* 00F0 */  0xD1, 0x4E, 0xEC, 0x71, 0x2C, 0xD7, 0x05, 0xA9,  // .N.q,...
                            /* 00F8 */  0xA9, 0x74, 0xB9, 0x89, 0x8E, 0x6A, 0x83, 0x82,  // .t...j..
                            /* 0100 */  0x64, 0xAE, 0xD0, 0x63, 0xD9, 0x0C, 0x26, 0x73,  // d..c..&s
                            /* 0108 */  0x71, 0xEF, 0x94, 0xFE, 0x88, 0x48, 0x55, 0xE6,  // q....HU.
                            /* 0110 */  0x1B, 0x1F, 0x87, 0x22, 0x24, 0x38, 0xA6, 0xF1,  // ..."$8..
                            /* 0118 */  0xE7, 0xC8, 0xF5, 0x64, 0x75, 0x36, 0xB2, 0x9F,  // ...du6..
                            /* 0120 */  0x55, 0xD3, 0x30, 0x72, 0x9D, 0x04, 0xB6, 0xD5,  // U.0r....
                            /* 0128 */  0xA4, 0x22, 0x3E, 0x9A, 0x09, 0x02, 0xBA, 0x77,  // .">....w
                            /* 0130 */  0x80, 0xB0, 0xA5, 0x1A, 0x2E, 0xF4, 0xBD, 0xBB,  // ........
                            /* 0138 */  0xE4, 0x28, 0x89, 0x10, 0x87, 0x62, 0x68, 0x09,  // .(...bh.
                            /* 0140 */  0xD6, 0x55, 0x09, 0x7E, 0x8E, 0x0D, 0xB2, 0x8D,  // .U.~....
                            /* 0148 */  0xBC, 0x98, 0xEB, 0x21, 0x91, 0x44, 0x96, 0x02,  // ...!.D..
                            /* 0150 */  0x3B, 0x03, 0xAA, 0xC4, 0xD7, 0xF4, 0x4C, 0x97,  // ;.....L.
                            /* 0158 */  0xC8, 0x0D, 0x3E, 0xF1, 0xA8, 0xE3, 0x80, 0x04,  // ..>.....
                            /* 0160 */  0xE0, 0xD3, 0xB6, 0x70, 0xF7, 0x18, 0xF2, 0xC5,  // ...p....
                            /* 0168 */  0xA3, 0x02, 0x31, 0x10, 0x40, 0x7F, 0x0D, 0x8D,  // ..1.@...
                            /* 0170 */  0xFC, 0x72, 0x44, 0x61, 0xD5, 0x44, 0xD3, 0x59,  // .rDa.D.Y
                            /* 0178 */  0x88, 0x9C, 0x26, 0x73, 0xBA, 0xD6, 0x4D, 0xC8,  // ..&s..M.
                            /* 0180 */  0x5F, 0xFC, 0x99, 0x9A, 0xC2, 0x9D, 0x02, 0x95,  // _.......
                            /* 0188 */  0xAB, 0x06, 0x37, 0x44, 0x48, 0x2D, 0x89, 0x53,  // ..7DH-.S
                            /* 0190 */  0xC4, 0xF9, 0x4F, 0xE2, 0x26, 0xF6, 0xC7, 0x7B,  // ..O.&..{
                            /* 0198 */  0xFF, 0x55, 0xE6, 0x14, 0xC4, 0xB1, 0x44, 0x03,  // .U....D.
                            /* 01A0 */  0xEF, 0x2D, 0x59, 0x22, 0x67, 0x69, 0xCF, 0x27,  // .-Y"gi.'
                            /* 01A8 */  0x9E, 0xD8, 0xE8, 0xC7, 0xBD, 0xA0, 0x67, 0x95,  // ......g.
                            /* 01B0 */  0x68, 0xB2, 0xDD, 0x4F, 0x7D, 0x1A, 0x77, 0xC4,  // h..O}.w.
                            /* 01B8 */  0x06, 0xD7, 0x8F, 0x3B, 0xE9, 0xE4, 0x8C, 0x0D,  // ...;....
                            /* 01C0 */  0x38, 0x45, 0x91, 0xBA, 0x19, 0xA0, 0x23, 0xD2,  // 8E....#.
                            /* 01C8 */  0x55, 0xBC, 0x7B, 0x2A, 0xA4, 0xBE, 0x95, 0xD4,  // U.{*....
                            /* 01D0 */  0xC3, 0xBF, 0x34, 0xE7, 0x3D, 0xB8, 0x4F, 0x53,  // ..4.=.OS
                            /* 01D8 */  0xFE, 0x37, 0x55, 0x63, 0x00, 0xD0, 0xC3, 0xAB,  // .7Uc....
                            /* 01E0 */  0xDA, 0x38, 0x7E, 0x41, 0x7B, 0x5A, 0x55, 0x7A,  // .8~A{ZUz
                            /* 01E8 */  0x4A, 0x7D, 0x1D, 0x1C, 0x98, 0x6B, 0xE8, 0xC4,  // J}...k..
                            /* 01F0 */  0x71, 0x13, 0x9A, 0x82, 0xC8, 0xCD, 0xAD, 0xDF,  // q.......
                            /* 01F8 */  0x5F, 0xE7, 0x2B, 0xC4, 0x5A, 0x3E, 0x66, 0xA4,  // _.+.Z>f.
                            /* 0200 */  0x18, 0x12, 0xFA, 0xDE, 0xCC, 0xD9, 0xF7, 0xBA,  // ........
                            /* 0208 */  0x8E, 0x5C, 0x5E, 0x7C, 0x12, 0x68, 0x6B, 0xDB,  // .\^|.hk.
                            /* 0210 */  0xA0, 0xA0, 0x50, 0x4D, 0xD2, 0x09, 0xEC, 0xEF,  // ..PM....
                            /* 0218 */  0x1B, 0xE5, 0xB3, 0xA0, 0xFF, 0xED, 0x52, 0xDC,  // ......R.
                            /* 0220 */  0xD6, 0x59, 0xCF, 0x80, 0xC4, 0xF3, 0x7A, 0x2B,  // .Y....z+
                            /* 0228 */  0x64, 0x9A, 0x18, 0x87, 0xDC, 0x94, 0xAB, 0x59,  // d......Y
                            /* 0230 */  0x51, 0xFC, 0xA0, 0x39, 0x3E, 0x16, 0x95, 0x66,  // Q..9>..f
                            /* 0238 */  0x99, 0x63, 0x7B, 0xF8, 0x64, 0x30, 0x01, 0x01,  // .c{.d0..
                            /* 0240 */  0x6C, 0x89, 0x1B, 0x72, 0x7B, 0xA4, 0x10, 0x49,  // l..r{..I
                            /* 0248 */  0x6A, 0xAD, 0x4A, 0x75, 0x6A, 0x4E, 0xFF, 0x78,  // j.JujN.x
                            /* 0250 */  0xC8, 0xC7, 0x08, 0x36, 0x49, 0xE4, 0x58, 0x94,  // ...6I.X.
                            /* 0258 */  0x32, 0xCA, 0x52, 0x58, 0x21, 0xAA, 0x95, 0xE9,  // 2.RX!...
                            /* 0260 */  0xB7, 0x96, 0xEE, 0xAD, 0x76, 0xE0, 0x54, 0xDB,  // ....v.T.
                            /* 0268 */  0xC3, 0xF1, 0x34, 0xB7, 0x46, 0xC9, 0x1F, 0xBA,  // ..4.F...
                            /* 0270 */  0x4A, 0xB7, 0x28, 0x59, 0xA8, 0x9F, 0x88, 0x25,  // J.(Y...%
                            /* 0278 */  0x4E, 0xF5, 0x40, 0x0E, 0xE0, 0xFB, 0x10, 0x18,  // N.@.....
                            /* 0280 */  0x2A, 0xCE, 0x68, 0x1C, 0xA7, 0x19, 0x11, 0xEC,  // *.h.....
                            /* 0288 */  0xC2, 0x40, 0xE6, 0x87, 0xBF, 0x65, 0x22, 0x17,  // .@...e".
                            /* 0290 */  0xB1, 0x4C, 0x16, 0xBF, 0x7A, 0x29, 0x4F, 0x2B,  // .L..z)O+
                            /* 0298 */  0x2D, 0xA8, 0x50, 0xCE, 0x99, 0xD2, 0xF9, 0xE2,  // -.P.....
                            /* 02A0 */  0xAE, 0xA0, 0x5C, 0xBA, 0xEC, 0xEF, 0xED, 0xCF,  // ..\.....
                            /* 02A8 */  0x44, 0xCA, 0x44, 0xB8, 0x9F, 0x58, 0xF2, 0x24,  // D.D..X.$
                            /* 02B0 */  0x7D, 0xC4, 0x3B, 0xE3, 0x84, 0x42, 0x7C, 0xCB,  // }.;..B|.
                            /* 02B8 */  0xD6, 0xBE, 0xDC, 0xB1, 0xB4, 0x52, 0xF5, 0xD6,  // .....R..
                            /* 02C0 */  0xBE, 0x49, 0x37, 0x07, 0x08, 0xC1, 0x91, 0xCE,  // .I7.....
                            /* 02C8 */  0x44, 0x38, 0x28, 0x7A, 0x97, 0xF4, 0x3F, 0x21,  // D8(z..?!
                            /* 02D0 */  0xF1, 0xC2, 0xB4, 0x61, 0x83, 0x7C, 0xFE, 0xF5,  // ...a.|..
                            /* 02D8 */  0x7B, 0xFF, 0xE9, 0x49, 0x70, 0xBE, 0x02, 0x31,  // {..Ip..1
                            /* 02E0 */  0xB9, 0xBE, 0x8D, 0x75, 0xB4, 0xA5, 0x3C, 0x07,  // ...u..<.
                            /* 02E8 */  0x5F, 0x72, 0xA5, 0x51, 0x16, 0x42, 0x83, 0xA1,  // _r.Q.B..
                            /* 02F0 */  0xA4, 0xFC, 0xB2, 0x63, 0x74, 0xAD, 0x61, 0xA0,  // ...ct.a.
                            /* 02F8 */  0x65, 0x4F, 0x01, 0xC2, 0xBC, 0xFB, 0x66, 0xDF,  // eO....f.
                            /* 0300 */  0x8F, 0xEF, 0x7A, 0xD1, 0x91, 0xA0, 0x72, 0xDC,  // ..z...r.
                            /* 0308 */  0x97, 0x81, 0xA3, 0x9C, 0xBF, 0xE4, 0x7A, 0x16,  // ......z.
                            /* 0310 */  0xE1, 0x08, 0xDD, 0x67, 0x2A, 0x32, 0x6C, 0x0F,  // ...g*2l.
                            /* 0318 */  0x1F, 0xD8, 0xA0, 0x55, 0x6E, 0x77, 0x18, 0xF8,  // ...Unw..
                            /* 0320 */  0xEC, 0x48, 0xC0, 0x9A, 0xB0, 0x19, 0x3E, 0xE2,  // .H....>.
                            /* 0328 */  0xC6, 0xF9, 0x47, 0x38, 0x89, 0x50, 0xDD, 0x99,  // ..G8.P..
                            /* 0330 */  0xFE, 0x56, 0xA1, 0x2C, 0xE9, 0x80, 0x85, 0x86,  // .V.,....
                            /* 0338 */  0x7F, 0x08, 0x79, 0x68, 0x81, 0x74, 0xED, 0x07,  // ..yh.t..
                            /* 0340 */  0x61, 0x22, 0x61, 0xD5, 0x56, 0x47, 0xA2, 0xBD,  // a"a.VG..
                            /* 0348 */  0x58, 0x8A, 0x69, 0x7B, 0xC2, 0x8E, 0xF2, 0x5F,  // X.i{..._
                            /* 0350 */  0xE2, 0xE5, 0x5D, 0x3C, 0x74, 0xE1, 0x2E, 0x33,  // ..]<t..3
                            /* 0358 */  0xD4, 0x5C, 0xB6, 0xB7, 0x3D, 0xCB, 0x4B, 0x63,  // .\..=.Kc
                            /* 0360 */  0x5D, 0x59, 0x9C, 0x64, 0x9A, 0x90, 0x03, 0xC7,  // ]Y.d....
                            /* 0368 */  0x74, 0xCA, 0xE0, 0xFF, 0x35, 0x1E, 0x0A, 0xF0,  // t...5...
                            /* 0370 */  0x7C, 0xB5, 0x41, 0x30, 0x2D, 0x47, 0x3F, 0x6D,  // |.A0-G?m
                            /* 0378 */  0x26                                             // &
                        }
                    })
                }
                Case (0x75)
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x0300)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0xBF, 0x91, 0x27, 0x06,  // ......'.
                            /* 0070 */  0xEA, 0x3A, 0xCC, 0x52, 0x85, 0xA2, 0x50, 0x87,  // .:.R..P.
                            /* 0078 */  0x55, 0xA7, 0xDE, 0x1D, 0x52, 0xF0, 0x27, 0x39,  // U...R.'9
                            /* 0080 */  0x4F, 0x34, 0xA2, 0xE4, 0x87, 0xEA, 0xE1, 0x50,  // O4.....P
                            /* 0088 */  0x1A, 0xB7, 0x4A, 0xC2, 0x6C, 0x02, 0x00, 0x00,  // ..J.l...
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x40, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00,  // .@......
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x43, 0xED, 0x82,  // u8...C..
                            /* 00D0 */  0xF5, 0xD8, 0x08, 0x03, 0x53, 0x11, 0x4F, 0x34,  // ....S.O4
                            /* 00D8 */  0x76, 0xA3, 0x42, 0x65, 0x95, 0x70, 0xA6, 0x69,  // v.Be.p.i
                            /* 00E0 */  0x09, 0xA5, 0x2D, 0x26, 0xAC, 0x1B, 0x79, 0x3B,  // ..-&..y;
                            /* 00E8 */  0x2D, 0x44, 0x3F, 0xE1, 0x47, 0xF6, 0xF2, 0x2F,  // -D?.G../
                            /* 00F0 */  0x3D, 0x53, 0x79, 0xC1, 0x4A, 0x91, 0x5E, 0xBA,  // =Sy.J.^.
                            /* 00F8 */  0x11, 0xFD, 0x91, 0x72, 0x99, 0x68, 0x61, 0x1D,  // ...r.ha.
                            /* 0100 */  0xDD, 0xE9, 0xEA, 0xB1, 0xCA, 0x4F, 0xA5, 0xED,  // .....O..
                            /* 0108 */  0xFE, 0x19, 0xFA, 0xA7, 0x18, 0xBC, 0x6B, 0x71,  // ......kq
                            /* 0110 */  0xE6, 0x62, 0xCC, 0x4B, 0x81, 0xCF, 0xD1, 0x53,  // .b.K...S
                            /* 0118 */  0x7D, 0x73, 0x54, 0x37, 0x72, 0x11, 0xA1, 0x33,  // }sT7r..3
                            /* 0120 */  0x26, 0x6B, 0x30, 0xC9, 0xD4, 0x17, 0xC4, 0xFF,  // &k0.....
                            /* 0128 */  0x26, 0x86, 0x19, 0xE5, 0x0F, 0x33, 0x2E, 0xF2,  // &....3..
                            /* 0130 */  0x16, 0x5F, 0xD9, 0x53, 0xB4, 0x63, 0xBE, 0x72,  // ._.S.c.r
                            /* 0138 */  0xF7, 0x73, 0xE8, 0x21, 0x0E, 0xC2, 0x62, 0xD7,  // .s.!..b.
                            /* 0140 */  0xCB, 0xF4, 0xA9, 0x95, 0x5D, 0x01, 0x78, 0xB3,  // ....].x.
                            /* 0148 */  0x83, 0xDE, 0x2E, 0x7B, 0x85, 0xE0, 0xED, 0xDC,  // ...{....
                            /* 0150 */  0xE3, 0x3B, 0xD0, 0x95, 0x25, 0x5F, 0x47, 0x77,  // .;..%_Gw
                            /* 0158 */  0x60, 0x65, 0xE9, 0xDD, 0xEB, 0x4E, 0x02, 0x53,  // `e...N.S
                            /* 0160 */  0xB1, 0xA5, 0xAF, 0x7B, 0xBA, 0xF7, 0x6B, 0x39,  // ...{..k9
                            /* 0168 */  0x32, 0xCB, 0xD7, 0x03, 0x66, 0xC5, 0x79, 0xFC,  // 2...f.y.
                            /* 0170 */  0x3B, 0x88, 0x56, 0x5E, 0x59, 0xE1, 0x16, 0x8C,  // ;.V^Y...
                            /* 0178 */  0xBB, 0x2B, 0x3B, 0x83, 0xD3, 0xC8, 0x5D, 0x8F,  // .+;...].
                            /* 0180 */  0x7B, 0xFE, 0x69, 0x19, 0xC9, 0x3C, 0x9D, 0xA0,  // {.i..<..
                            /* 0188 */  0x8B, 0xA0, 0x43, 0x22, 0xC3, 0xBE, 0x4C, 0x14,  // ..C"..L.
                            /* 0190 */  0x88, 0x41, 0x44, 0x5E, 0x18, 0xFA, 0x17, 0xC1,  // .AD^....
                            /* 0198 */  0xDA, 0x17, 0xB3, 0x42, 0x81, 0x7E, 0xA1, 0x7E,  // ...B.~.~
                            /* 01A0 */  0xA4, 0x42, 0xB6, 0x56, 0x3B, 0xCF, 0xB2, 0x08,  // .B.V;...
                            /* 01A8 */  0xAA, 0xCB, 0x42, 0x25, 0x82, 0x45, 0xF4, 0xBF,  // ..B%.E..
                            /* 01B0 */  0xD5, 0xF7, 0xEE, 0x1E, 0xC4, 0x8B, 0xA7, 0x04,  // ........
                            /* 01B8 */  0xB2, 0x0C, 0xE9, 0x18, 0x1A, 0x91, 0xFB, 0xD8,  // ........
                            /* 01C0 */  0x46, 0xAB, 0x39, 0xDB, 0xDD, 0x36, 0x35, 0x13,  // F.9..65.
                            /* 01C8 */  0x68, 0x02, 0x7A, 0x5A, 0xCA, 0x45, 0xEC, 0xFF,  // h.zZ.E..
                            /* 01D0 */  0x19, 0x22, 0x22, 0x48, 0xF0, 0xF8, 0x99, 0xE3,  // .""H....
                            /* 01D8 */  0xDA, 0x30, 0x72, 0x9F, 0x02, 0x8A, 0x63, 0x54,  // .0r...cT
                            /* 01E0 */  0x13, 0x00, 0xAF, 0x22, 0x14, 0xD6, 0x39, 0x55,  // ..."..9U
                            /* 01E8 */  0xA2, 0x61, 0x24, 0x8E, 0x51, 0xBC, 0xA0, 0xF4,  // .a$.Q...
                            /* 01F0 */  0x7B, 0x9F, 0x17, 0xBE, 0xBF, 0x99, 0x5E, 0xB9,  // {.....^.
                            /* 01F8 */  0x58, 0x8A, 0x66, 0x16, 0xC4, 0x51, 0x8F, 0xE3,  // X.f..Q..
                            /* 0200 */  0xFB, 0x82, 0xA8, 0x70, 0x89, 0x18, 0xCB, 0x10,  // ...p....
                            /* 0208 */  0x52, 0xB4, 0x6C, 0xE7, 0xC3, 0x4E, 0x94, 0xF2,  // R.l..N..
                            /* 0210 */  0x86, 0x0C, 0xD9, 0xCD, 0x71, 0x7B, 0xEC, 0x50,  // ....q{.P
                            /* 0218 */  0x3C, 0x88, 0x93, 0xD7, 0xA1, 0x60, 0x5D, 0x18,  // <....`].
                            /* 0220 */  0xE1, 0x10, 0xF4, 0xE8, 0x77, 0x00, 0xEC, 0xB2,  // ....w...
                            /* 0228 */  0xD2, 0xC1, 0x5D, 0x6C, 0xCA, 0x14, 0xD3, 0xD4,  // ..]l....
                            /* 0230 */  0x1C, 0x45, 0xFF, 0xBB, 0x93, 0x85, 0xC8, 0x92,  // .E......
                            /* 0238 */  0x55, 0xFB, 0x48, 0x74, 0x0B, 0xD7, 0x32, 0x8C,  // U.Ht..2.
                            /* 0240 */  0xA8, 0x98, 0x88, 0xE3, 0x55, 0x25, 0x9D, 0xAF,  // ....U%..
                            /* 0248 */  0xAC, 0x72, 0xA6, 0x4F, 0x63, 0xA7, 0x33, 0xD9,  // .r.Oc.3.
                            /* 0250 */  0x67, 0xEE, 0x4C, 0x98, 0xB0, 0xEF, 0x26, 0x41,  // g.L...&A
                            /* 0258 */  0xC4, 0x00, 0x98, 0xAB, 0x91, 0xC8, 0x42, 0x11,  // ......B.
                            /* 0260 */  0x6D, 0x47, 0x5A, 0x77, 0xBB, 0x91, 0x4C, 0x71,  // mGZw..Lq
                            /* 0268 */  0x50, 0x80, 0x8F, 0x43, 0x57, 0x9F, 0xFD, 0xC6,  // P..CW...
                            /* 0270 */  0x3C, 0x3A, 0x85, 0x92, 0xF1, 0x36, 0xCE, 0x00,  // <:...6..
                            /* 0278 */  0xC1, 0x02, 0x07, 0x0A, 0xD6, 0x5E, 0x89, 0x0A,  // .....^..
                            /* 0280 */  0x5A, 0xA8, 0xC6, 0x98, 0x91, 0x27, 0xB2, 0x98,  // Z....'..
                            /* 0288 */  0x81, 0x73, 0x70, 0xB2, 0x28, 0x2A, 0xB8, 0xCF,  // .sp.(*..
                            /* 0290 */  0x5C, 0xC1, 0x0F, 0x7D, 0x35, 0x96, 0x82, 0xDC,  // \..}5...
                            /* 0298 */  0x6F, 0xA3, 0x18, 0x90, 0x14, 0xFE, 0xD4, 0xEB,  // o.......
                            /* 02A0 */  0xB0, 0xEB, 0x0A, 0x32, 0x18, 0x27, 0xD7, 0x68,  // ...2.'.h
                            /* 02A8 */  0x26, 0x84, 0xBE, 0x99, 0x8B, 0x96, 0x0C, 0x6F,  // &......o
                            /* 02B0 */  0x6C, 0x07, 0x1F, 0xE9, 0x2B, 0xC6, 0xC0, 0xB3,  // l...+...
                            /* 02B8 */  0xE6, 0x41, 0x17, 0xCF, 0x49, 0xBC, 0x55, 0xF5,  // .A..I.U.
                            /* 02C0 */  0x95, 0x3F, 0xFC, 0x68, 0xF0, 0xCB, 0xB2, 0x08,  // .?.h....
                            /* 02C8 */  0xA5, 0x16, 0x3C, 0xAB, 0x34, 0xF0, 0xAD, 0x52,  // ..<.4..R
                            /* 02D0 */  0x63, 0xD9, 0x52, 0x4E, 0x8F, 0x2F, 0x3A, 0x24,  // c.RN./:$
                            /* 02D8 */  0x4F, 0x5C, 0xB5, 0x68, 0x5D, 0x29, 0xE5, 0xE3,  // O\.h])..
                            /* 02E0 */  0x0F, 0xF0, 0xCB, 0x2A, 0xEF, 0x76, 0xEA, 0x0A,  // ...*.v..
                            /* 02E8 */  0xE4, 0x41, 0xFF, 0xA3, 0x22, 0xEE, 0xC4, 0x58,  // .A.."..X
                            /* 02F0 */  0x89, 0x77, 0x85, 0x84, 0xDE, 0xBC, 0xB3, 0x3C,  // .w.....<
                            /* 02F8 */  0x94, 0xAA, 0xBA, 0x01, 0xFC, 0xDB, 0x37, 0x00   // ......7.
                        }
                    })
                }
                Case (0x76)
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x0379)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0x36, 0x8E, 0xB1, 0xB5,  // ....6...
                            /* 0070 */  0xFE, 0x5A, 0x71, 0x99, 0xA1, 0xB3, 0x8C, 0xF1,  // .Zq.....
                            /* 0078 */  0x43, 0x9D, 0x76, 0xE1, 0x4C, 0x84, 0xF4, 0xEE,  // C.v.L...
                            /* 0080 */  0x4F, 0x79, 0xDD, 0x18, 0xFF, 0x83, 0x10, 0xE7,  // Oy......
                            /* 0088 */  0x54, 0x99, 0xAF, 0xA4, 0xE5, 0x02, 0x00, 0x00,  // T.......
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x84, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x40, 0xDF, 0x5C,  // u8...@.\
                            /* 00D0 */  0x4F, 0x89, 0xF0, 0xDC, 0xC2, 0x03, 0xE4, 0x33,  // O......3
                            /* 00D8 */  0x41, 0xDE, 0x14, 0x4A, 0x70, 0x1C, 0xD7, 0xB5,  // A..Jp...
                            /* 00E0 */  0xA2, 0xD1, 0xFE, 0x4C, 0xAC, 0x05, 0xED, 0xE7,  // ...L....
                            /* 00E8 */  0x60, 0x6C, 0xF9, 0xAB, 0x9C, 0x07, 0x0E, 0x87,  // `l......
                            /* 00F0 */  0xD1, 0x4E, 0xEC, 0x71, 0x2C, 0xD7, 0x05, 0xA9,  // .N.q,...
                            /* 00F8 */  0xA9, 0x74, 0xB9, 0x89, 0x8E, 0x6A, 0x83, 0x82,  // .t...j..
                            /* 0100 */  0x64, 0xAE, 0xD0, 0x63, 0xD9, 0x0C, 0x26, 0x73,  // d..c..&s
                            /* 0108 */  0x71, 0xEF, 0x94, 0xFE, 0x88, 0x48, 0x55, 0xE6,  // q....HU.
                            /* 0110 */  0x1B, 0x1F, 0x87, 0x22, 0x24, 0x38, 0xA6, 0xF1,  // ..."$8..
                            /* 0118 */  0xE7, 0xC8, 0xF5, 0x64, 0x75, 0x36, 0xB2, 0x9F,  // ...du6..
                            /* 0120 */  0x55, 0xD3, 0x30, 0x72, 0x9D, 0x04, 0xB6, 0xD5,  // U.0r....
                            /* 0128 */  0xA4, 0x22, 0x3E, 0x9A, 0x09, 0x02, 0xBA, 0x77,  // .">....w
                            /* 0130 */  0x80, 0xB0, 0xA5, 0x1A, 0x2E, 0xF4, 0xBD, 0xBB,  // ........
                            /* 0138 */  0xE4, 0x28, 0x89, 0x10, 0x87, 0x62, 0x68, 0x09,  // .(...bh.
                            /* 0140 */  0xD6, 0x55, 0x09, 0x7E, 0x8E, 0x0D, 0xB2, 0x8D,  // .U.~....
                            /* 0148 */  0xBC, 0x98, 0xEB, 0x21, 0x91, 0x44, 0x96, 0x02,  // ...!.D..
                            /* 0150 */  0x3B, 0x03, 0xAA, 0xC4, 0xD7, 0xF4, 0x4C, 0x97,  // ;.....L.
                            /* 0158 */  0xC8, 0x0D, 0x3E, 0xF1, 0xA8, 0xE3, 0x80, 0x04,  // ..>.....
                            /* 0160 */  0xE0, 0xD3, 0xB6, 0x70, 0xF7, 0x18, 0xF2, 0xC5,  // ...p....
                            /* 0168 */  0xA3, 0x02, 0x31, 0x10, 0x40, 0x7F, 0x0D, 0x8D,  // ..1.@...
                            /* 0170 */  0xFC, 0x72, 0x44, 0x61, 0xD5, 0x44, 0xD3, 0x59,  // .rDa.D.Y
                            /* 0178 */  0x88, 0x9C, 0x26, 0x73, 0xBA, 0xD6, 0x4D, 0xC8,  // ..&s..M.
                            /* 0180 */  0x5F, 0xFC, 0x99, 0x9A, 0xC2, 0x9D, 0x02, 0x95,  // _.......
                            /* 0188 */  0xAB, 0x06, 0x37, 0x44, 0x48, 0x2D, 0x89, 0x53,  // ..7DH-.S
                            /* 0190 */  0xC4, 0xF9, 0x4F, 0xE2, 0x26, 0xF6, 0xC7, 0x7B,  // ..O.&..{
                            /* 0198 */  0xFF, 0x55, 0xE6, 0x14, 0xC4, 0xB1, 0x44, 0x03,  // .U....D.
                            /* 01A0 */  0xEF, 0x2D, 0x59, 0x22, 0x67, 0x69, 0xCF, 0x27,  // .-Y"gi.'
                            /* 01A8 */  0x9E, 0xD8, 0xE8, 0xC7, 0xBD, 0xA0, 0x67, 0x95,  // ......g.
                            /* 01B0 */  0x68, 0xB2, 0xDD, 0x4F, 0x7D, 0x1A, 0x77, 0xC4,  // h..O}.w.
                            /* 01B8 */  0x06, 0xD7, 0x8F, 0x3B, 0xE9, 0xE4, 0x8C, 0x0D,  // ...;....
                            /* 01C0 */  0x38, 0x45, 0x91, 0xBA, 0x19, 0xA0, 0x23, 0xD2,  // 8E....#.
                            /* 01C8 */  0x55, 0xBC, 0x7B, 0x2A, 0xA4, 0xBE, 0x95, 0xD4,  // U.{*....
                            /* 01D0 */  0xC3, 0xBF, 0x34, 0xE7, 0x3D, 0xB8, 0x4F, 0x53,  // ..4.=.OS
                            /* 01D8 */  0xFE, 0x37, 0x55, 0x63, 0x00, 0xD0, 0xC3, 0xAB,  // .7Uc....
                            /* 01E0 */  0xDA, 0x38, 0x7E, 0x41, 0x7B, 0x5A, 0x55, 0x7A,  // .8~A{ZUz
                            /* 01E8 */  0x4A, 0x7D, 0x1D, 0x1C, 0x98, 0x6B, 0xE8, 0xC4,  // J}...k..
                            /* 01F0 */  0x71, 0x13, 0x9A, 0x82, 0xC8, 0xCD, 0xAD, 0xDF,  // q.......
                            /* 01F8 */  0x5F, 0xE7, 0x2B, 0xC4, 0x5A, 0x3E, 0x66, 0xA4,  // _.+.Z>f.
                            /* 0200 */  0x18, 0x12, 0xFA, 0xDE, 0xCC, 0xD9, 0xF7, 0xBA,  // ........
                            /* 0208 */  0x8E, 0x5C, 0x5E, 0x7C, 0x12, 0x68, 0x6B, 0xDB,  // .\^|.hk.
                            /* 0210 */  0xA0, 0xA0, 0x50, 0x4D, 0xD2, 0x09, 0xEC, 0xEF,  // ..PM....
                            /* 0218 */  0x1B, 0xE5, 0xB3, 0xA0, 0xFF, 0xED, 0x52, 0xDC,  // ......R.
                            /* 0220 */  0xD6, 0x59, 0xCF, 0x80, 0xC4, 0xF3, 0x7A, 0x2B,  // .Y....z+
                            /* 0228 */  0x64, 0x9A, 0x18, 0x87, 0xDC, 0x94, 0xAB, 0x59,  // d......Y
                            /* 0230 */  0x51, 0xFC, 0xA0, 0x39, 0x3E, 0x16, 0x95, 0x66,  // Q..9>..f
                            /* 0238 */  0x99, 0x63, 0x7B, 0xF8, 0x64, 0x30, 0x01, 0x01,  // .c{.d0..
                            /* 0240 */  0x6C, 0x89, 0x1B, 0x72, 0x7B, 0xA4, 0x10, 0x49,  // l..r{..I
                            /* 0248 */  0x6A, 0xAD, 0x4A, 0x75, 0x6A, 0x4E, 0xFF, 0x78,  // j.JujN.x
                            /* 0250 */  0xC8, 0xC7, 0x08, 0x36, 0x49, 0xE4, 0x58, 0x94,  // ...6I.X.
                            /* 0258 */  0x32, 0xCA, 0x52, 0x58, 0x21, 0xAA, 0x95, 0xE9,  // 2.RX!...
                            /* 0260 */  0xB7, 0x96, 0xEE, 0xAD, 0x76, 0xE0, 0x54, 0xDB,  // ....v.T.
                            /* 0268 */  0xC3, 0xF1, 0x34, 0xB7, 0x46, 0xC9, 0x1F, 0xBA,  // ..4.F...
                            /* 0270 */  0x4A, 0xB7, 0x28, 0x59, 0xA8, 0x9F, 0x88, 0x25,  // J.(Y...%
                            /* 0278 */  0x4E, 0xF5, 0x40, 0x0E, 0xE0, 0xFB, 0x10, 0x18,  // N.@.....
                            /* 0280 */  0x2A, 0xCE, 0x68, 0x1C, 0xA7, 0x19, 0x11, 0xEC,  // *.h.....
                            /* 0288 */  0xC2, 0x40, 0xE6, 0x87, 0xBF, 0x65, 0x22, 0x17,  // .@...e".
                            /* 0290 */  0xB1, 0x4C, 0x16, 0xBF, 0x7A, 0x29, 0x4F, 0x2B,  // .L..z)O+
                            /* 0298 */  0x2D, 0xA8, 0x50, 0xCE, 0x99, 0xD2, 0xF9, 0xE2,  // -.P.....
                            /* 02A0 */  0xAE, 0xA0, 0x5C, 0xBA, 0xEC, 0xEF, 0xED, 0xCF,  // ..\.....
                            /* 02A8 */  0x44, 0xCA, 0x44, 0xB8, 0x9F, 0x58, 0xF2, 0x24,  // D.D..X.$
                            /* 02B0 */  0x7D, 0xC4, 0x3B, 0xE3, 0x84, 0x42, 0x7C, 0xCB,  // }.;..B|.
                            /* 02B8 */  0xD6, 0xBE, 0xDC, 0xB1, 0xB4, 0x52, 0xF5, 0xD6,  // .....R..
                            /* 02C0 */  0xBE, 0x49, 0x37, 0x07, 0x08, 0xC1, 0x91, 0xCE,  // .I7.....
                            /* 02C8 */  0x44, 0x38, 0x28, 0x7A, 0x97, 0xF4, 0x3F, 0x21,  // D8(z..?!
                            /* 02D0 */  0xF1, 0xC2, 0xB4, 0x61, 0x83, 0x7C, 0xFE, 0xF5,  // ...a.|..
                            /* 02D8 */  0x7B, 0xFF, 0xE9, 0x49, 0x70, 0xBE, 0x02, 0x31,  // {..Ip..1
                            /* 02E0 */  0xB9, 0xBE, 0x8D, 0x75, 0xB4, 0xA5, 0x3C, 0x07,  // ...u..<.
                            /* 02E8 */  0x5F, 0x72, 0xA5, 0x51, 0x16, 0x42, 0x83, 0xA1,  // _r.Q.B..
                            /* 02F0 */  0xA4, 0xFC, 0xB2, 0x63, 0x74, 0xAD, 0x61, 0xA0,  // ...ct.a.
                            /* 02F8 */  0x65, 0x4F, 0x01, 0xC2, 0xBC, 0xFB, 0x66, 0xDF,  // eO....f.
                            /* 0300 */  0x8F, 0xEF, 0x7A, 0xD1, 0x91, 0xA0, 0x72, 0xDC,  // ..z...r.
                            /* 0308 */  0x97, 0x81, 0xA3, 0x9C, 0xBF, 0xE4, 0x7A, 0x16,  // ......z.
                            /* 0310 */  0xE1, 0x08, 0xDD, 0x67, 0x2A, 0x32, 0x6C, 0x0F,  // ...g*2l.
                            /* 0318 */  0x1F, 0xD8, 0xA0, 0x55, 0x6E, 0x77, 0x18, 0xF8,  // ...Unw..
                            /* 0320 */  0xEC, 0x48, 0xC0, 0x9A, 0xB0, 0x19, 0x3E, 0xE2,  // .H....>.
                            /* 0328 */  0xC6, 0xF9, 0x47, 0x38, 0x89, 0x50, 0xDD, 0x99,  // ..G8.P..
                            /* 0330 */  0xFE, 0x56, 0xA1, 0x2C, 0xE9, 0x80, 0x85, 0x86,  // .V.,....
                            /* 0338 */  0x7F, 0x08, 0x79, 0x68, 0x81, 0x74, 0xED, 0x07,  // ..yh.t..
                            /* 0340 */  0x61, 0x22, 0x61, 0xD5, 0x56, 0x47, 0xA2, 0xBD,  // a"a.VG..
                            /* 0348 */  0x58, 0x8A, 0x69, 0x7B, 0xC2, 0x8E, 0xF2, 0x5F,  // X.i{..._
                            /* 0350 */  0xE2, 0xE5, 0x5D, 0x3C, 0x74, 0xE1, 0x2E, 0x33,  // ..]<t..3
                            /* 0358 */  0xD4, 0x5C, 0xB6, 0xB7, 0x3D, 0xCB, 0x4B, 0x63,  // .\..=.Kc
                            /* 0360 */  0x5D, 0x59, 0x9C, 0x64, 0x9A, 0x90, 0x03, 0xC7,  // ]Y.d....
                            /* 0368 */  0x74, 0xCA, 0xE0, 0xFF, 0x35, 0x1E, 0x0A, 0xF0,  // t...5...
                            /* 0370 */  0x7C, 0xB5, 0x41, 0x30, 0x2D, 0x47, 0x3F, 0x6D,  // |.A0-G?m
                            /* 0378 */  0x26                                             // &
                        }
                    })
                }
                Case (0x77)
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x03EE)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0xA2, 0x1F, 0x83, 0x79,  // .......y
                            /* 0070 */  0xCE, 0x6C, 0xC7, 0xD6, 0x3D, 0x4F, 0x42, 0xAD,  // .l..=OB.
                            /* 0078 */  0x9C, 0xE6, 0x09, 0x7B, 0x57, 0xBC, 0xDF, 0x77,  // ...{W..w
                            /* 0080 */  0x67, 0xBF, 0x99, 0xBC, 0x96, 0xB4, 0xA7, 0x32,  // g......2
                            /* 0088 */  0x57, 0xD2, 0xA0, 0xD6, 0x5A, 0x03, 0x00, 0x00,  // W...Z...
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x2D, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,  // .-$.....
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x42, 0xA3, 0xD6,  // u8...B..
                            /* 00D0 */  0x71, 0x3F, 0x1A, 0x9F, 0xEC, 0x50, 0x2D, 0x60,  // q?...P-`
                            /* 00D8 */  0x79, 0xD0, 0x42, 0x7D, 0xDB, 0x9E, 0xBE, 0x4C,  // y.B}...L
                            /* 00E0 */  0x9E, 0x6E, 0xEC, 0x11, 0x79, 0xF7, 0x31, 0x45,  // .n..y.1E
                            /* 00E8 */  0xB1, 0x12, 0xDA, 0x42, 0xE5, 0x4B, 0x80, 0xC2,  // ...B.K..
                            /* 00F0 */  0xB8, 0xFB, 0x87, 0x4E, 0x6B, 0x99, 0xD1, 0x5E,  // ...Nk..^
                            /* 00F8 */  0xD3, 0x1D, 0x5C, 0x22, 0x05, 0x91, 0xB7, 0xD1,  // ..\"....
                            /* 0100 */  0x46, 0x3F, 0x3D, 0xD7, 0x27, 0x44, 0x7D, 0xC4,  // F?=.'D}.
                            /* 0108 */  0x06, 0x80, 0x0E, 0x1C, 0xE3, 0xEA, 0xC9, 0x7B,  // .......{
                            /* 0110 */  0x3A, 0xCB, 0x75, 0xB7, 0xD2, 0xAC, 0x19, 0x68,  // :.u....h
                            /* 0118 */  0x9C, 0xD1, 0x2B, 0xCE, 0xC2, 0x84, 0x79, 0xE5,  // ..+...y.
                            /* 0120 */  0xCA, 0x03, 0x53, 0xFA, 0x88, 0xC1, 0xF3, 0x8B,  // ..S.....
                            /* 0128 */  0x26, 0xE6, 0xA8, 0xCA, 0xD5, 0x02, 0x7D, 0x84,  // &.....}.
                            /* 0130 */  0x7C, 0xA3, 0x08, 0x6C, 0xB0, 0x21, 0x8F, 0x45,  // |..l.!.E
                            /* 0138 */  0x25, 0x2C, 0x35, 0x57, 0xB1, 0x75, 0x37, 0xEB,  // %,5W.u7.
                            /* 0140 */  0xB9, 0x10, 0x0D, 0xA2, 0xF0, 0xAF, 0x7C, 0x25,  // ......|%
                            /* 0148 */  0x9D, 0xFE, 0x4E, 0xB3, 0xD6, 0xD4, 0x4C, 0x18,  // ..N...L.
                            /* 0150 */  0x9F, 0xEE, 0xCD, 0x45, 0xC4, 0xA3, 0x9B, 0xA2,  // ...E....
                            /* 0158 */  0x77, 0x6B, 0xC5, 0xBE, 0xF3, 0x63, 0xF7, 0x22,  // wk...c."
                            /* 0160 */  0x2F, 0xE7, 0xB6, 0xF0, 0xBB, 0xAD, 0xA2, 0x3B,  // /......;
                            /* 0168 */  0x52, 0xB0, 0xBF, 0x74, 0x0A, 0x10, 0x73, 0x77,  // R..t..sw
                            /* 0170 */  0x04, 0x60, 0x92, 0x28, 0x41, 0xC4, 0xF2, 0xD9,  // .`.(A...
                            /* 0178 */  0x9E, 0x67, 0x0D, 0xFF, 0xA3, 0x2C, 0x55, 0x35,  // .g...,U5
                            /* 0180 */  0x17, 0xB7, 0x24, 0x06, 0xE5, 0x46, 0x1B, 0x1E,  // ..$..F..
                            /* 0188 */  0x33, 0x30, 0x41, 0x57, 0xAB, 0x9C, 0x23, 0x95,  // 30AW..#.
                            /* 0190 */  0x83, 0xFD, 0x59, 0xEC, 0x9D, 0x6A, 0x30, 0xA4,  // ..Y..j0.
                            /* 0198 */  0xBA, 0x4F, 0x2F, 0xF2, 0xC4, 0x32, 0x76, 0x51,  // .O/..2vQ
                            /* 01A0 */  0xC4, 0xDD, 0x10, 0x72, 0x62, 0x42, 0x14, 0xA8,  // ...rbB..
                            /* 01A8 */  0x45, 0xB3, 0x53, 0x7A, 0xC8, 0xFF, 0x37, 0xBC,  // E.Sz..7.
                            /* 01B0 */  0x40, 0x3E, 0xFF, 0x39, 0x9D, 0xC2, 0xA2, 0x94,  // @>.9....
                            /* 01B8 */  0xAB, 0xA5, 0xB2, 0xCD, 0xFF, 0x65, 0x5C, 0x71,  // .....e\q
                            /* 01C0 */  0x27, 0x15, 0x49, 0x48, 0x67, 0x67, 0x43, 0x6E,  // '.IHggCn
                            /* 01C8 */  0x69, 0x5D, 0x8C, 0xA6, 0xBE, 0xF3, 0x67, 0x6D,  // i]....gm
                            /* 01D0 */  0xF5, 0xC7, 0x51, 0x5A, 0xA2, 0x25, 0xE4, 0x74,  // ..QZ.%.t
                            /* 01D8 */  0xC7, 0x23, 0x7F, 0xED, 0xBB, 0xC0, 0x44, 0xB9,  // .#....D.
                            /* 01E0 */  0x40, 0xD8, 0x4D, 0x93, 0x84, 0xE7, 0xC2, 0x6A,  // @.M....j
                            /* 01E8 */  0x71, 0x55, 0xB2, 0x76, 0x9A, 0xB5, 0x4C, 0x3E,  // qU.v..L>
                            /* 01F0 */  0x1D, 0x6D, 0x6B, 0x41, 0xC3, 0x6B, 0x2F, 0x96,  // .mkA.k/.
                            /* 01F8 */  0x66, 0x97, 0x58, 0x4D, 0xFD, 0xA0, 0xFB, 0x1E,  // f.XM....
                            /* 0200 */  0xD5, 0xF1, 0x51, 0x31, 0x88, 0xBF, 0x26, 0x0D,  // ..Q1..&.
                            /* 0208 */  0xB5, 0x7F, 0xAC, 0x9A, 0x91, 0x0F, 0xEE, 0xD3,  // ........
                            /* 0210 */  0x34, 0xF6, 0xEA, 0x19, 0x9B, 0xB9, 0xAC, 0x13,  // 4.......
                            /* 0218 */  0x1C, 0x3B, 0xBC, 0x3C, 0x0F, 0xF2, 0x39, 0x08,  // .;.<..9.
                            /* 0220 */  0xE4, 0x3A, 0xE6, 0xE2, 0x92, 0x79, 0xA7, 0x91,  // .:...y..
                            /* 0228 */  0x30, 0x11, 0xC9, 0x6F, 0x13, 0x1E, 0x94, 0xDF,  // 0..o....
                            /* 0230 */  0xCC, 0x79, 0xE2, 0xB8, 0x9B, 0x60, 0xD0, 0x4F,  // .y...`.O
                            /* 0238 */  0xBC, 0x45, 0x9A, 0xC1, 0x2F, 0xAE, 0xDC, 0xDF,  // .E../...
                            /* 0240 */  0xD7, 0xA4, 0x0E, 0xA4, 0x21, 0x95, 0xEF, 0x85,  // ....!...
                            /* 0248 */  0x04, 0x45, 0x29, 0x35, 0x92, 0x7E, 0x4F, 0x2C,  // .E)5.~O,
                            /* 0250 */  0x7D, 0x12, 0x20, 0x4A, 0xD2, 0xBA, 0x96, 0x3C,  // }. J...<
                            /* 0258 */  0x2B, 0xCD, 0x50, 0x29, 0xF5, 0x73, 0x8F, 0x3D,  // +.P).s.=
                            /* 0260 */  0x8B, 0x9B, 0x65, 0xF0, 0x05, 0x3D, 0x7B, 0xEF,  // ..e..={.
                            /* 0268 */  0x61, 0x0B, 0x7D, 0xF9, 0x7A, 0x43, 0x2F, 0x94,  // a.}.zC/.
                            /* 0270 */  0xD7, 0x5F, 0x8B, 0xF4, 0x3F, 0x8E, 0xC5, 0x42,  // ._..?..B
                            /* 0278 */  0x26, 0xE5, 0x0E, 0xF6, 0x17, 0x71, 0x80, 0xB1,  // &....q..
                            /* 0280 */  0x01, 0xAB, 0x47, 0x65, 0x8D, 0x20, 0x70, 0xD3,  // ..Ge. p.
                            /* 0288 */  0xC1, 0xE1, 0x4C, 0x2A, 0x94, 0x51, 0x35, 0xEF,  // ..L*.Q5.
                            /* 0290 */  0x29, 0x7E, 0x55, 0x07, 0x99, 0x41, 0xD8, 0xF5,  // )~U..A..
                            /* 0298 */  0xC0, 0xB0, 0xC3, 0x93, 0x0B, 0x42, 0xE6, 0xE3,  // .....B..
                            /* 02A0 */  0x6F, 0x2D, 0xA8, 0x6A, 0x7D, 0x05, 0x75, 0x88,  // o-.j}.u.
                            /* 02A8 */  0x93, 0x32, 0xDE, 0x4B, 0x85, 0x5E, 0xCF, 0x48,  // .2.K.^.H
                            /* 02B0 */  0xB0, 0x88, 0x0A, 0xA0, 0x36, 0x7D, 0x8A, 0x8F,  // ....6}..
                            /* 02B8 */  0x23, 0x30, 0x14, 0x8A, 0xAB, 0x5E, 0xEE, 0xAE,  // #0...^..
                            /* 02C0 */  0x84, 0x6E, 0x84, 0x94, 0xA1, 0x81, 0xAC, 0xE1,  // .n......
                            /* 02C8 */  0x05, 0x5A, 0x17, 0x7B, 0xA7, 0x5C, 0xA9, 0x03,  // .Z.{.\..
                            /* 02D0 */  0xFD, 0xE9, 0x0F, 0x7E, 0xBF, 0x29, 0x4F, 0x1C,  // ...~.)O.
                            /* 02D8 */  0x5E, 0xF8, 0xC2, 0xA5, 0x8C, 0x7C, 0x35, 0x1D,  // ^....|5.
                            /* 02E0 */  0xD7, 0x9B, 0x37, 0xBA, 0x4B, 0x63, 0xD0, 0x88,  // ..7.Kc..
                            /* 02E8 */  0x86, 0x8E, 0xC4, 0x49, 0x1A, 0x39, 0xC9, 0xBC,  // ...I.9..
                            /* 02F0 */  0xC8, 0xEE, 0x39, 0x6F, 0x26, 0xC1, 0xDF, 0x05,  // ..9o&...
                            /* 02F8 */  0xDD, 0x7E, 0x94, 0xEC, 0x96, 0x79, 0x8F, 0x3B,  // .~...y.;
                            /* 0300 */  0x1C, 0x3C, 0x3B, 0x5B, 0x2A, 0xAC, 0x54, 0xD4,  // .<;[*.T.
                            /* 0308 */  0x35, 0x4E, 0x38, 0x48, 0x1D, 0xF9, 0x7C, 0x36,  // 5N8H..|6
                            /* 0310 */  0x4E, 0x38, 0x05, 0xDA, 0xD7, 0x0A, 0x6D, 0xD2,  // N8....m.
                            /* 0318 */  0xB0, 0x2D, 0x4A, 0xB8, 0x55, 0x7A, 0x02, 0x64,  // .-J.Uz.d
                            /* 0320 */  0x99, 0x1B, 0xEF, 0xB7, 0xA1, 0xCB, 0xBD, 0x2C,  // .......,
                            /* 0328 */  0xF3, 0xDC, 0xD0, 0x79, 0xD5, 0x0F, 0xCA, 0x39,  // ...y...9
                            /* 0330 */  0x51, 0xC7, 0xC2, 0xBD, 0xB4, 0x12, 0x2D, 0x7B,  // Q.....-{
                            /* 0338 */  0x7A, 0x3A, 0xDC, 0x66, 0xF0, 0x12, 0x8B, 0x19,  // z:.f....
                            /* 0340 */  0x38, 0xC8, 0x71, 0x17, 0x86, 0xBD, 0xE3, 0x8B,  // 8.q.....
                            /* 0348 */  0x6A, 0xA0, 0x3E, 0x9C, 0x5D, 0xC5, 0x10, 0xB2,  // j.>.]...
                            /* 0350 */  0xBB, 0x3A, 0xBD, 0x74, 0x00, 0xF0, 0x7C, 0x5C,  // .:.t..|\
                            /* 0358 */  0x95, 0x85, 0x1B, 0xFE, 0xFB, 0x09, 0x97, 0x7E,  // .......~
                            /* 0360 */  0x96, 0x14, 0x0C, 0xB1, 0x35, 0xD6, 0x96, 0x4F,  // ....5..O
                            /* 0368 */  0x84, 0xD5, 0x64, 0x5D, 0x6B, 0xC5, 0x7E, 0x9C,  // ..d]k.~.
                            /* 0370 */  0x16, 0x84, 0x2B, 0x3B, 0x18, 0x1F, 0xD6, 0xED,  // ..+;....
                            /* 0378 */  0x5C, 0x55, 0xCB, 0x3A, 0xD3, 0x6D, 0xB4, 0x94,  // \U.:.m..
                            /* 0380 */  0xDA, 0xC8, 0xA0, 0x9B, 0x3F, 0x80, 0x40, 0x56,  // ....?.@V
                            /* 0388 */  0x10, 0xBB, 0x5F, 0x1C, 0xD6, 0x15, 0x48, 0x6E,  // .._...Hn
                            /* 0390 */  0xB7, 0xDB, 0x61, 0xAD, 0x19, 0x8C, 0xCD, 0x30,  // ..a....0
                            /* 0398 */  0xC8, 0x8D, 0x26, 0xD2, 0xCB, 0x0C, 0xC2, 0x9B,  // ..&.....
                            /* 03A0 */  0xA5, 0xCF, 0x3C, 0xCE, 0x9A, 0xA0, 0xDE, 0x21,  // ..<....!
                            /* 03A8 */  0x2A, 0xC1, 0x2E, 0x60, 0x55, 0xE0, 0x93, 0x05,  // *..`U...
                            /* 03B0 */  0xC6, 0x2D, 0xCE, 0x45, 0x39, 0x72, 0xA1, 0x36,  // .-.E9r.6
                            /* 03B8 */  0x4A, 0xA4, 0x3F, 0x5E, 0xBD, 0xC0, 0x65, 0x15,  // J.?^..e.
                            /* 03C0 */  0x99, 0x51, 0xAD, 0xAE, 0xFD, 0x57, 0x5E, 0x3C,  // .Q...W^<
                            /* 03C8 */  0x6F, 0xB0, 0x9D, 0x04, 0xD5, 0x5A, 0x0C, 0xC7,  // o....Z..
                            /* 03D0 */  0x21, 0x43, 0xDB, 0xCE, 0x6D, 0xD4, 0x6A, 0x13,  // !C..m.j.
                            /* 03D8 */  0x6A, 0xEF, 0x8D, 0xE3, 0xCB, 0x58, 0x16, 0x4B,  // j....X.K
                            /* 03E0 */  0x7D, 0xFE, 0x2B, 0x8D, 0x29, 0xC1, 0xFB, 0xBB,  // }.+.)...
                            /* 03E8 */  0x7B, 0x9D, 0xB0, 0xE5, 0x00, 0x00               // {.....
                        }
                    })
                }
                Default
                {
                    Return (Package (0x01)
                    {
                        Buffer (0x0304)
                        {
                            /* 0000 */  0xE5, 0x1F, 0x94, 0x00, 0x00, 0x00, 0x00, 0x02,  // ........
                            /* 0008 */  0x00, 0x00, 0x00, 0x40, 0x67, 0x64, 0x64, 0x76,  // ...@gddv
                            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x4F, 0x45, 0x4D, 0x20,  // ....OEM 
                            /* 0030 */  0x45, 0x78, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x64,  // Exported
                            /* 0038 */  0x20, 0x44, 0x61, 0x74, 0x61, 0x56, 0x61, 0x75,  //  DataVau
                            /* 0040 */  0x6C, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // lt......
                            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0050 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0058 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ........
                            /* 0068 */  0x00, 0x00, 0x00, 0x00, 0x61, 0x08, 0x38, 0x46,  // ....a.8F
                            /* 0070 */  0x5F, 0x8A, 0xF0, 0x9A, 0xE0, 0xD6, 0x54, 0x03,  // _.....T.
                            /* 0078 */  0x29, 0xC5, 0xC4, 0xC7, 0x82, 0x92, 0xF2, 0xA6,  // ).......
                            /* 0080 */  0x61, 0x1C, 0x7D, 0xD5, 0x61, 0x41, 0xC8, 0x06,  // a.}.aA..
                            /* 0088 */  0xD1, 0x8F, 0x48, 0x75, 0x70, 0x02, 0x00, 0x00,  // ..Hup...
                            /* 0090 */  0x52, 0x45, 0x50, 0x4F, 0x5D, 0x00, 0x00, 0x00,  // REPO]...
                            /* 0098 */  0x01, 0x72, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,  // .r......
                            /* 00A0 */  0x00, 0x00, 0x72, 0x87, 0xCD, 0xFF, 0x6D, 0x24,  // ..r...m$
                            /* 00A8 */  0x47, 0xDB, 0x3D, 0x24, 0x92, 0xB4, 0x16, 0x6F,  // G.=$...o
                            /* 00B0 */  0x45, 0xD8, 0xC3, 0xF5, 0x66, 0x14, 0x9F, 0x22,  // E...f.."
                            /* 00B8 */  0xD7, 0xF7, 0xDE, 0x67, 0x90, 0x9A, 0xA2, 0x0D,  // ...g....
                            /* 00C0 */  0x39, 0x25, 0xAD, 0xC3, 0x1A, 0xAD, 0x52, 0x0B,  // 9%....R.
                            /* 00C8 */  0x75, 0x38, 0xE1, 0xA4, 0x14, 0x44, 0xBD, 0xF8,  // u8...D..
                            /* 00D0 */  0x61, 0x43, 0xC6, 0x9E, 0x95, 0x32, 0x0C, 0xFD,  // aC...2..
                            /* 00D8 */  0x0D, 0x22, 0xAB, 0x6F, 0x87, 0x29, 0x8C, 0x27,  // .".o.).'
                            /* 00E0 */  0x0F, 0xE8, 0x09, 0x94, 0xEC, 0xB0, 0x26, 0x11,  // ......&.
                            /* 00E8 */  0x00, 0x81, 0xF3, 0xEA, 0x1E, 0xE9, 0xA4, 0xCA,  // ........
                            /* 00F0 */  0x49, 0xAB, 0x93, 0xE1, 0x6D, 0xB3, 0x2A, 0x9C,  // I...m.*.
                            /* 00F8 */  0x53, 0x47, 0xB3, 0xC0, 0xB7, 0xE9, 0x52, 0xA9,  // SG....R.
                            /* 0100 */  0x28, 0x38, 0x31, 0x69, 0x68, 0x67, 0xD7, 0x27,  // (81ihg.'
                            /* 0108 */  0x51, 0x6E, 0xA9, 0x35, 0x48, 0x24, 0xAE, 0xC0,  // Qn.5H$..
                            /* 0110 */  0x1C, 0x03, 0x8F, 0x0C, 0x91, 0x13, 0x58, 0xC5,  // ......X.
                            /* 0118 */  0xBE, 0xB8, 0x3B, 0xF9, 0x15, 0xF1, 0xD3, 0x08,  // ..;.....
                            /* 0120 */  0x2C, 0xC4, 0xBB, 0xEA, 0xC2, 0x61, 0x48, 0xC0,  // ,....aH.
                            /* 0128 */  0x74, 0x42, 0x0F, 0x66, 0x49, 0xDB, 0x1A, 0x5C,  // tB.fI..\
                            /* 0130 */  0x33, 0x0D, 0x8F, 0x26, 0x05, 0xF7, 0xF3, 0xAA,  // 3..&....
                            /* 0138 */  0x26, 0xD9, 0x45, 0xC8, 0x3D, 0x70, 0x35, 0xB5,  // &.E.=p5.
                            /* 0140 */  0x06, 0xF7, 0xDD, 0x65, 0xFF, 0x71, 0xFE, 0x46,  // ...e.q.F
                            /* 0148 */  0x9E, 0xC9, 0xA5, 0xCE, 0x85, 0x59, 0x48, 0x74,  // .....YHt
                            /* 0150 */  0xEF, 0x5E, 0x93, 0xFA, 0x4E, 0x14, 0xC3, 0xAB,  // .^..N...
                            /* 0158 */  0xFE, 0x98, 0xEA, 0xEF, 0x77, 0x37, 0xAD, 0x69,  // ....w7.i
                            /* 0160 */  0x2E, 0xA3, 0x9A, 0x73, 0xCE, 0x10, 0x07, 0x4D,  // ...s...M
                            /* 0168 */  0x48, 0x70, 0x4F, 0xE1, 0xD1, 0xFD, 0xE6, 0xD8,  // HpO.....
                            /* 0170 */  0x31, 0xF0, 0xB8, 0x5D, 0xB1, 0xEF, 0xDA, 0x89,  // 1..]....
                            /* 0178 */  0xCE, 0x5C, 0x6B, 0x39, 0xF8, 0x55, 0x1D, 0xB2,  // .\k9.U..
                            /* 0180 */  0x52, 0x95, 0x64, 0x37, 0xFC, 0x52, 0xEA, 0xB3,  // R.d7.R..
                            /* 0188 */  0x6B, 0x78, 0x81, 0x8D, 0x06, 0x44, 0x6E, 0x54,  // kx...DnT
                            /* 0190 */  0x9C, 0xDD, 0xFF, 0xE3, 0x01, 0xA1, 0xB7, 0x6F,  // .......o
                            /* 0198 */  0x31, 0xBF, 0x6E, 0x86, 0xF1, 0x4B, 0xFA, 0xF4,  // 1.n..K..
                            /* 01A0 */  0x36, 0x96, 0xAB, 0x3F, 0x41, 0x42, 0xDD, 0x01,  // 6..?AB..
                            /* 01A8 */  0xC5, 0x8B, 0xF7, 0x24, 0x34, 0xB8, 0x6E, 0xF1,  // ...$4.n.
                            /* 01B0 */  0x2B, 0xB6, 0xDB, 0xE6, 0x7F, 0xD8, 0x18, 0x53,  // +......S
                            /* 01B8 */  0x9B, 0x20, 0x60, 0x49, 0x0F, 0xBA, 0x45, 0x10,  // . `I..E.
                            /* 01C0 */  0x84, 0x4F, 0xCA, 0xF4, 0x89, 0x0C, 0xFC, 0x42,  // .O.....B
                            /* 01C8 */  0xCB, 0x8C, 0x9F, 0x9A, 0x5F, 0xFC, 0xA3, 0x9B,  // ...._...
                            /* 01D0 */  0x82, 0x0F, 0xAF, 0xF6, 0x41, 0x0C, 0xF6, 0x02,  // ....A...
                            /* 01D8 */  0xC8, 0x3F, 0xC5, 0x3F, 0x90, 0x73, 0x7D, 0x5D,  // .?.?.s}]
                            /* 01E0 */  0x4C, 0xE4, 0x9E, 0xBC, 0xDC, 0xC9, 0x32, 0xFA,  // L.....2.
                            /* 01E8 */  0xC4, 0x33, 0x23, 0x27, 0x61, 0x67, 0x22, 0x50,  // .3#'ag"P
                            /* 01F0 */  0xED, 0x23, 0xC7, 0x25, 0xE2, 0x89, 0x31, 0x06,  // .#.%..1.
                            /* 01F8 */  0xF8, 0x89, 0x26, 0xB5, 0x32, 0x67, 0xD9, 0x1F,  // ..&.2g..
                            /* 0200 */  0x26, 0x29, 0x94, 0x75, 0x5E, 0x7C, 0x57, 0xB6,  // &).u^|W.
                            /* 0208 */  0x0F, 0xB6, 0x0E, 0x68, 0xE7, 0x63, 0x19, 0xB1,  // ...h.c..
                            /* 0210 */  0x01, 0xAF, 0x73, 0xEB, 0xF5, 0x29, 0xC3, 0x73,  // ..s..).s
                            /* 0218 */  0x9F, 0x64, 0x35, 0xE1, 0x2A, 0xA1, 0x32, 0x33,  // .d5.*.23
                            /* 0220 */  0x66, 0x3D, 0x9C, 0x69, 0xC0, 0x5F, 0x03, 0x69,  // f=.i._.i
                            /* 0228 */  0xD7, 0xD4, 0x4D, 0x64, 0x3F, 0xED, 0x95, 0xCF,  // ..Md?...
                            /* 0230 */  0x51, 0x34, 0xBF, 0x33, 0x48, 0xFD, 0x0B, 0xE6,  // Q4.3H...
                            /* 0238 */  0xC3, 0xD1, 0x16, 0xBB, 0x92, 0x3F, 0xAE, 0x1F,  // .....?..
                            /* 0240 */  0x37, 0x0D, 0x0E, 0x38, 0xFA, 0xE5, 0x13, 0x35,  // 7..8...5
                            /* 0248 */  0x7B, 0x5A, 0x1D, 0xFC, 0x33, 0x30, 0x4D, 0x69,  // {Z..30Mi
                            /* 0250 */  0x2F, 0xE3, 0xA8, 0xFA, 0x1C, 0xBE, 0x3C, 0x71,  // /.....<q
                            /* 0258 */  0x09, 0x09, 0x23, 0xDD, 0xBD, 0x9A, 0x14, 0x2B,  // ..#....+
                            /* 0260 */  0x22, 0x99, 0x8F, 0xF4, 0x4C, 0xC6, 0x71, 0x8A,  // "...L.q.
                            /* 0268 */  0xFF, 0x0D, 0x6F, 0xCA, 0x4E, 0xD3, 0x89, 0x3A,  // ..o.N..:
                            /* 0270 */  0xDC, 0xE2, 0x04, 0x4D, 0xD6, 0xF5, 0xA7, 0xE6,  // ...M....
                            /* 0278 */  0xE8, 0x66, 0x95, 0x0C, 0xBE, 0x5F, 0xB2, 0x2F,  // .f..._./
                            /* 0280 */  0xFE, 0xD4, 0xD4, 0x63, 0x3D, 0x58, 0xEB, 0x6E,  // ...c=X.n
                            /* 0288 */  0x69, 0x89, 0xFF, 0xC9, 0xFA, 0xAC, 0x67, 0x93,  // i.....g.
                            /* 0290 */  0xDA, 0x47, 0xDC, 0xEB, 0x52, 0x1D, 0xCA, 0xA7,  // .G..R...
                            /* 0298 */  0x4D, 0x4E, 0x73, 0x2D, 0xC7, 0xA0, 0x44, 0xEF,  // MNs-..D.
                            /* 02A0 */  0x79, 0x93, 0xF9, 0xF4, 0x9B, 0x1F, 0x7E, 0xA0,  // y.....~.
                            /* 02A8 */  0x48, 0xBF, 0x93, 0x25, 0x69, 0x48, 0x9D, 0xBD,  // H..%iH..
                            /* 02B0 */  0x64, 0x73, 0x38, 0xEA, 0x45, 0x7E, 0xAA, 0x1F,  // ds8.E~..
                            /* 02B8 */  0xC7, 0x78, 0x3D, 0xFA, 0xC1, 0x18, 0xAC, 0x9D,  // .x=.....
                            /* 02C0 */  0x46, 0x1C, 0xAA, 0xF8, 0x6C, 0x77, 0x72, 0x25,  // F...lwr%
                            /* 02C8 */  0xB5, 0xF7, 0xFF, 0xE4, 0xF4, 0x99, 0x2A, 0x4A,  // ......*J
                            /* 02D0 */  0x9B, 0x6C, 0x0D, 0xF5, 0x69, 0xB8, 0x18, 0x9C,  // .l..i...
                            /* 02D8 */  0x41, 0x51, 0x0D, 0xFC, 0x3C, 0x99, 0xCF, 0xB1,  // AQ..<...
                            /* 02E0 */  0x63, 0xD1, 0x76, 0x14, 0x4E, 0x1E, 0xEC, 0xB5,  // c.v.N...
                            /* 02E8 */  0xB2, 0x6B, 0x10, 0x72, 0x10, 0x79, 0x32, 0x9E,  // .k.r.y2.
                            /* 02F0 */  0xD3, 0xE5, 0x69, 0xE4, 0x73, 0x6B, 0xA1, 0xB0,  // ..i.sk..
                            /* 02F8 */  0x56, 0x92, 0x94, 0x1A, 0xA6, 0xCC, 0x98, 0x83,  // V.......
                            /* 0300 */  0x05, 0xD6, 0xE8, 0x37                           // ...7
                        }
                    })
                }

            }
        }

        Method (IMOK, 1, NotSerialized)
        {
            Return (Arg0)
        }
    }
}

