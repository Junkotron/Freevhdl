#include "dasm.hpp"

Disassembler::OpcodeProfile Disassembler::get_profile(uint8_t op) {
    switch (op) {
        // --- GRUPP 1: ADC / SBC / LDA / STA / CMP / ORA / AND / EOR ---
        // Immediate (#$XX)
        case 0x09: return { "ORA #$", 2, 1, -1, 2 }; case 0x29: return { "AND #$", 2, 1, -1, 2 };
        case 0x49: return { "EOR #$", 2, 1, -1, 2 }; case 0x69: return { "ADC #$", 2, 1, -1, 2 };
        case 0xA9: return { "LDA #$", 2, 1, -1, 2 }; case 0xC9: return { "CMP #$", 2, 1, -1, 2 };
        case 0xE9: return { "SBC #$", 2, 1, -1, 2 };
        
        // Zero Page ($XX)
        case 0x05: return { "ORA $",  3, 1, -1, 2 }; case 0x25: return { "AND $",  3, 1, -1, 2 };
        case 0x45: return { "EOR $",  3, 1, -1, 2 }; case 0x65: return { "ADC $",  3, 1, -1, 2 };
        case 0x85: return { "STA $",  3, 1, -1, 2 }; case 0xA5: return { "LDA $",  3, 1, -1, 2 };
        case 0xC5: return { "CMP $",  3, 1, -1, 2 }; case 0xE5: return { "SBC $",  3, 1, -1, 2 };

        // Absolute ($XXXX)
        case 0x0D: return { "ORA $",  4, 1,  2, 3 }; case 0x2D: return { "AND $",  4, 1,  2, 3 };
        case 0x4D: return { "EOR $",  4, 1,  2, 3 }; case 0x6D: return { "ADC $",  4, 1,  2, 3 };
        case 0x8D: return { "STA $",  4, 1,  2, 3 }; case 0xAD: return { "LDA $",  4, 1,  2, 3 };
        case 0xCD: return { "CMP $",  4, 1,  2, 3 }; case 0xED: return { "SBC $",  4, 1,  2, 3 };

        // Zero Page Indexed (,X / ,Y)
        case 0x15: return { "ORA $",  4, 1, -1, 2 }; case 0x35: return { "AND $",  4, 1, -1, 2 }; 
        case 0x55: return { "EOR $",  4, 1, -1, 2 }; case 0x75: return { "ADC $",  4, 1, -1, 2 };
        case 0x95: return { "STA $",  4, 1, -1, 2 }; case 0xB5: return { "LDA $",  4, 1, -1, 2 };
        case 0xD5: return { "CMP $",  4, 1, -1, 2 }; case 0xF5: return { "SBC $",  4, 1, -1, 2 };
        case 0x96: return { "STX $",  4, 1, -1, 2 }; case 0xB6: return { "LDX $",  4, 1, -1, 2 };

        // Absolute Indexed (,X / ,Y)
        case 0x1D: return { "ORA $",  4, 1,  2, 3 }; case 0x3D: return { "AND $",  4, 1,  2, 3 };
        case 0x5D: return { "EOR $",  4, 1,  2, 3 }; case 0x7D: return { "ADC $",  4, 1,  2, 3 };
        case 0x9D: return { "STA $",  5, 1,  2, 3 }; case 0xBD: return { "LDA $",  4, 1,  2, 3 };
        case 0xDD: return { "CMP $",  4, 1,  2, 3 }; case 0xFD: return { "SBC $",  4, 1,  2, 3 };
        case 0x19: return { "ORA $",  4, 1,  2, 3 }; case 0x39: return { "AND $",  4, 1,  2, 3 };
        case 0x59: return { "EOR $",  4, 1,  2, 3 }; case 0x79: return { "ADC $",  4, 1,  2, 3 };
        case 0x99: return { "STA $",  5, 1,  2, 3 }; case 0xB9: return { "LDA $",  4, 1,  2, 3 };
        case 0xD9: return { "CMP $",  4, 1,  2, 3 }; case 0xF9: return { "SBC $",  4, 1,  2, 3 };

        // Indexed Indirect (($XX,X))
        case 0x01: return { "ORA ($", 6, 1, -1, 2 }; case 0x21: return { "AND ($", 6, 1, -1, 2 };
        case 0x41: return { "EOR ($", 6, 1, -1, 2 }; case 0x61: return { "ADC ($", 6, 1, -1, 2 };
        case 0x81: return { "STA ($", 6, 1, -1, 2 }; case 0xA1: return { "LDA ($", 6, 1, -1, 2 };
        case 0xC1: return { "CMP ($", 6, 1, -1, 2 }; case 0xE1: return { "SBC ($", 6, 1, -1, 2 };

        // Indirect Indexed (($XX),Y)
        case 0x11: return { "ORA ($", 5, 1, -1, 2 }; case 0x31: return { "AND ($", 5, 1, -1, 2 };
        case 0x51: return { "EOR ($", 5, 1, -1, 2 }; case 0x71: return { "ADC ($", 5, 1, -1, 2 };
        case 0x91: return { "STA ($", 6, 1, -1, 2 }; case 0xB1: return { "LDA ($", 5, 1, -1, 2 };
        case 0xD1: return { "CMP ($", 5, 1, -1, 2 }; case 0xF1: return { "SBC ($", 5, 1, -1, 2 };

        // --- GRUPP 2: ASL / LSR / ROL / ROR / INC / DEC / LDX / STX / BIT / CPX / CPY ---
        case 0xA2: return { "LDX #$", 2, 1, -1, 2 }; case 0xAE: return { "LDX $",  4, 1,  2, 3 };
        case 0xA6: return { "LDX $",  3, 1, -1, 2 }; case 0xBE: return { "LDX $",  4, 1,  2, 3 };
        case 0x8E: return { "STX $",  4, 1,  2, 3 }; case 0x86: return { "STX $",  3, 1, -1, 2 };
        case 0xEE: return { "INC $",  6, 1,  2, 3 }; case 0xE6: return { "INC $",  5, 1, -1, 2 };
        case 0xFE: return { "INC $",  7, 1,  2, 3 }; case 0xF6: return { "INC $",  6, 1, -1, 2 };
        case 0xCE: return { "DEC $",  6, 1,  2, 3 }; case 0xC6: return { "DEC $",  5, 1, -1, 2 };
        case 0xDE: return { "DEC $",  7, 1,  2, 3 }; case 0xD6: return { "DEC $",  6, 1, -1, 2 };
        case 0xE0: return { "CPX #$", 2, 1, -1, 2 }; case 0xEC: return { "CPX $",  4, 1,  2, 3 };
        case 0xE4: return { "CPX $",  3, 1, -1, 2 };
        case 0xC0: return { "CPY #$", 2, 1, -1, 2 }; case 0xCC: return { "CPY $",  4, 1,  2, 3 };
        case 0xC4: return { "CPY $",  3, 1, -1, 2 };
        case 0x24: return { "BIT $",  3, 1, -1, 2 }; case 0x2C: return { "BIT $",  4, 1,  2, 3 };

        // Skift och Roterationer
        case 0x0E: return { "ASL $",  6, 1,  2, 3 }; case 0x06: return { "ASL $",  5, 1, -1, 2 };
        case 0x1E: return { "ASL $",  7, 1,  2, 3 }; case 0x16: return { "ASL $",  6, 1, -1, 2 };
        case 0x4E: return { "LSR $",  6, 1,  2, 3 }; case 0x46: return { "LSR $",  5, 1, -1, 2 };
        case 0x5E: return { "LSR $",  7, 1,  2, 3 }; case 0x56: return { "LSR $",  6, 1, -1, 2 };
        case 0x2E: return { "ROL $",  6, 1,  2, 3 }; case 0x26: return { "ROL $",  5, 1, -1, 2 };
        case 0x3E: return { "ROL $",  7, 1,  2, 3 }; case 0x36: return { "ROL $",  6, 1, -1, 2 };
        case 0x6E: return { "ROR $",  6, 1,  2, 3 }; case 0x66: return { "ROR $",  5, 1, -1, 2 };
        case 0x7E: return { "ROR $",  7, 1,  2, 3 }; case 0x76: return { "ROR $",  6, 1, -1, 2 };

        case 0x0A: return { "ASL A",  2, -1, -1, 1 }; case 0x4A: return { "LSR A",  2, -1, -1, 1 };
        case 0x2A: return { "ROL A",  2, -1, -1, 1 }; case 0x6A: return { "ROR A",  2, -1, -1, 1 };

        // --- GRUPP 3: JMP / JSR / RTS / BRANCHES / FLAGS / REGISTERS ---
        case 0x20: return { "JSR $",  6, 1,  5, 3 }; 
        case 0x60: return { "RTS",    6, -1, -1, 1 }; 
        case 0x4C: return { "JMP $",  3, 1,  2, 3 }; case 0x6C: return { "JMP ($", 5, 1,  2, 3 };
        case 0x40: return { "RTI",    6, -1, -1, 1 };

        // Villkorliga grenar
        case 0x10: return { "BPL *+", 2, 1, -1, 2 }; case 0x30: return { "BMI *+", 2, 1, -1, 2 };
        case 0x50: return { "BVC *+", 2, 1, -1, 2 }; case 0x70: return { "BVS *+", 2, 1, -1, 2 };
        case 0x90: return { "BCC *+", 2, 1, -1, 2 }; case 0xB0: return { "BCS *+", 2, 1, -1, 2 };
        case 0xD0: return { "BNE *+", 2, 1, -1, 2 }; case 0xF0: return { "BEQ *+", 2, 1, -1, 2 };

        // Stack och registerflaggor
        case 0x48: return { "PHA",    3, -1, -1, 1 }; case 0x68: return { "PLA",    4, -1, -1, 1 };
        case 0x08: return { "PHP",    3, -1, -1, 1 }; case 0x28: return { "PLP",    4, -1, -1, 1 };

        case 0xE8: return { "INX",    2, -1, -1, 1 }; case 0xCA: return { "DEX",    2, -1, -1, 1 };
        case 0xC8: return { "INY",    2, -1, -1, 1 }; case 0x88: return { "DEY",    2, -1, -1, 1 };
        case 0xAA: return { "TAX",    2, -1, -1, 1 }; case 0x8A: return { "TXA",    2, -1, -1, 1 };
        case 0xA8: return { "TAY",    2, -1, -1, 1 }; case 0x98: return { "TYA",    2, -1, -1, 1 };
        case 0xBA: return { "TSX",    2, -1, -1, 1 }; case 0x9A: return { "TXS",    2, -1, -1, 1 };
        case 0x18: return { "CLC",    2, -1, -1, 1 }; case 0x38: return { "SEC",    2, -1, -1, 1 };
        case 0x58: return { "CLI",    2, -1, -1, 1 }; case 0x78: return { "SEI",    2, -1, -1, 1 };
        case 0xB8: return { "CLV",    2, -1, -1, 1 }; case 0xD8: return { "CLD",    2, -1, -1, 1 };
        case 0xF8: return { "SED",    2, -1, -1, 1 };
        case 0xEA: return { "NOP",    2, -1, -1, 1 };

        default:   return { "NOP (Illeg) $", 2, -1, -1, 1 };
    }
}
