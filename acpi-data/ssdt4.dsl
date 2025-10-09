/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20241212 (64-bit version)
 * Copyright (c) 2000 - 2023 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of ssdt4.dat
 *
 * Original Table Header:
 *     Signature        "SSDT"
 *     Length           0x0000334B (13131)
 *     Revision         0x02
 *     Checksum         0x96
 *     OEM ID           "ALASKA"
 *     OEM Table ID     "Adl_DDR4"
 *     OEM Revision     0x00001000 (4096)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20200717 (538969879)
 */
DefinitionBlock ("", "SSDT", 2, "ALASKA", "Adl_DDR4", 0x00001000)
{
    External (_SB_.CAGS, MethodObj)    // 1 Arguments
    External (_SB_.CSD0, MethodObj)    // 1 Arguments
    External (_SB_.CSD3, MethodObj)    // 1 Arguments
    External (_SB_.ISME, MethodObj)    // 1 Arguments
    External (_SB_.OSCO, UnknownObj)
    External (_SB_.PC00, DeviceObj)
    External (_SB_.PC00.GLAN, DeviceObj)
    External (_SB_.PC00.HDAS, DeviceObj)
    External (_SB_.PC00.HDAS.VDID, UnknownObj)
    External (_SB_.PC00.I2C0, DeviceObj)
    External (_SB_.PC00.I2C0.TPD0, DeviceObj)
    External (_SB_.PC00.I2C1, DeviceObj)
    External (_SB_.PC00.I2C1.TPL1, DeviceObj)
    External (_SB_.PC00.PEG0, DeviceObj)
    External (_SB_.PC00.PEG0.PRMV, IntObj)
    External (_SB_.PC00.PEG0.PRTP, IntObj)
    External (_SB_.PC00.PEG0.RD3C, IntObj)
    External (_SB_.PC00.PEG0.SLOT, IntObj)
    External (_SB_.PC00.PEG0.VDID, UnknownObj)
    External (_SB_.PC00.PEG1, DeviceObj)
    External (_SB_.PC00.PEG1.PRMV, IntObj)
    External (_SB_.PC00.PEG1.PRTP, IntObj)
    External (_SB_.PC00.PEG1.RD3C, IntObj)
    External (_SB_.PC00.PEG1.SLOT, IntObj)
    External (_SB_.PC00.PEG1.VDID, UnknownObj)
    External (_SB_.PC00.PEG2, DeviceObj)
    External (_SB_.PC00.PEG2.PRMV, IntObj)
    External (_SB_.PC00.PEG2.PRTP, IntObj)
    External (_SB_.PC00.PEG2.RD3C, IntObj)
    External (_SB_.PC00.PEG2.SLOT, IntObj)
    External (_SB_.PC00.PEG2.VDID, UnknownObj)
    External (_SB_.PC00.RP01, DeviceObj)
    External (_SB_.PC00.RP01.PRMV, IntObj)
    External (_SB_.PC00.RP01.PRTP, IntObj)
    External (_SB_.PC00.RP01.RD3C, IntObj)
    External (_SB_.PC00.RP01.SLOT, IntObj)
    External (_SB_.PC00.RP01.VDID, UnknownObj)
    External (_SB_.PC00.RP02, DeviceObj)
    External (_SB_.PC00.RP02.PRMV, IntObj)
    External (_SB_.PC00.RP02.PRTP, IntObj)
    External (_SB_.PC00.RP02.RD3C, IntObj)
    External (_SB_.PC00.RP02.SLOT, IntObj)
    External (_SB_.PC00.RP02.VDID, UnknownObj)
    External (_SB_.PC00.RP03, DeviceObj)
    External (_SB_.PC00.RP03.PRMV, IntObj)
    External (_SB_.PC00.RP03.PRTP, IntObj)
    External (_SB_.PC00.RP03.RD3C, IntObj)
    External (_SB_.PC00.RP03.SLOT, IntObj)
    External (_SB_.PC00.RP03.VDID, UnknownObj)
    External (_SB_.PC00.RP04, DeviceObj)
    External (_SB_.PC00.RP04.PRMV, IntObj)
    External (_SB_.PC00.RP04.PRTP, IntObj)
    External (_SB_.PC00.RP04.RD3C, IntObj)
    External (_SB_.PC00.RP04.SLOT, IntObj)
    External (_SB_.PC00.RP04.VDID, UnknownObj)
    External (_SB_.PC00.RP05, DeviceObj)
    External (_SB_.PC00.RP05.PRMV, IntObj)
    External (_SB_.PC00.RP05.PRTP, IntObj)
    External (_SB_.PC00.RP05.RD3C, IntObj)
    External (_SB_.PC00.RP05.SLOT, IntObj)
    External (_SB_.PC00.RP05.VDID, UnknownObj)
    External (_SB_.PC00.RP06, DeviceObj)
    External (_SB_.PC00.RP06.PRMV, IntObj)
    External (_SB_.PC00.RP06.PRTP, IntObj)
    External (_SB_.PC00.RP06.RD3C, IntObj)
    External (_SB_.PC00.RP06.SLOT, IntObj)
    External (_SB_.PC00.RP06.VDID, UnknownObj)
    External (_SB_.PC00.RP07, DeviceObj)
    External (_SB_.PC00.RP07.PRMV, IntObj)
    External (_SB_.PC00.RP07.PRTP, IntObj)
    External (_SB_.PC00.RP07.RD3C, IntObj)
    External (_SB_.PC00.RP07.SLOT, IntObj)
    External (_SB_.PC00.RP07.VDID, UnknownObj)
    External (_SB_.PC00.RP08, DeviceObj)
    External (_SB_.PC00.RP08.PRMV, IntObj)
    External (_SB_.PC00.RP08.PRTP, IntObj)
    External (_SB_.PC00.RP08.RD3C, IntObj)
    External (_SB_.PC00.RP08.SLOT, IntObj)
    External (_SB_.PC00.RP08.VDID, UnknownObj)
    External (_SB_.PC00.RP09, DeviceObj)
    External (_SB_.PC00.RP09.CEMP, MethodObj)    // 1 Arguments
    External (_SB_.PC00.RP09.D3HT, FieldUnitObj)
    External (_SB_.PC00.RP09.DHDW, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.DL23, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.DVES, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.EHDW, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.HBSL, FieldUnitObj)
    External (_SB_.PC00.RP09.L23D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.PCPB, IntObj)
    External (_SB_.PC00.RP09.PRMV, IntObj)
    External (_SB_.PC00.RP09.PRTP, IntObj)
    External (_SB_.PC00.RP09.PXSX, DeviceObj)
    External (_SB_.PC00.RP09.PXSX.PAHC, MethodObj)    // 0 Arguments
    External (_SB_.PC00.RP09.RD3C, IntObj)
    External (_SB_.PC00.RP09.SLOT, IntObj)
    External (_SB_.PC00.RP09.TMCS, IntObj)
    External (_SB_.PC00.RP09.VDID, UnknownObj)
    External (_SB_.PC00.RP10, DeviceObj)
    External (_SB_.PC00.RP10.PRMV, IntObj)
    External (_SB_.PC00.RP10.PRTP, IntObj)
    External (_SB_.PC00.RP10.RD3C, IntObj)
    External (_SB_.PC00.RP10.SLOT, IntObj)
    External (_SB_.PC00.RP10.VDID, UnknownObj)
    External (_SB_.PC00.RP11, DeviceObj)
    External (_SB_.PC00.RP11.PRMV, IntObj)
    External (_SB_.PC00.RP11.PRTP, IntObj)
    External (_SB_.PC00.RP11.RD3C, IntObj)
    External (_SB_.PC00.RP11.SLOT, IntObj)
    External (_SB_.PC00.RP11.VDID, UnknownObj)
    External (_SB_.PC00.RP12, DeviceObj)
    External (_SB_.PC00.RP12.PRMV, IntObj)
    External (_SB_.PC00.RP12.PRTP, IntObj)
    External (_SB_.PC00.RP12.RD3C, IntObj)
    External (_SB_.PC00.RP12.SLOT, IntObj)
    External (_SB_.PC00.RP12.VDID, UnknownObj)
    External (_SB_.PC00.RP13, DeviceObj)
    External (_SB_.PC00.RP13.PRMV, IntObj)
    External (_SB_.PC00.RP13.PRTP, IntObj)
    External (_SB_.PC00.RP13.RD3C, IntObj)
    External (_SB_.PC00.RP13.SLOT, IntObj)
    External (_SB_.PC00.RP13.VDID, UnknownObj)
    External (_SB_.PC00.RP14, DeviceObj)
    External (_SB_.PC00.RP14.PRMV, IntObj)
    External (_SB_.PC00.RP14.PRTP, IntObj)
    External (_SB_.PC00.RP14.RD3C, IntObj)
    External (_SB_.PC00.RP14.SLOT, IntObj)
    External (_SB_.PC00.RP14.VDID, UnknownObj)
    External (_SB_.PC00.RP15, DeviceObj)
    External (_SB_.PC00.RP15.PRMV, IntObj)
    External (_SB_.PC00.RP15.PRTP, IntObj)
    External (_SB_.PC00.RP15.RD3C, IntObj)
    External (_SB_.PC00.RP15.SLOT, IntObj)
    External (_SB_.PC00.RP15.VDID, UnknownObj)
    External (_SB_.PC00.RP16, DeviceObj)
    External (_SB_.PC00.RP16.PRMV, IntObj)
    External (_SB_.PC00.RP16.PRTP, IntObj)
    External (_SB_.PC00.RP16.RD3C, IntObj)
    External (_SB_.PC00.RP16.SLOT, IntObj)
    External (_SB_.PC00.RP16.VDID, UnknownObj)
    External (_SB_.PC00.RP17, DeviceObj)
    External (_SB_.PC00.RP17.PRMV, IntObj)
    External (_SB_.PC00.RP17.PRTP, IntObj)
    External (_SB_.PC00.RP17.RD3C, IntObj)
    External (_SB_.PC00.RP17.SLOT, IntObj)
    External (_SB_.PC00.RP17.VDID, UnknownObj)
    External (_SB_.PC00.RP18, DeviceObj)
    External (_SB_.PC00.RP18.PRMV, IntObj)
    External (_SB_.PC00.RP18.PRTP, IntObj)
    External (_SB_.PC00.RP18.RD3C, IntObj)
    External (_SB_.PC00.RP18.SLOT, IntObj)
    External (_SB_.PC00.RP18.VDID, UnknownObj)
    External (_SB_.PC00.RP19, DeviceObj)
    External (_SB_.PC00.RP19.PRMV, IntObj)
    External (_SB_.PC00.RP19.PRTP, IntObj)
    External (_SB_.PC00.RP19.RD3C, IntObj)
    External (_SB_.PC00.RP19.SLOT, IntObj)
    External (_SB_.PC00.RP19.VDID, UnknownObj)
    External (_SB_.PC00.RP20, DeviceObj)
    External (_SB_.PC00.RP20.PRMV, IntObj)
    External (_SB_.PC00.RP20.PRTP, IntObj)
    External (_SB_.PC00.RP20.RD3C, IntObj)
    External (_SB_.PC00.RP20.SLOT, IntObj)
    External (_SB_.PC00.RP20.VDID, UnknownObj)
    External (_SB_.PC00.RP21, DeviceObj)
    External (_SB_.PC00.RP21.PRMV, IntObj)
    External (_SB_.PC00.RP21.PRTP, IntObj)
    External (_SB_.PC00.RP21.RD3C, IntObj)
    External (_SB_.PC00.RP21.SLOT, IntObj)
    External (_SB_.PC00.RP21.VDID, UnknownObj)
    External (_SB_.PC00.RP22, DeviceObj)
    External (_SB_.PC00.RP22.PRMV, IntObj)
    External (_SB_.PC00.RP22.PRTP, IntObj)
    External (_SB_.PC00.RP22.RD3C, IntObj)
    External (_SB_.PC00.RP22.SLOT, IntObj)
    External (_SB_.PC00.RP22.VDID, UnknownObj)
    External (_SB_.PC00.RP23, DeviceObj)
    External (_SB_.PC00.RP23.PRMV, IntObj)
    External (_SB_.PC00.RP23.PRTP, IntObj)
    External (_SB_.PC00.RP23.RD3C, IntObj)
    External (_SB_.PC00.RP23.SLOT, IntObj)
    External (_SB_.PC00.RP23.VDID, UnknownObj)
    External (_SB_.PC00.RP24, DeviceObj)
    External (_SB_.PC00.RP24.PRMV, IntObj)
    External (_SB_.PC00.RP24.PRTP, IntObj)
    External (_SB_.PC00.RP24.RD3C, IntObj)
    External (_SB_.PC00.RP24.SLOT, IntObj)
    External (_SB_.PC00.RP24.VDID, UnknownObj)
    External (_SB_.PC00.RP25, DeviceObj)
    External (_SB_.PC00.RP25.PRMV, IntObj)
    External (_SB_.PC00.RP25.PRTP, IntObj)
    External (_SB_.PC00.RP25.RD3C, IntObj)
    External (_SB_.PC00.RP25.SLOT, IntObj)
    External (_SB_.PC00.RP25.VDID, UnknownObj)
    External (_SB_.PC00.RP26, DeviceObj)
    External (_SB_.PC00.RP26.PRMV, IntObj)
    External (_SB_.PC00.RP26.PRTP, IntObj)
    External (_SB_.PC00.RP26.RD3C, IntObj)
    External (_SB_.PC00.RP26.SLOT, IntObj)
    External (_SB_.PC00.RP26.VDID, UnknownObj)
    External (_SB_.PC00.RP27, DeviceObj)
    External (_SB_.PC00.RP27.PRMV, IntObj)
    External (_SB_.PC00.RP27.PRTP, IntObj)
    External (_SB_.PC00.RP27.RD3C, IntObj)
    External (_SB_.PC00.RP27.SLOT, IntObj)
    External (_SB_.PC00.RP27.VDID, UnknownObj)
    External (_SB_.PC00.RP28, DeviceObj)
    External (_SB_.PC00.RP28.PRMV, IntObj)
    External (_SB_.PC00.RP28.PRTP, IntObj)
    External (_SB_.PC00.RP28.RD3C, IntObj)
    External (_SB_.PC00.RP28.SLOT, IntObj)
    External (_SB_.PC00.RP28.VDID, UnknownObj)
    External (_SB_.PC00.SAT0, DeviceObj)
    External (_SB_.PC00.SAT0.NVM1, DeviceObj)
    External (_SB_.PC00.SAT0.NVM1._ADR, IntObj)
    External (_SB_.PC00.SAT0.NVM1.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM1.RPOF, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM1.RPON, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM2, DeviceObj)
    External (_SB_.PC00.SAT0.NVM2._ADR, IntObj)
    External (_SB_.PC00.SAT0.NVM2.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM2.RPOF, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM2.RPON, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM3, DeviceObj)
    External (_SB_.PC00.SAT0.NVM3._ADR, IntObj)
    External (_SB_.PC00.SAT0.NVM3.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM3.RPOF, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.NVM3.RPON, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT0, DeviceObj)
    External (_SB_.PC00.SAT0.PRT0._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT0.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT0.PRES, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT0.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT1, DeviceObj)
    External (_SB_.PC00.SAT0.PRT1._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT1.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT1.PRES, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT1.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT2, DeviceObj)
    External (_SB_.PC00.SAT0.PRT2._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT2.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT2.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT3, DeviceObj)
    External (_SB_.PC00.SAT0.PRT3._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT3.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT3.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT4, DeviceObj)
    External (_SB_.PC00.SAT0.PRT4._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT4.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT4.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT5, DeviceObj)
    External (_SB_.PC00.SAT0.PRT5._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT5.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT5.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT6, DeviceObj)
    External (_SB_.PC00.SAT0.PRT6._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT6.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT6.PWRG, UnknownObj)
    External (_SB_.PC00.SAT0.PRT7, DeviceObj)
    External (_SB_.PC00.SAT0.PRT7._ADR, IntObj)
    External (_SB_.PC00.SAT0.PRT7.IR3D, MethodObj)    // 0 Arguments
    External (_SB_.PC00.SAT0.PRT7.PWRG, UnknownObj)
    External (_SB_.PC00.XDCI, DeviceObj)
    External (_SB_.PC00.XDCI.D0I3, UnknownObj)
    External (_SB_.PC00.XDCI.XDCB, UnknownObj)
    External (_SB_.PC00.XHCI, DeviceObj)
    External (_SB_.PC00.XHCI.MEMB, UnknownObj)
    External (_SB_.PC00.XHCI.RHUB, DeviceObj)
    External (_SB_.PC00.XHCI.RHUB.HS01, DeviceObj)
    External (_SB_.PC00.XHCI.RHUB.HS02, DeviceObj)
    External (_SB_.PC00.XHCI.RHUB.SS01, DeviceObj)
    External (_SB_.PC00.XHCI.RHUB.SS02, DeviceObj)
    External (_SB_.PSD0, MethodObj)    // 1 Arguments
    External (_SB_.PSD3, MethodObj)    // 1 Arguments
    External (_SB_.SHPO, MethodObj)    // 2 Arguments
    External (AUDD, FieldUnitObj)
    External (DGBA, FieldUnitObj)
    External (DGOP, FieldUnitObj)
    External (DTFS, IntObj)
    External (DTOE, IntObj)
    External (DTRC, IntObj)
    External (DTRD, IntObj)
    External (DTRO, IntObj)
    External (DVID, UnknownObj)
    External (GBEP, UnknownObj)
    External (GBES, UnknownObj)
    External (HBCL, FieldUnitObj)
    External (HBPL, FieldUnitObj)
    External (IC0D, FieldUnitObj)
    External (IC1D, FieldUnitObj)
    External (IC1S, FieldUnitObj)
    External (P1WK, FieldUnitObj)
    External (PEP0, UnknownObj)
    External (PIN_.OFF_, MethodObj)    // 1 Arguments
    External (PIN_.ON__, MethodObj)    // 1 Arguments
    External (PIN_.STA_, MethodObj)    // 1 Arguments
    External (RCG0, IntObj)
    External (RCG1, IntObj)
    External (RPS0, IntObj)
    External (RPT0, IntObj)
    External (S0ID, UnknownObj)
    External (SATP, UnknownObj)
    External (SDPP, UnknownObj)
    External (SDRP, UnknownObj)
    External (SDS0, FieldUnitObj)
    External (SDS1, FieldUnitObj)
    External (SHSB, FieldUnitObj)
    External (SPCO, MethodObj)    // 2 Arguments
    External (SPCX, MethodObj)    // 3 Arguments
    External (SSDP, UnknownObj)
    External (SSDR, UnknownObj)
    External (STD3, FieldUnitObj)
    External (STPP, UnknownObj)
    External (TBPE, IntObj)
    External (TEDC, IntObj)
    External (TOFF, IntObj)
    External (UAMS, UnknownObj)
    External (VRRD, FieldUnitObj)
    External (WLWK, UnknownObj)
    External (XDCE, UnknownObj)
    External (XDST, IntObj)
    External (XHPR, UnknownObj)

    If ((GBES != Zero)){}
    Scope (\_SB.PC00.RP09)
    {
        Name (RSTG, Package (0x02)
        {
            Zero, 
            Zero
        })
        RSTG [Zero] = SSDR /* External reference */
        RSTG [One] = SDRP /* External reference */
        Name (PWRG, Package (0x02)
        {
            Zero, 
            Zero
        })
        PWRG [Zero] = SSDP /* External reference */
        PWRG [One] = SDPP /* External reference */
        Name (WAKG, Zero)
        Name (WAKP, Zero)
        Name (SCLK, Zero)
        Name (WKEN, Zero)
        Name (WOFF, Zero)
        Name (LNRD, Zero)
        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
        {
            If (CondRefOf (RD3C))
            {
                If ((RD3C == 0x02))
                {
                    Return (0x04)
                }
            }

            Return (Zero)
        }

        Method (_DSW, 3, NotSerialized)  // _DSW: Device Sleep Wake
        {
            If (Arg1)
            {
                WKEN = One
            }
            ElseIf ((Arg0 && Arg2))
            {
                WKEN = One
            }
            Else
            {
                WKEN = Zero
            }
        }

        Method (PPS0, 0, Serialized)
        {
        }

        Method (PPS3, 0, Serialized)
        {
        }

        PowerResource (PXP, 0x00, 0x0000)
        {
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If ((VDID == 0xFFFFFFFF))
                {
                    Return (Zero)
                }

                If ((GPRS () == Zero))
                {
                    Return (Zero)
                }

                Return (PSTA ())
            }

            Method (_ON, 0, NotSerialized)  // _ON_: Power On
            {
                If ((VDID == 0xFFFFFFFF)){}
                ElseIf ((GPRS () == Zero)){}
                Else
                {
                    PON ()
                    L23D ()
                }
            }

            Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
            {
                If ((VDID == 0xFFFFFFFF)){}
                ElseIf ((GPRS () == Zero)){}
                Else
                {
                    DL23 ()
                    POFF ()
                }
            }
        }

        Method (GPPR, 0, NotSerialized)
        {
            If (CondRefOf (WAKP))
            {
                If (((WAKP != Zero) && (WKEN == Zero)))
                {
                    Return (Zero)
                }
            }

            If (CondRefOf (PCPB))
            {
                If ((PCPB != Zero))
                {
                    Return (Zero)
                }
            }

            If (CondRefOf (DVES))
            {
                If ((DVES () == Zero))
                {
                    Return (Zero)
                }
            }

            Return (One)
        }

        Method (GPRS, 0, NotSerialized)
        {
            If ((CondRefOf (PRTP) && (PRTP == 0x04)))
            {
                If (CondRefOf (\HBCL))
                {
                    If (((\HBCL != 0xFF) || (\HBCL <= 0x02)))
                    {
                        If ((\HBCL == SLOT))
                        {
                            Return (Zero)
                        }
                    }
                }
            }

            If ((CondRefOf (PRTP) && (PRTP == 0x02)))
            {
                If (CondRefOf (HBSL))
                {
                    Local0 = ((SLOT - One) / 0x04)
                    If ((HBSL & (One << Local0)))
                    {
                        Return (Zero)
                    }
                }

                If ((CondRefOf (\HBCL) && CondRefOf (\HBPL)))
                {
                    If (((\HBCL != 0xFF) || (\HBCL <= 0x02)))
                    {
                        If ((\HBPL == (SLOT - One)))
                        {
                            Return (Zero)
                        }
                    }
                }
            }

            If (CondRefOf (RD3C))
            {
                If ((RD3C != 0x02))
                {
                    Return (Zero)
                }
            }

            If (CondRefOf (PRMV))
            {
                If ((PRMV == One))
                {
                    Return (Zero)
                }
            }

            Return (One)
        }

        Method (PSTA, 0, NotSerialized)
        {
            If (\PIN.STA (RSTG))
            {
                Return (Zero)
            }
            Else
            {
                Return (One)
            }
        }

        Method (PON, 0, NotSerialized)
        {
            If (CondRefOf (CEMP))
            {
                CEMP (One)
            }

            If (CondRefOf (WAKG))
            {
                \_SB.SHPO (WAKG, One)
                \_SB.CAGS (WAKG)
            }

            If (CondRefOf (DHDW))
            {
                DHDW ()
            }

            If ((CondRefOf (PRTP) && (PRTP == 0x02)))
            {
                \_SB.PSD0 (SLOT)
            }

            If (CondRefOf (PWRG))
            {
                If (CondRefOf (WOFF))
                {
                    If ((WOFF != Zero))
                    {
                        Local0 = ((Timer - WOFF) / 0x2710)
                        If ((Local0 < 0xC8))
                        {
                            Sleep ((0xC8 - Local0))
                        }

                        WOFF = Zero
                    }
                }

                \PIN.ON (PWRG)
                Sleep (PEP0)
            }

            If (CondRefOf (SCLK))
            {
                If (CondRefOf (TMCS))
                {
                    SPCX (SCLK, One, TMCS)
                }
                Else
                {
                    SPCO (SCLK, One)
                }
            }

            \PIN.OFF (RSTG)
        }

        Method (POFF, 0, NotSerialized)
        {
            Local1 = (LNRD / 0x03E8)
            Sleep (Local1)
            \PIN.ON (RSTG)
            If ((CondRefOf (PRTP) && (PRTP == 0x02)))
            {
                \_SB.PSD3 (SLOT)
            }

            If (CondRefOf (SCLK))
            {
                If (CondRefOf (TMCS))
                {
                    SPCX (SCLK, Zero, TMCS)
                }
                Else
                {
                    SPCO (SCLK, Zero)
                }
            }

            If (CondRefOf (PWRG))
            {
                If ((GPPR () == One))
                {
                    \PIN.OFF (PWRG)
                }

                If (CondRefOf (WOFF))
                {
                    WOFF = Timer
                }
            }

            If (CondRefOf (WAKG))
            {
                If (((WAKG != Zero) && WKEN))
                {
                    \_SB.SHPO (WAKG, Zero)
                }
            }

            If (CondRefOf (EHDW))
            {
                EHDW ()
            }

            If (CondRefOf (CEMP))
            {
                CEMP (Zero)
            }
        }

        Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
        {
            Return (Package (0x01)
            {
                PXP
            })
        }

        Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
        {
            Return (Package (0x01)
            {
                PXP
            })
        }

        Method (UPRD, 1, Serialized)
        {
            If ((Arg0 <= 0x2710))
            {
                LNRD = Arg0
            }

            Return (LNRD) /* \_SB_.PC00.RP09.LNRD */
        }

        Scope (\_SB.PC00.RP09.PXSX)
        {
            Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
            {
                If (CondRefOf (^^RD3C))
                {
                    If ((^^RD3C == 0x02))
                    {
                        Return (0x04)
                    }
                }

                Return (0x03)
            }

            Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
            {
                Return (^^_PR0 ())
            }

            Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
            {
                Return (^^_PR0 ())
            }

            Method (_PS0, 0, Serialized)  // _PS0: Power State 0
            {
            }

            Method (_PS3, 0, Serialized)  // _PS3: Power State 3
            {
            }

            Method (_DSD, 0, NotSerialized)  // _DSD: Device-Specific Data
            {
                Return (Package (0x02)
                {
                    ToUUID ("5025030f-842f-4ab4-a561-99a5189762d0") /* Unknown UUID */, 
                    Package (0x01)
                    {
                        Package (0x02)
                        {
                            "StorageD3Enable", 
                            One
                        }
                    }
                })
            }

            Device (MINI)
            {
                Method (_STA, 0, Serialized)  // _STA: Status
                {
                    If (CondRefOf (PAHC))
                    {
                        If (PAHC ())
                        {
                            Return (0x0F)
                        }
                    }

                    Return (Zero)
                }

                Name (_ADR, 0xFFFF)  // _ADR: Address
                Name (_S0W, 0x04)  // _S0W: S0 Device Wake State
                Method (_PS0, 0, Serialized)  // _PS0: Power State 0
                {
                }

                Method (_PS3, 0, Serialized)  // _PS3: Power State 3
                {
                }

                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                {
                    Return (^^^_PR0 ())
                }

                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                {
                    Return (^^^_PR0 ())
                }
            }
        }
    }

    Scope (\_SB.PC00.SAT0)
    {
        Scope (PRT0)
        {
            If (PRES ())
            {
                Name (PWRG, Package (0x02)
                {
                    Zero, 
                    Zero
                })
                PWRG [Zero] = SATP /* External reference */
                PWRG [One] = STPP /* External reference */
            }
        }

        Scope (PRT1)
        {
            If (PRES ())
            {
                Name (PWRG, Package (0x02)
                {
                    Zero, 
                    Zero
                })
                PWRG [Zero] = SSDP /* External reference */
                PWRG [One] = SDPP /* External reference */
            }
        }

        Scope (\_SB.PC00.SAT0)
        {
            OperationRegion (SMIO, PCI_Config, 0x24, 0x04)
            Field (SMIO, AnyAcc, NoLock, Preserve)
            {
                MBR6,   32
            }

            OperationRegion (PCIR, PCI_Config, Zero, 0x10)
            Field (PCIR, DWordAcc, NoLock, Preserve)
            {
                Offset (0x0A), 
                SUBC,   8
            }

            If ((One & RCG1))
            {
                Scope (PRT0)
                {
                    Name (PBAR, 0x0118)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT0.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x02 & RCG1))
            {
                Scope (PRT1)
                {
                    Name (PBAR, 0x0198)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT1.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x04 & RCG1))
            {
                Scope (PRT2)
                {
                    Name (PBAR, 0x0218)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT2.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x08 & RCG1))
            {
                Scope (PRT3)
                {
                    Name (PBAR, 0x0298)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT3.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x10 & RCG1))
            {
                Scope (PRT4)
                {
                    Name (PBAR, 0x0318)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT4.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x20 & RCG1))
            {
                Scope (PRT5)
                {
                    Name (PBAR, 0x0398)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT5.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x40 & RCG1))
            {
                Scope (PRT6)
                {
                    Name (PBAR, 0x0418)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT6.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x80 & RCG1))
            {
                Scope (PRT7)
                {
                    Name (PBAR, 0x0498)
                    If (CondRefOf (PWRG))
                    {
                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                Method (_PR0, 0, NotSerialized)  // _PR0: Power Resources for D0
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }

                                Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
                                {
                                    Return (Package (0x01)
                                    {
                                        STPR
                                    })
                                }
                            }
                        }

                        Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                        {
                            If (CondRefOf (\STD3))
                            {
                                If ((\STD3 == 0x02))
                                {
                                    Return (0x04)
                                }

                                Return (0x03)
                            }

                            Return (0x03)
                        }

                        If (CondRefOf (\STD3))
                        {
                            If ((\STD3 == 0x02))
                            {
                                PowerResource (STPR, 0x00, 0x0000)
                                {
                                    Method (_STA, 0, NotSerialized)  // _STA: Status
                                    {
                                        Return (SPSA ())
                                    }

                                    Method (_ON, 0, NotSerialized)  // _ON_: Power On
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPON ()
                                        Sleep (0x10)
                                    }

                                    Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                                    {
                                        If (IR3D ())
                                        {
                                            Return (Zero)
                                        }

                                        SPOF ()
                                    }
                                }
                            }
                        }

                        Name (OFTM, Zero)
                        Method (SPSA, 0, NotSerialized)
                        {
                            Return (\PIN.STA (PWRG))
                        }

                        Method (SPON, 0, NotSerialized)
                        {
                            \PIN.ON (PWRG)
                        }

                        Method (SPOF, 0, Serialized)
                        {
                            Local0 = (\_SB.PC00.SAT0.MBR6 + PBAR) /* \_SB_.PC00.SAT0.PRT7.PBAR */
                            If ((S0ID == One))
                            {
                                OperationRegion (PSTS, SystemMemory, Local0, 0x18)
                                Field (PSTS, DWordAcc, NoLock, Preserve)
                                {
                                    CMST,   1, 
                                    CSUD,   1, 
                                        ,   2, 
                                    CFRE,   1, 
                                    Offset (0x10), 
                                    SDET,   4, 
                                    Offset (0x14), 
                                    CDET,   4
                                }

                                If (((\_SB.PC00.SAT0.SUBC == 0x06) && ((SDET == One) || (SDET == 0x03))))
                                {
                                    CMST = Zero
                                    CFRE = Zero
                                    CSUD = Zero
                                    CDET = 0x04
                                    Sleep (0x10)
                                    While ((SDET != 0x04))
                                    {
                                        Sleep (0x10)
                                    }
                                }
                            }

                            \PIN.OFF (PWRG)
                            ^OFTM = Timer
                        }
                    }
                }
            }

            If ((0x0100 & RCG1))
            {
                Scope (NVM1)
                {
                    Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                    {
                        Return (0x04)
                    }

                    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
                    {
                        NVPR
                    })
                    Name (_PR3, Package (0x01)  // _PR3: Power Resources for D3hot
                    {
                        NVPR
                    })
                    PowerResource (NVPR, 0x00, 0x0000)
                    {
                        Name (_STA, One)  // _STA: Status
                        Method (_ON, 0, NotSerialized)  // _ON_: Power On
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPON ()
                            _STA = One
                        }

                        Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPOF ()
                            _STA = Zero
                        }
                    }
                }
            }

            If ((0x0200 & RCG1))
            {
                Scope (NVM2)
                {
                    Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                    {
                        Return (0x04)
                    }

                    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
                    {
                        NVPR
                    })
                    Name (_PR3, Package (0x01)  // _PR3: Power Resources for D3hot
                    {
                        NVPR
                    })
                    PowerResource (NVPR, 0x00, 0x0000)
                    {
                        Name (_STA, One)  // _STA: Status
                        Method (_ON, 0, NotSerialized)  // _ON_: Power On
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPON ()
                            _STA = One
                        }

                        Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPOF ()
                            _STA = Zero
                        }
                    }
                }
            }

            If ((0x0400 & RCG1))
            {
                Scope (NVM3)
                {
                    Method (_S0W, 0, NotSerialized)  // _S0W: S0 Device Wake State
                    {
                        Return (0x04)
                    }

                    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
                    {
                        NVPR
                    })
                    Name (_PR3, Package (0x01)  // _PR3: Power Resources for D3hot
                    {
                        NVPR
                    })
                    PowerResource (NVPR, 0x00, 0x0000)
                    {
                        Name (_STA, One)  // _STA: Status
                        Method (_ON, 0, NotSerialized)  // _ON_: Power On
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPON ()
                            _STA = One
                        }

                        Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                        {
                            If (IR3D ())
                            {
                                Return (Zero)
                            }

                            RPOF ()
                            _STA = Zero
                        }
                    }
                }
            }
        }
    }

    If ((XDCE == One))
    {
        Scope (\_SB)
        {
            PowerResource (USBC, 0x00, 0x0000)
            {
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Method (_ON, 0, NotSerialized)  // _ON_: Power On
                {
                }

                Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
                {
                }
            }
        }

        Scope (\_SB.PC00.XDCI)
        {
            OperationRegion (GENR, SystemMemory, ((XDCB & 0xFFFFFFFFFFFFFF00) + 0x0010F81C), 0x04)
            Field (GENR, WordAcc, NoLock, Preserve)
            {
                    ,   2, 
                CPME,   1, 
                U3EN,   1, 
                U2EN,   1
            }

            Method (_PS3, 0, NotSerialized)  // _PS3: Power State 3
            {
                CPME = One
                U2EN = One
                U3EN = One
                \_SB.CSD3 (0x17)
            }

            Method (_PS0, 0, NotSerialized)  // _PS0: Power State 0
            {
                CPME = Zero
                U2EN = Zero
                U3EN = Zero
                If ((DVID == 0xFFFF))
                {
                    Return (Zero)
                }

                \_SB.CSD0 (0x17)
            }

            Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
            {
                Return (Zero)
            }

            Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
            {
                Return (Package (0x01)
                {
                    USBC
                })
            }
        }
    }

    Scope (\_SB.PC00)
    {
        PowerResource (PAUD, 0x00, 0x0000)
        {
            Name (PSTA, One)
            Name (ONTM, Zero)
            Name (_STA, One)  // _STA: Status
            Method (_ON, 0, NotSerialized)  // _ON_: Power On
            {
                _STA = One
                PUAM ()
            }

            Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
            {
                _STA = Zero
                PUAM ()
            }

            Method (PUAM, 0, Serialized)
            {
                If (((^_STA == Zero) && (\UAMS != Zero))){}
                ElseIf ((^PSTA != One))
                {
                    ^PSTA = One
                    ^ONTM = Timer
                }
            }
        }
    }

    If ((\_SB.PC00.HDAS.VDID != 0xFFFFFFFF))
    {
        Scope (\_SB.PC00.HDAS)
        {
            Method (PS0X, 0, Serialized)
            {
                If ((\_SB.PC00.PAUD.ONTM == Zero))
                {
                    Return (Zero)
                }

                Local0 = ((Timer - \_SB.PC00.PAUD.ONTM) / 0x2710)
                Local1 = (AUDD + VRRD) /* External reference */
                If ((Local0 < Local1))
                {
                    Sleep ((Local1 - Local0))
                }
            }

            Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
            {
                \_SB.PC00.PAUD
            })
        }
    }

    Scope (\_GPE)
    {
        Method (AL6F, 0, NotSerialized)
        {
            If (\_SB.ISME (WLWK))
            {
                \_SB.SHPO (WLWK, One)
                Notify (\_SB.PC00.RP07, 0x02) // Device Wake
                \_SB.CAGS (WLWK)
            }
        }
    }
}

