/*
 * RSP Interpreter plug in for Project 64 (A Nintendo 64 emulator).
 *
 * (c) Copyright 2001 zilmar (zilmar@emulation64.com) 
 *
 * RSP Interpreter plug in homepage: www.pj64.net
 *
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * RSP Interpreter plug in is freeware for PERSONAL USE only. 
 * Commercial users should seek permission of the copyright holders first. 
 * Commercial use includes charging money for RSP Interpreter plug in or 
 * software derived from RSP Interpreter plug in.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */
#include <windows.h>
#include <stdio.h>
#include "RSP.h"
#include "RSP_Cpu.h"
#include "RSP_opcodes.h"
#include "RSP_registers.h"
#include "RSP_Command.h"
#include "memory.h"
#include "opcode.h"
#include "breakpoint.h"

#if ( !(defined(DLIST) & defined(ALIST)) )
UDWORD EleSpec[32], Indx[32];

void * RSP_Opcode[64];
void * RSP_RegImm[32];
void * RSP_Special[64];
void * RSP_Cop0[32];
void * RSP_Cop2[32];
void * RSP_Vector[64];
void * RSP_Lc2[32];
void * RSP_Sc2[32];
OPCODE RSPOpC;

DWORD RSP_NextInstruction, RSP_JumpTo, * PrgCount, RSP_Running;

void Build_RSP ( void ) {
	int i;

	RSP_Opcode[ 0] = RSP_Opcode_SPECIAL;
	RSP_Opcode[ 1] = RSP_Opcode_REGIMM;
	RSP_Opcode[ 2] = RSP_Opcode_J;
	RSP_Opcode[ 3] = RSP_Opcode_JAL;
	RSP_Opcode[ 4] = RSP_Opcode_BEQ;
	RSP_Opcode[ 5] = RSP_Opcode_BNE;
	RSP_Opcode[ 6] = RSP_Opcode_BLEZ;
	RSP_Opcode[ 7] = RSP_Opcode_BGTZ;
	RSP_Opcode[ 8] = RSP_Opcode_ADDI;
	RSP_Opcode[ 9] = RSP_Opcode_ADDIU;
	RSP_Opcode[10] = RSP_Opcode_SLTI;
	RSP_Opcode[11] = RSP_Opcode_SLTIU;
	RSP_Opcode[12] = RSP_Opcode_ANDI;
	RSP_Opcode[13] = RSP_Opcode_ORI;
	RSP_Opcode[14] = RSP_Opcode_XORI;
	RSP_Opcode[15] = RSP_Opcode_LUI;
	RSP_Opcode[16] = RSP_Opcode_COP0;
	RSP_Opcode[17] = rsp_UnknownOpcode;
	RSP_Opcode[18] = RSP_Opcode_COP2;
	RSP_Opcode[19] = rsp_UnknownOpcode;
	RSP_Opcode[20] = rsp_UnknownOpcode;
	RSP_Opcode[21] = rsp_UnknownOpcode;
	RSP_Opcode[22] = rsp_UnknownOpcode;
	RSP_Opcode[23] = rsp_UnknownOpcode;
	RSP_Opcode[24] = rsp_UnknownOpcode;
	RSP_Opcode[25] = rsp_UnknownOpcode;
	RSP_Opcode[26] = rsp_UnknownOpcode;
	RSP_Opcode[27] = rsp_UnknownOpcode;
	RSP_Opcode[28] = rsp_UnknownOpcode;
	RSP_Opcode[29] = rsp_UnknownOpcode;
	RSP_Opcode[30] = rsp_UnknownOpcode;
	RSP_Opcode[31] = rsp_UnknownOpcode;
	RSP_Opcode[32] = RSP_Opcode_LB;
	RSP_Opcode[33] = RSP_Opcode_LH;
	RSP_Opcode[34] = rsp_UnknownOpcode;
	RSP_Opcode[35] = RSP_Opcode_LW;
	RSP_Opcode[36] = RSP_Opcode_LBU;
	RSP_Opcode[37] = RSP_Opcode_LHU;
	RSP_Opcode[38] = rsp_UnknownOpcode;
	RSP_Opcode[39] = rsp_UnknownOpcode;
	RSP_Opcode[40] = RSP_Opcode_SB;
	RSP_Opcode[41] = RSP_Opcode_SH;
	RSP_Opcode[42] = rsp_UnknownOpcode;
	RSP_Opcode[43] = RSP_Opcode_SW;
	RSP_Opcode[44] = rsp_UnknownOpcode;
	RSP_Opcode[45] = rsp_UnknownOpcode;
	RSP_Opcode[46] = rsp_UnknownOpcode;
	RSP_Opcode[47] = rsp_UnknownOpcode;
	RSP_Opcode[48] = rsp_UnknownOpcode;
	RSP_Opcode[49] = rsp_UnknownOpcode;
	RSP_Opcode[50] = RSP_Opcode_LC2;
	RSP_Opcode[51] = rsp_UnknownOpcode;
	RSP_Opcode[52] = rsp_UnknownOpcode;
	RSP_Opcode[53] = rsp_UnknownOpcode;
	RSP_Opcode[54] = rsp_UnknownOpcode;
	RSP_Opcode[55] = rsp_UnknownOpcode;
	RSP_Opcode[56] = rsp_UnknownOpcode;
	RSP_Opcode[57] = rsp_UnknownOpcode;
	RSP_Opcode[58] = RSP_Opcode_SC2;
	RSP_Opcode[59] = rsp_UnknownOpcode;
	RSP_Opcode[60] = rsp_UnknownOpcode;
	RSP_Opcode[61] = rsp_UnknownOpcode;
	RSP_Opcode[62] = rsp_UnknownOpcode;
	RSP_Opcode[63] = rsp_UnknownOpcode;

	RSP_Special[ 0] = RSP_Special_SLL;
	RSP_Special[ 1] = rsp_UnknownOpcode;
	RSP_Special[ 2] = RSP_Special_SRL;
	RSP_Special[ 3] = RSP_Special_SRA;
	RSP_Special[ 4] = RSP_Special_SLLV;
	RSP_Special[ 5] = rsp_UnknownOpcode;
	RSP_Special[ 6] = RSP_Special_SRLV;
	RSP_Special[ 7] = RSP_Special_SRAV;
	RSP_Special[ 8] = RSP_Special_JR;
	RSP_Special[ 9] = RSP_Special_JALR;
	RSP_Special[10] = rsp_UnknownOpcode;
	RSP_Special[11] = rsp_UnknownOpcode;
	RSP_Special[12] = rsp_UnknownOpcode;
	RSP_Special[13] = RSP_Special_BREAK;
	RSP_Special[14] = rsp_UnknownOpcode;
	RSP_Special[15] = rsp_UnknownOpcode;
	RSP_Special[16] = rsp_UnknownOpcode;
	RSP_Special[17] = rsp_UnknownOpcode;
	RSP_Special[18] = rsp_UnknownOpcode;
	RSP_Special[19] = rsp_UnknownOpcode;
	RSP_Special[20] = rsp_UnknownOpcode;
	RSP_Special[21] = rsp_UnknownOpcode;
	RSP_Special[22] = rsp_UnknownOpcode;
	RSP_Special[23] = rsp_UnknownOpcode;
	RSP_Special[24] = rsp_UnknownOpcode;
	RSP_Special[25] = rsp_UnknownOpcode;
	RSP_Special[26] = rsp_UnknownOpcode;
	RSP_Special[27] = rsp_UnknownOpcode;
	RSP_Special[28] = rsp_UnknownOpcode;
	RSP_Special[29] = rsp_UnknownOpcode;
	RSP_Special[30] = rsp_UnknownOpcode;
	RSP_Special[31] = rsp_UnknownOpcode;
	RSP_Special[32] = RSP_Special_ADD;
	RSP_Special[33] = RSP_Special_ADDU;
	RSP_Special[34] = RSP_Special_SUB;
	RSP_Special[35] = RSP_Special_SUBU;
	RSP_Special[36] = RSP_Special_AND;
	RSP_Special[37] = RSP_Special_OR;
	RSP_Special[38] = RSP_Special_XOR;
	RSP_Special[39] = RSP_Special_NOR;
	RSP_Special[40] = rsp_UnknownOpcode;
	RSP_Special[41] = rsp_UnknownOpcode;
	RSP_Special[42] = RSP_Special_SLT;
	RSP_Special[43] = RSP_Special_SLTU;
	RSP_Special[44] = rsp_UnknownOpcode;
	RSP_Special[45] = rsp_UnknownOpcode;
	RSP_Special[46] = rsp_UnknownOpcode;
	RSP_Special[47] = rsp_UnknownOpcode;
	RSP_Special[48] = rsp_UnknownOpcode;
	RSP_Special[49] = rsp_UnknownOpcode;
	RSP_Special[50] = rsp_UnknownOpcode;
	RSP_Special[51] = rsp_UnknownOpcode;
	RSP_Special[52] = rsp_UnknownOpcode;
	RSP_Special[53] = rsp_UnknownOpcode;
	RSP_Special[54] = rsp_UnknownOpcode;
	RSP_Special[55] = rsp_UnknownOpcode;
	RSP_Special[56] = rsp_UnknownOpcode;
	RSP_Special[57] = rsp_UnknownOpcode;
	RSP_Special[58] = rsp_UnknownOpcode;
	RSP_Special[59] = rsp_UnknownOpcode;
	RSP_Special[60] = rsp_UnknownOpcode;
	RSP_Special[61] = rsp_UnknownOpcode;
	RSP_Special[62] = rsp_UnknownOpcode;
	RSP_Special[63] = rsp_UnknownOpcode;

	RSP_RegImm[ 0] = RSP_Opcode_BLTZ;
	RSP_RegImm[ 1] = RSP_Opcode_BGEZ;
	RSP_RegImm[ 2] = rsp_UnknownOpcode;
	RSP_RegImm[ 3] = rsp_UnknownOpcode;
	RSP_RegImm[ 4] = rsp_UnknownOpcode;
	RSP_RegImm[ 5] = rsp_UnknownOpcode;
	RSP_RegImm[ 6] = rsp_UnknownOpcode;
	RSP_RegImm[ 7] = rsp_UnknownOpcode;
	RSP_RegImm[ 8] = rsp_UnknownOpcode;
	RSP_RegImm[ 9] = rsp_UnknownOpcode;
	RSP_RegImm[10] = rsp_UnknownOpcode;
	RSP_RegImm[11] = rsp_UnknownOpcode;
	RSP_RegImm[12] = rsp_UnknownOpcode;
	RSP_RegImm[13] = rsp_UnknownOpcode;
	RSP_RegImm[14] = rsp_UnknownOpcode;
	RSP_RegImm[15] = rsp_UnknownOpcode;
	RSP_RegImm[16] = RSP_Opcode_BLTZAL;
	RSP_RegImm[17] = RSP_Opcode_BGEZAL;
	RSP_RegImm[18] = rsp_UnknownOpcode;
	RSP_RegImm[19] = rsp_UnknownOpcode;
	RSP_RegImm[20] = rsp_UnknownOpcode;
	RSP_RegImm[21] = rsp_UnknownOpcode;
	RSP_RegImm[22] = rsp_UnknownOpcode;
	RSP_RegImm[23] = rsp_UnknownOpcode;
	RSP_RegImm[24] = rsp_UnknownOpcode;
	RSP_RegImm[25] = rsp_UnknownOpcode;
	RSP_RegImm[26] = rsp_UnknownOpcode;
	RSP_RegImm[27] = rsp_UnknownOpcode;
	RSP_RegImm[28] = rsp_UnknownOpcode;
	RSP_RegImm[29] = rsp_UnknownOpcode;
	RSP_RegImm[30] = rsp_UnknownOpcode;
	RSP_RegImm[31] = rsp_UnknownOpcode;

	RSP_Cop0[ 0] = RSP_Cop0_MF;
	RSP_Cop0[ 1] = rsp_UnknownOpcode;
	RSP_Cop0[ 2] = rsp_UnknownOpcode;
	RSP_Cop0[ 3] = rsp_UnknownOpcode;
	RSP_Cop0[ 4] = RSP_Cop0_MT;
	RSP_Cop0[ 5] = rsp_UnknownOpcode;
	RSP_Cop0[ 6] = rsp_UnknownOpcode;
	RSP_Cop0[ 7] = rsp_UnknownOpcode;
	RSP_Cop0[ 8] = rsp_UnknownOpcode;
	RSP_Cop0[ 9] = rsp_UnknownOpcode;
	RSP_Cop0[10] = rsp_UnknownOpcode;
	RSP_Cop0[11] = rsp_UnknownOpcode;
	RSP_Cop0[12] = rsp_UnknownOpcode;
	RSP_Cop0[13] = rsp_UnknownOpcode;
	RSP_Cop0[14] = rsp_UnknownOpcode;
	RSP_Cop0[15] = rsp_UnknownOpcode;
	RSP_Cop0[16] = rsp_UnknownOpcode;
	RSP_Cop0[17] = rsp_UnknownOpcode;
	RSP_Cop0[18] = rsp_UnknownOpcode;
	RSP_Cop0[19] = rsp_UnknownOpcode;
	RSP_Cop0[20] = rsp_UnknownOpcode;
	RSP_Cop0[21] = rsp_UnknownOpcode;
	RSP_Cop0[22] = rsp_UnknownOpcode;
	RSP_Cop0[23] = rsp_UnknownOpcode;
	RSP_Cop0[24] = rsp_UnknownOpcode;
	RSP_Cop0[25] = rsp_UnknownOpcode;
	RSP_Cop0[26] = rsp_UnknownOpcode;
	RSP_Cop0[27] = rsp_UnknownOpcode;
	RSP_Cop0[28] = rsp_UnknownOpcode;
	RSP_Cop0[29] = rsp_UnknownOpcode;
	RSP_Cop0[30] = rsp_UnknownOpcode;
	RSP_Cop0[31] = rsp_UnknownOpcode;
	
	RSP_Cop2[ 0] = RSP_Cop2_MF;
	RSP_Cop2[ 1] = rsp_UnknownOpcode;
	RSP_Cop2[ 2] = RSP_Cop2_CF;
	RSP_Cop2[ 3] = rsp_UnknownOpcode;
	RSP_Cop2[ 4] = RSP_Cop2_MT;
	RSP_Cop2[ 5] = rsp_UnknownOpcode;
	RSP_Cop2[ 6] = RSP_Cop2_CT;
	RSP_Cop2[ 7] = rsp_UnknownOpcode;
	RSP_Cop2[ 8] = rsp_UnknownOpcode;
	RSP_Cop2[ 9] = rsp_UnknownOpcode;
	RSP_Cop2[10] = rsp_UnknownOpcode;
	RSP_Cop2[11] = rsp_UnknownOpcode;
	RSP_Cop2[12] = rsp_UnknownOpcode;
	RSP_Cop2[13] = rsp_UnknownOpcode;
	RSP_Cop2[14] = rsp_UnknownOpcode;
	RSP_Cop2[15] = rsp_UnknownOpcode;
	RSP_Cop2[16] = RSP_COP2_VECTOR;
	RSP_Cop2[17] = RSP_COP2_VECTOR;
	RSP_Cop2[18] = RSP_COP2_VECTOR;
	RSP_Cop2[19] = RSP_COP2_VECTOR;
	RSP_Cop2[20] = RSP_COP2_VECTOR;
	RSP_Cop2[21] = RSP_COP2_VECTOR;
	RSP_Cop2[22] = RSP_COP2_VECTOR;
	RSP_Cop2[23] = RSP_COP2_VECTOR;
	RSP_Cop2[24] = RSP_COP2_VECTOR;
	RSP_Cop2[25] = RSP_COP2_VECTOR;
	RSP_Cop2[26] = RSP_COP2_VECTOR;
	RSP_Cop2[27] = RSP_COP2_VECTOR;
	RSP_Cop2[28] = RSP_COP2_VECTOR;
	RSP_Cop2[29] = RSP_COP2_VECTOR;
	RSP_Cop2[30] = RSP_COP2_VECTOR;
	RSP_Cop2[31] = RSP_COP2_VECTOR;

	RSP_Vector[ 0] = RSP_Vector_VMULF;
	RSP_Vector[ 1] = RSP_Vector_VMULU;
	RSP_Vector[ 2] = rsp_UnknownOpcode;
	RSP_Vector[ 3] = rsp_UnknownOpcode;
	RSP_Vector[ 4] = RSP_Vector_VMUDL;
	RSP_Vector[ 5] = RSP_Vector_VMUDM;
	RSP_Vector[ 6] = RSP_Vector_VMUDN;
	RSP_Vector[ 7] = RSP_Vector_VMUDH;
	RSP_Vector[ 8] = RSP_Vector_VMACF;
	RSP_Vector[ 9] = RSP_Vector_VMACU;
	RSP_Vector[10] = rsp_UnknownOpcode;
	RSP_Vector[11] = RSP_Vector_VMACQ;
	RSP_Vector[12] = RSP_Vector_VMADL;
	RSP_Vector[13] = RSP_Vector_VMADM;
	RSP_Vector[14] = RSP_Vector_VMADN;
	RSP_Vector[15] = RSP_Vector_VMADH;
	RSP_Vector[16] = RSP_Vector_VADD;
	RSP_Vector[17] = RSP_Vector_VSUB;
	RSP_Vector[18] = rsp_UnknownOpcode;
	RSP_Vector[19] = RSP_Vector_VABS;
	RSP_Vector[20] = RSP_Vector_VADDC;
	RSP_Vector[21] = RSP_Vector_VSUBC;
	RSP_Vector[22] = rsp_UnknownOpcode;
	RSP_Vector[23] = rsp_UnknownOpcode;
	RSP_Vector[24] = rsp_UnknownOpcode;
	RSP_Vector[25] = rsp_UnknownOpcode;
	RSP_Vector[26] = rsp_UnknownOpcode;
	RSP_Vector[27] = rsp_UnknownOpcode;
	RSP_Vector[28] = rsp_UnknownOpcode;
	RSP_Vector[29] = RSP_Vector_VSAW;
	RSP_Vector[30] = rsp_UnknownOpcode;
	RSP_Vector[31] = rsp_UnknownOpcode;
	RSP_Vector[32] = RSP_Vector_VLT;
	RSP_Vector[33] = RSP_Vector_VEQ;
	RSP_Vector[34] = RSP_Vector_VNE;
	RSP_Vector[35] = RSP_Vector_VGE;
	RSP_Vector[36] = RSP_Vector_VCL;
	RSP_Vector[37] = RSP_Vector_VCH;
	RSP_Vector[38] = RSP_Vector_VCR;
	RSP_Vector[39] = RSP_Vector_VMRG;
	RSP_Vector[40] = RSP_Vector_VAND;
	RSP_Vector[41] = RSP_Vector_VNAND;
	RSP_Vector[42] = RSP_Vector_VOR;
	RSP_Vector[43] = RSP_Vector_VNOR;
	RSP_Vector[44] = RSP_Vector_VXOR;
	RSP_Vector[45] = RSP_Vector_VNXOR;
	RSP_Vector[46] = rsp_UnknownOpcode;
	RSP_Vector[47] = rsp_UnknownOpcode;
	RSP_Vector[48] = RSP_Vector_VRCP;
	RSP_Vector[49] = RSP_Vector_VRCPL;
	RSP_Vector[50] = RSP_Vector_VRCPH;
	RSP_Vector[51] = RSP_Vector_VMOV;
	RSP_Vector[52] = RSP_Vector_VRSQ;
	RSP_Vector[53] = RSP_Vector_VRSQL;
	RSP_Vector[54] = RSP_Vector_VRSQH;
	RSP_Vector[55] = RSP_Vector_VNOOP;
	RSP_Vector[56] = rsp_UnknownOpcode;
	RSP_Vector[57] = rsp_UnknownOpcode;
	RSP_Vector[58] = rsp_UnknownOpcode;
	RSP_Vector[59] = rsp_UnknownOpcode;
	RSP_Vector[60] = rsp_UnknownOpcode;
	RSP_Vector[61] = rsp_UnknownOpcode;
	RSP_Vector[62] = rsp_UnknownOpcode;
	RSP_Vector[63] = rsp_UnknownOpcode;

	RSP_Lc2[ 0] = RSP_Opcode_LBV;
	RSP_Lc2[ 1] = RSP_Opcode_LSV;
	RSP_Lc2[ 2] = RSP_Opcode_LLV;
	RSP_Lc2[ 3] = RSP_Opcode_LDV;
	RSP_Lc2[ 4] = RSP_Opcode_LQV;
	RSP_Lc2[ 5] = RSP_Opcode_LRV;
	RSP_Lc2[ 6] = RSP_Opcode_LPV;
	RSP_Lc2[ 7] = RSP_Opcode_LUV;
	RSP_Lc2[ 8] = RSP_Opcode_LHV;
	RSP_Lc2[ 9] = RSP_Opcode_LFV;
	RSP_Lc2[10] = rsp_UnknownOpcode;
	RSP_Lc2[11] = RSP_Opcode_LTV;
	RSP_Lc2[12] = rsp_UnknownOpcode;
	RSP_Lc2[13] = rsp_UnknownOpcode;
	RSP_Lc2[14] = rsp_UnknownOpcode;
	RSP_Lc2[15] = rsp_UnknownOpcode;
	RSP_Lc2[16] = rsp_UnknownOpcode;
	RSP_Lc2[17] = rsp_UnknownOpcode;
	RSP_Lc2[18] = rsp_UnknownOpcode;
	RSP_Lc2[19] = rsp_UnknownOpcode;
	RSP_Lc2[20] = rsp_UnknownOpcode;
	RSP_Lc2[21] = rsp_UnknownOpcode;
	RSP_Lc2[22] = rsp_UnknownOpcode;
	RSP_Lc2[23] = rsp_UnknownOpcode;
	RSP_Lc2[24] = rsp_UnknownOpcode;
	RSP_Lc2[25] = rsp_UnknownOpcode;
	RSP_Lc2[26] = rsp_UnknownOpcode;
	RSP_Lc2[27] = rsp_UnknownOpcode;
	RSP_Lc2[28] = rsp_UnknownOpcode;
	RSP_Lc2[29] = rsp_UnknownOpcode;
	RSP_Lc2[30] = rsp_UnknownOpcode;
	RSP_Lc2[31] = rsp_UnknownOpcode;

	RSP_Sc2[ 0] = RSP_Opcode_SBV;
	RSP_Sc2[ 1] = RSP_Opcode_SSV;
	RSP_Sc2[ 2] = RSP_Opcode_SLV;
	RSP_Sc2[ 3] = RSP_Opcode_SDV;
	RSP_Sc2[ 4] = RSP_Opcode_SQV;
	RSP_Sc2[ 5] = RSP_Opcode_SRV;
	RSP_Sc2[ 6] = RSP_Opcode_SPV;
	RSP_Sc2[ 7] = RSP_Opcode_SUV;
	RSP_Sc2[ 8] = RSP_Opcode_SHV;
	RSP_Sc2[ 9] = RSP_Opcode_SFV;
	RSP_Sc2[10] = RSP_Opcode_SWV;
	RSP_Sc2[11] = RSP_Opcode_STV;
	RSP_Sc2[12] = rsp_UnknownOpcode;
	RSP_Sc2[13] = rsp_UnknownOpcode;
	RSP_Sc2[14] = rsp_UnknownOpcode;
	RSP_Sc2[15] = rsp_UnknownOpcode;
	RSP_Sc2[16] = rsp_UnknownOpcode;
	RSP_Sc2[17] = rsp_UnknownOpcode;
	RSP_Sc2[18] = rsp_UnknownOpcode;
	RSP_Sc2[19] = rsp_UnknownOpcode;
	RSP_Sc2[20] = rsp_UnknownOpcode;
	RSP_Sc2[21] = rsp_UnknownOpcode;
	RSP_Sc2[22] = rsp_UnknownOpcode;
	RSP_Sc2[23] = rsp_UnknownOpcode;
	RSP_Sc2[24] = rsp_UnknownOpcode;
	RSP_Sc2[25] = rsp_UnknownOpcode;
	RSP_Sc2[26] = rsp_UnknownOpcode;
	RSP_Sc2[27] = rsp_UnknownOpcode;
	RSP_Sc2[28] = rsp_UnknownOpcode;
	RSP_Sc2[29] = rsp_UnknownOpcode;
	RSP_Sc2[30] = rsp_UnknownOpcode;
	RSP_Sc2[31] = rsp_UnknownOpcode;
	
	EleSpec[ 0].DW = 0;
	EleSpec[ 1].DW = 0;
	EleSpec[ 2].DW = 0;
	EleSpec[ 3].DW = 0;
	EleSpec[ 4].DW = 0;
	EleSpec[ 5].DW = 0;
	EleSpec[ 6].DW = 0;
	EleSpec[ 7].DW = 0;
	EleSpec[ 8].DW = 0;
	EleSpec[ 9].DW = 0;
	EleSpec[10].DW = 0;
	EleSpec[11].DW = 0;
	EleSpec[12].DW = 0;
	EleSpec[13].DW = 0;
	EleSpec[14].DW = 0;
	EleSpec[15].DW = 0;
	EleSpec[16].DW = 0x0001020304050607; /* None */
	EleSpec[17].DW = 0x0001020304050607; /* None */
	EleSpec[18].DW = 0x0000020204040606; /* 0q */
	EleSpec[19].DW = 0x0101030305050707; /* 1q */
	EleSpec[20].DW = 0x0000000004040404; /* 0h */
	EleSpec[21].DW = 0x0101010105050505; /* 1h */
	EleSpec[22].DW = 0x0202020206060606; /* 2h */
	EleSpec[23].DW = 0x0303030307070707; /* 3h */
	EleSpec[24].DW = 0x0000000000000000; /* 0 */
	EleSpec[25].DW = 0x0101010101010101; /* 1 */
	EleSpec[26].DW = 0x0202020202020202; /* 2 */
	EleSpec[27].DW = 0x0303030303030303; /* 3 */
	EleSpec[28].DW = 0x0404040404040404; /* 4 */
	EleSpec[29].DW = 0x0505050505050505; /* 5 */
	EleSpec[30].DW = 0x0606060606060606; /* 6 */
	EleSpec[31].DW = 0x0707070707070707; /* 7 */

	Indx[ 0].DW = 0;
	Indx[ 1].DW = 0;
	Indx[ 2].DW = 0;
	Indx[ 3].DW = 0;
	Indx[ 4].DW = 0;
	Indx[ 5].DW = 0;
	Indx[ 6].DW = 0;
	Indx[ 7].DW = 0;
	Indx[ 8].DW = 0;
	Indx[ 9].DW = 0;
	Indx[10].DW = 0;
	Indx[11].DW = 0;
	Indx[12].DW = 0;
	Indx[13].DW = 0;
	Indx[14].DW = 0;
	Indx[15].DW = 0;
	Indx[16].DW = 0x0001020304050607; /* None */
	Indx[17].DW = 0x0001020304050607; /* None */
	Indx[18].DW = 0x0103050700020406; /* 0q */
	Indx[19].DW = 0x0002040601030507; /* 1q */
	Indx[20].DW = 0x0102030506070004; /* 0h */
	Indx[21].DW = 0x0002030406070105; /* 1h */
	Indx[22].DW = 0x0001030405070206; /* 2h */
	Indx[23].DW = 0x0001020405060307; /* 3h */
	Indx[24].DW = 0x0102030405060700; /* 0 */
	Indx[25].DW = 0x0002030405060701; /* 1 */
	Indx[26].DW = 0x0001030405060702; /* 2 */
	Indx[27].DW = 0x0001020405060703; /* 3 */
	Indx[28].DW = 0x0001020305060704; /* 4 */
	Indx[29].DW = 0x0001020304060705; /* 5 */
	Indx[30].DW = 0x0001020304050706; /* 6 */
	Indx[31].DW = 0x0001020304050607; /* 7 */

	for (i = 16; i < 32; i ++) {
		int count;

		for (count = 0; count < 8; count ++) {
			Indx[i].B[count] = 7 - Indx[i].B[count];
			EleSpec[i].B[count] = 7 - EleSpec[i].B[count];
		}
		for (count = 0; count < 4; count ++) {
			BYTE Temp;
			
			Temp = Indx[i].B[count];
			Indx[i].B[count] = Indx[i].B[7 - count]; 
			Indx[i].B[7 - count] = Temp;
		}
	}


	PrgCount = RSPInfo.SP_PC_REG;
}
#endif
/******************************************************************
  Function: DoRspCycles
  Purpose:  This function is to allow the RSP to run in parrel with
            the r4300 switching control back to the r4300 once the
			function ends.
  input:    The number of cylces that is meant to be executed
  output:   The number of cycles that was executed. This value can
            be greater than the number of cycles that the RSP 
			should have performed.
			(this value is ignored if the RSP is stoped)
*******************************************************************/ 
__declspec(dllexport) DWORD DoRspCycles ( DWORD Cycles ) {
#if ( !(defined(DLIST) & defined(ALIST)) )
	DWORD CycleCount;
#endif

#if ( defined(DLIST) | defined(ALIST) )
	DWORD TaskType;

	TaskType = *( DWORD *)(RSPInfo.DMEM + 0xFC0);
#endif
	
#if defined (DLIST)
	if (TaskType==1) {
		if (RSPInfo.ProcessDlistList != NULL) {
			RSPInfo.ProcessDlistList();
		}
		*RSPInfo.SP_STATUS_REG |= (0x0203 );
		if ((*RSPInfo.SP_STATUS_REG & SP_STATUS_INTR_BREAK) != 0 ) {
			*RSPInfo.MI_INTR_REG |= R4300i_SP_Intr;
			RSPInfo.CheckInterrupts();
		}

		*RSPInfo.DPC_STATUS_REG &= ~2;
		return Cycles;
	}
#endif

#if defined(ALIST)
	if (TaskType==2) {
		if (RSPInfo.ProcessAlistList != NULL) {
			RSPInfo.ProcessAlistList();
		}
		*RSPInfo.SP_STATUS_REG |= (0x0203 );
		if ((*RSPInfo.SP_STATUS_REG & SP_STATUS_INTR_BREAK) != 0 ) {
			*RSPInfo.MI_INTR_REG |= R4300i_SP_Intr;
			RSPInfo.CheckInterrupts();
		}
		return Cycles;
	}
#endif
#if ( defined(DLIST) & defined(ALIST) )
	*RSPInfo.SP_STATUS_REG |= (0x0203 );
	if ((*RSPInfo.SP_STATUS_REG & SP_STATUS_INTR_BREAK) != 0 ) {
		*RSPInfo.MI_INTR_REG |= R4300i_SP_Intr;
		RSPInfo.CheckInterrupts();
	}
	return Cycles;
#endif

#if ( !(defined(DLIST) & defined(ALIST)) )
	RSP_Running = TRUE;
	Enable_RSP_Commands_Window();
	CycleCount = 0;
	while(RSP_Running) {
		if (NoOfBpoints != 0) {
			if (CheckForRSPBPoint(*PrgCount)) {
				if (InRSPCommandsWindow) {
					Enter_RSP_Commands_Window();
					if (Stepping_Commands) {
						DisplayError ( "Encounted a R4300i Breakpoint" );
					} else {
						DisplayError ( "Encounted a R4300i Breakpoint\n\nNow Stepping" );
						SetRSPCommandViewto( *PrgCount );
						SetRSPCommandToStepping();
					}
				} else {
					DisplayError ( "Encounted a RSP Breakpoint\n\nEntering Command Window" );
					Enter_RSP_Commands_Window();
				}
			}
		}
		
		if (Stepping_Commands) {
			WaitingForStep = TRUE;
			SetRSPCommandViewto( *PrgCount );
			UpdateRSPRegistersScreen();
			while ( WaitingForStep == TRUE ){ 
				Sleep(20);						
				if (!Stepping_Commands) {
					WaitingForStep = FALSE;
				}
			}
		}

		RSP_LW_IMEM(*PrgCount, &RSPOpC.Hex);
		((void (*)()) RSP_Opcode[ RSPOpC.op ])();

		switch (RSP_NextInstruction) {
		case NORMAL: 
			*PrgCount = (*PrgCount + 4) & 0xFFC; 
			break;
		case DELAY_SLOT:
			RSP_NextInstruction = JUMP;
			*PrgCount = (*PrgCount + 4) & 0xFFC; 
			break;
		case JUMP:
			RSP_NextInstruction = NORMAL;
			*PrgCount  = RSP_JumpTo;
			break;
		}
		/*CycleCount += 1;
		if (CycleCount > Cycles) {
			RSP_Running = FALSE;
			return CycleCount;
		}*/
	}
	*PrgCount -= 4;
	//Disable_RSP_Commands_Window();
	return Cycles;
#endif
}

