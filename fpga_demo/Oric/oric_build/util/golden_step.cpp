#include "golden_6502.hpp"
#include <iostream>

bool Golden6502::step() {

    // 💡 DETTA ÄR DET MAGISKA TRICKET!
    // Vi skapar en lokal referens till hela CPU-objektet självt.
    // Det gör att alla "mem[...]" i hela denna fil automatiskt pekar på operatorn i hpp-filen!
    Golden6502& mem = *this; 

    
    uint16_t inst_pc = pc;
    uint8_t opcode = mem[pc];
    step_counter++;

    // Hämta bytes direkt för enkel avkodning
    uint8_t  imm   = mem.raw_mem[pc+1];
    uint16_t abs_base = mem.raw_mem[pc+1] | (mem.raw_mem[pc + 2] << 8);

    switch (opcode) {
        // --- SYSTEM, FLAGGER & NOP ---
        case 0x78: pc += 1; p |= 0x04;  print_log_line(inst_pc, "SEI", 1, 0, false); break;
        case 0x58: pc += 1; p &= ~0x04; print_log_line(inst_pc, "CLI", 1, 0, false); break;
        case 0xD8: pc += 1; p &= ~0x08; print_log_line(inst_pc, "CLD", 1, 0, false); break;
        case 0x38: pc += 1; p |= 0x01;  print_log_line(inst_pc, "SEC", 1, 0, false); break; // Set Carry
        case 0x18: pc += 1; p &= ~0x01; print_log_line(inst_pc, "CLC", 1, 0, false); break; // Clear Carry
        case 0xEA: pc += 1;             print_log_line(inst_pc, "NOP", 1, 0, false); break;

        // --- REGISTERÖVERFÖRINGAR & STACK ---
        case 0x9A: pc += 1; sp = x;      print_log_line(inst_pc, "TXS", 1, 0, false); break;
        case 0xAA: pc += 1; x = a;  update_nz(x); print_log_line(inst_pc, "TAX", 1, 0, false); break;
        case 0x8A: pc += 1; a = x;  update_nz(a); print_log_line(inst_pc, "TXA", 1, 0, false); break;
        case 0xA8: pc += 1; y = a;  update_nz(y); print_log_line(inst_pc, "TAY", 1, 0, false); break;
        case 0x98: pc += 1; a = y;  update_nz(a); print_log_line(inst_pc, "TYA", 1, 0, false); break;
        case 0x48: pc += 1; push_stack(a);        print_log_line(inst_pc, "PHA", 1, 0, false); break;
        case 0x68: pc += 1; a = pop_stack(); update_nz(a); print_log_line(inst_pc, "PLA", 1, 0, false); break;

        // --- LDA (Load Accumulator) ---
        case 0xA9: pc += 2; a = imm; update_nz(a); print_log_line(inst_pc, "LDA", 2, 0, true);  break;
        case 0xA5: pc += 2; a = mem[imm]; update_nz(a); print_log_line(inst_pc, "LDA ZP", 2, imm, false); break;
        case 0xB5: pc += 2; a = mem[(imm + x) & 0xFF]; update_nz(a); print_log_line(inst_pc, "LDA ZP,X", 2, (imm+x)&0xFF, false); break;
        case 0xAD: pc += 3; a = mem[abs_base]; update_nz(a); print_log_line(inst_pc, "LDA", 3, abs_base, false); break;
        case 0xBD: { // 💡 HÄR ÄR VÅR NYA TRÄFF: LDA Absolute,X
            pc += 3; 
            uint16_t target = abs_base + x;
            a = mem[target]; 
            update_nz(a); 
            print_log_line(inst_pc, "LDA", 3, target, false); 
            break; 
        }
        case 0xB9: pc += 3; a = mem[abs_base + y]; update_nz(a); print_log_line(inst_pc, "LDA Absolute,Y", 3, abs_base+y, false); break;

        // --- LDX / LDY (Load X & Y) ---
        case 0xA2: pc += 2; x = imm; update_nz(x); print_log_line(inst_pc, "LDX", 2, 0, true);  break;
        case 0xA6: pc += 2; x = mem[imm]; update_nz(x); print_log_line(inst_pc, "LDX ZP", 2, imm, false); break;
        case 0xAE: pc += 3; x = mem[abs_base]; update_nz(x); print_log_line(inst_pc, "LDX Absolute", 3, abs_base, false); break;
        case 0xA0: pc += 2; y = imm; update_nz(y); print_log_line(inst_pc, "LDY", 2, 0, true);  break;
        case 0xA4: pc += 2; y = mem[imm]; update_nz(y); print_log_line(inst_pc, "LDY ZP", 2, imm, false); break;
        case 0xAC: pc += 3; y = mem[abs_base]; update_nz(y); print_log_line(inst_pc, "LDY Absolute", 3, abs_base, false); break;

        // --- STA / STX / STY (Store) ---
        case 0x8D: pc += 3; mem[abs_base] = a; print_log_line(inst_pc, "STA", 3, abs_base, false); break;
        case 0x9D: pc += 3; mem[abs_base + x] = a; print_log_line(inst_pc, "STA Absolute,X", 3, abs_base+x, false); break;
        case 0x99: pc += 3; mem[abs_base + y] = a; print_log_line(inst_pc, "STA Absolute,Y", 3, abs_base+y, false); break;
        case 0x85: pc += 2; mem[imm] = a; print_log_line(inst_pc, "STA ZP", 2, imm, false); break;
        case 0x95: pc += 2; mem[(imm + x) & 0xFF] = a; print_log_line(inst_pc, "STA ZP,X", 2, (imm+x)&0xFF, false); break;
        case 0x8E: pc += 3; mem[abs_base] = x; print_log_line(inst_pc, "STX", 3, abs_base, false); break;
        case 0x86: pc += 2; mem[imm] = x; print_log_line(inst_pc, "STX ZP", 2, imm, false); break;
        case 0x8C: pc += 3; mem[abs_base] = y; print_log_line(inst_pc, "STY", 3, abs_base, false); break;
        case 0x84: pc += 2; mem[imm] = y; print_log_line(inst_pc, "STY ZP", 2, imm, false); break;

        // --- INC / DEC / MATH ---
        case 0xE8: pc += 1; x++; update_nz(x); print_log_line(inst_pc, "INX", 1, 0, false); break;
        case 0xC8: pc += 1; y++; update_nz(y); print_log_line(inst_pc, "INY", 1, 0, false); break;
        case 0xCA: pc += 1; x--; update_nz(x); print_log_line(inst_pc, "DEX", 1, 0, false); break;
        case 0x88: pc += 1; y--; update_nz(y); print_log_line(inst_pc, "DEY", 1, 0, false); break;

        // --- CMP / CPX / CPY (Compare) ---
        case 0xC9: { // CMP Immediate
            pc += 2;
            uint16_t result = a - imm;
            p &= ~0x01; if (a >= imm) p |= 0x01; // Sätt Carry om A >= Imm
            update_nz(result & 0xFF);
            print_log_line(inst_pc, "CMP Immediate", 2, 0, true);
            break;
        }
        case 0xE0: { // CPX Immediate
            pc += 2;
            uint16_t result = x - imm;
            p &= ~0x01; if (x >= imm) p |= 0x01;
            update_nz(result & 0xFF);
            print_log_line(inst_pc, "CPX Immediate", 2, 0, true);
            break;
        }

        // --- HOUPP, SUBRUTINER & RETUR ---
        case 0x4C: pc = abs_base; print_log_line(inst_pc, "JMP", 3, abs_base, false); break;
        case 0x20: { // JSR
            uint16_t return_pc = pc + 2;
            push_stack((return_pc >> 8) & 0xFF);
            push_stack(return_pc & 0xFF);
            pc = abs_base;
            print_log_line(inst_pc, "JSR", 3, abs_base, false);
            break;
        }
        case 0x60: { // RTS
            uint16_t low = pop_stack(), high = pop_stack();
            pc = ((high << 8) | low) + 1;
            print_log_line(inst_pc, "RTS", 1, 0, false);
            break;
        }

        // --- VILLKORLIGA HOPP (Branches) ---
        case 0xD0: { // BNE
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x02) == 0) pc = target_addr;
            print_log_line(inst_pc, "BNE", 2, target_addr, false);
            break;
        }
        case 0xF0: { // BEQ
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x02) != 0) pc = target_addr;
            print_log_line(inst_pc, "BEQ", 2, target_addr, false);
            break;
        }
        case 0x90: { // BCC
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x01) == 0) pc = target_addr;
            print_log_line(inst_pc, "BCC", 2, target_addr, false);
            break;
        }
        case 0xB0: { // BCS
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x01) != 0) pc = target_addr;
            print_log_line(inst_pc, "BCS", 2, target_addr, false);
            break;
        }
        case 0x10: { // 💡 BPL (Branch on Plus) - Korrigerad måladressberäkning!
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x80) == 0) pc = target_addr;
            print_log_line(inst_pc, "BPL", 2, target_addr, false);
            break;
        }
        case 0x30: { // 💡 BMI (Branch on Minus) - Korrigerad!
            uint16_t target_addr = pc + 2 + static_cast<int8_t>(imm);
            pc += 2;
            if ((p & 0x80) != 0) pc = target_addr;
            print_log_line(inst_pc, "BMI", 2, target_addr, false);
            break;
        }
        // 💡 NYA VERKSTADSTRÄFFAR: INC och DEC för Zero-Page (ZP)
        case 0xE6: { // INC ZP
            pc += 2;
            mem[imm] = (mem[imm] + 1) & 0xFF;
            update_nz(mem[imm]);
            print_log_line(inst_pc, "INC ZP", 2, imm, false);
            break;
        }
        case 0xC6: { // DEC ZP
            pc += 2;
            mem[imm] = (mem[imm] - 1) & 0xFF;
            update_nz(mem[imm]);
            print_log_line(inst_pc, "DEC ZP", 2, imm, false);
            break;
        }

        // 💡 NYA TRAFFAR FÖR MINNESTEST-LOOPAR:
        case 0xC5: { // CMP ZP
            pc += 2;
            uint8_t val = mem[imm];
            uint16_t result = a - val;
            p &= ~0x01; if (a >= val) p |= 0x01; // Sätt Carry om A >= ZP-värdet
            update_nz(result & 0xFF);
            print_log_line(inst_pc, "CMP ZP", 2, imm, false);
            break;
        }
        case 0xCD: { // CMP Absolute
            pc += 3;
            uint8_t val = mem[abs_base];
            uint16_t result = a - val;
            p &= ~0x01; if (a >= val) p |= 0x01;
            update_nz(result & 0xFF);
            print_log_line(inst_pc, "CMP Absolute", 3, abs_base, false);
            break;
        }
        case 0xD1: { // CMP (Indirect),Y
            pc += 2;
            // Slå upp den 16-bitars basadressen från Zero-Page
            uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y;
            uint8_t val = mem[target];
            uint16_t result = a - val;
            p &= ~0x01; if (a >= val) p |= 0x01;
            update_nz(result & 0xFF);
            print_log_line(inst_pc, "CMP (Indirect),Y", 2, target, false);
            break;
        }

        // 💡 NY TRÄFF FÖR MINNESVERIFIERINGEN:
        case 0xB1: { // LDA (Indirect),Y
            pc += 2;
            uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y;
            a = mem[target];
            update_nz(a);
            print_log_line(inst_pc, "LDA (Indirect),Y", 2, target, false);
            break;
        }

        // 💡 NY TRÄFF FÖR MINNESSKRIVNINGEN:
        case 0x91: { // STA (Indirect),Y
            pc += 2;
            uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y;
            mem[target] = a;
            print_log_line(inst_pc, "STA (Indirect),Y", 2, target, false);
            break;
        }


        // --- 💡 NYA FÄRSKA VERKSTADS-CASES ATT MIPPLE IN ---

        case 0x05: { // ORA ZP (Bitvis OR med Zero Page)
            pc += 2;
            a |= mem[imm];
            update_nz(a);
            print_log_line(inst_pc, "ORA ZP", 2, imm, false);
            break;
        }

        case 0x09: { // ORA Immediate (Bitvis OR med konstant)
            pc += 2;
            a |= imm;
            update_nz(a);
            print_log_line(inst_pc, "ORA Immediate", 2, 0, true);
            break;
        }

        case 0x25: { // AND ZP (Bitvis AND med Zero Page)
            pc += 2;
            a &= mem[imm];
            update_nz(a);
            print_log_line(inst_pc, "AND ZP", 2, imm, false);
            break;
        }

        case 0x29: { // AND Immediate (Bitvis AND med konstant)
            pc += 2;
            a &= imm;
            update_nz(a);
            print_log_line(inst_pc, "AND Immediate", 2, 0, true);
            break;
        }

        case 0x4A: { // LSR A (Logical Shift Right på Ackumulatorn)
            pc += 1;
            p &= ~0x01;              // Rensa Carry först
            if (a & 0x01) p |= 0x01; // Sätt Carry om lägsta biten var 1
            a >>= 1;
            update_nz(a);
            print_log_line(inst_pc, "LSR A", 1, 0, false);
            break;
        }

        case 0x46: { // LSR ZP (Logical Shift Right på ett minne i Zero Page)
            pc += 2;
            uint8_t val = mem[imm];
            p &= ~0x01;
            if (val & 0x01) p |= 0x01;
            val >>= 1;
            mem[imm] = val;
            update_nz(val);
            print_log_line(inst_pc, "LSR ZP", 2, imm, false);
            break;
        }

        case 0x06: { // ASL ZP (Arithmetic Shift Left på Zero Page)
            pc += 2;
            uint8_t val = mem[imm];
            p &= ~0x01;
            if (val & 0x80) p |= 0x01; // Sätt Carry om högsta biten var 1
            val <<= 1;
            mem[imm] = val;
            update_nz(val);
            print_log_line(inst_pc, "ASL ZP", 2, imm, false);
            break;
        }

        case 0x0A: { // ASL A (Arithmetic Shift Left på Ackumulatorn)
            pc += 1;
            p &= ~0x01;
            if (a & 0x80) p |= 0x01;
            a <<= 1;
            update_nz(a);
            print_log_line(inst_pc, "ASL A", 1, 0, false);
            break;
        }

        case 0x15: { // ORA ZP,X (Bitvis OR med Zero Page indexerat med X)
            pc += 2;
            uint8_t target = (imm + x) & 0xFF;
            a |= mem[target];
            update_nz(a);
            print_log_line(inst_pc, "ORA ZP,X", 2, target, false);
            break;
        }

        case 0x1d: { // ORA Absolute,X (Bitvis OR med Absolute indexerat med X)
            pc += 3;
            uint16_t target = abs_base + x;
            a |= mem[target];
            update_nz(a);
            print_log_line(inst_pc, "ORA Absolute,X", 3, target, false);
            break;
        }

        case 0x3d: { // AND Absolute,X (Bitvis AND med Absolute indexerat med X)
            pc += 3;
            uint16_t target = abs_base + x;
            a &= mem[target];
            update_nz(a);
            print_log_line(inst_pc, "AND Absolute,X", 3, target, false);
            break;
        }

        case 0x50: { // BVC (Branch on Overflow Clear)
            pc += 2;
            if ((p & 0x40) == 0) pc += static_cast<int8_t>(imm);
            print_log_line(inst_pc, "BVC", 2, pc, false);
            break;
        }

        // --- 💡 MER MATEMATIK TILL VERKSTADEN ---

        case 0x69: { // ADC Immediate (Add with Carry)
            pc += 2;
            uint16_t carry_in = (p & 0x01) ? 1 : 0;
            uint16_t sum = a + imm + carry_in;
            
            p &= ~0x01; // Rensa Carry
            if (sum > 0xFF) p |= 0x01; // Sätt Carry om det rinner över
            
            a = sum & 0xFF;
            update_nz(a);
            print_log_line(inst_pc, "ADC Immediate", 2, 0, true);
            break;
        }

        case 0xE9: { // SBC Immediate (Subtract with Carry)
            pc += 2;
            // På 6502 är Carry inverterad vid subtraktion (1 = inget lån, 0 = lån)
            uint16_t borrow = (p & 0x01) ? 0 : 1;
            uint16_t diff = a - imm - borrow;
            
            p &= ~0x01; // Rensa Carry
            if (a >= (imm + borrow)) p |= 0x01; // Sätt Carry om inget lån krävdes
            
            a = diff & 0xFF;
            update_nz(a);
            print_log_line(inst_pc, "SBC Immediate", 2, 0, true);
            break;
        }
        // --- 💡 NYA SUBTRAKTIONS- & ADDITIONSVERKTYG FÖR ZERO PAGE ---

        case 0x65: { // ADC ZP (Add with Carry från Zero Page)
            pc += 2;
            uint8_t val = mem[imm];
            uint16_t carry_in = (p & 0x01) ? 1 : 0;
            uint16_t sum = a + val + carry_in;
            
            p &= ~0x01; // Rensa Carry
            if (sum > 0xFF) p |= 0x01; // Sätt Carry om det rinner över
            
            a = sum & 0xFF;
            update_nz(a);
            print_log_line(inst_pc, "ADC ZP", 2, imm, false);
            break;
        }

        case 0xE5: { // SBC ZP (Subtract with Carry från Zero Page)
            pc += 2;
            uint8_t val = mem[imm];
            uint16_t borrow = (p & 0x01) ? 0 : 1;
            uint16_t diff = a - val - borrow;
            
            p &= ~0x01; // Rensa Carry
            if (a >= (val + borrow)) p |= 0x01; // Sätt Carry om inget lån krävdes
            
            a = diff & 0xFF;
            update_nz(a);
            print_log_line(inst_pc, "SBC ZP", 2, imm, false);
            break;
        }
  
        // --- 💡 OS-KÄRNANS VERKTYG (PHP, PLP, BIT) ---

        case 0x08: { // PHP (Push Processor Status på stacken)
            pc += 1;
            // Bit 4 och 5 sätts alltid till 1 när P trycks på stacken via PHP
            push_stack(p | 0x30); 
            print_log_line(inst_pc, "PHP", 1, 0, false);
            break;
        }

        case 0x28: { // PLP (Pull Processor Status från stacken)
            pc += 1;
            // Hämta flaggorna men behåll bit 5 som 1 och strunta i bit 4 (Break)
            uint8_t pulled_p = pop_stack();
            p = (pulled_p & 0xEF) | 0x20; 
            print_log_line(inst_pc, "PLP", 1, 0, false);
            break;
        }

        case 0x24: { // BIT ZP (Bit Test Zero Page)
            pc += 2;
            uint8_t val = mem[imm];
            // Sätt N (bit 7) och V (bit 6) direkt från minnesvärdets bit 7 och 6
            p &= ~0xC2; // Rensa N, V, Z
            if (val & 0x80) p |= 0x80;
            if (val & 0x40) p |= 0x40;
            if ((a & val) == 0) p |= 0x02; // Z-flaggan sätts om AND blir noll
            print_log_line(inst_pc, "BIT ZP", 2, imm, false);
            break;
        }

        case 0x2C: { // BIT Absolute (Bit Test Absolute)
            pc += 3;
            uint8_t val = mem[abs_base];
            p &= ~0xC2; // Rensa N, V, Z
            if (val & 0x80) p |= 0x80;
            if (val & 0x40) p |= 0x40;
            if ((a & val) == 0) p |= 0x02;
            print_log_line(inst_pc, "BIT Absolute", 3, abs_base, false);
            break;
        }
	  
        // === 🛠️ BLOCK 1: ROTERINGAR, SKIFT & CPY ===

        case 0x0E: { // ASL Absolute
            pc += 3; uint8_t val = mem[abs_base];
            p &= ~0x01; if (val & 0x80) p |= 0x01;
            val <<= 1; mem[abs_base] = val; update_nz(val);
            print_log_line(inst_pc, "ASL Absolute", 3, abs_base, false); break;
        }
        case 0x4E: { // LSR Absolute
            pc += 3; uint8_t val = mem[abs_base];
            p &= ~0x01; if (val & 0x01) p |= 0x01;
            val >>= 1; mem[abs_base] = val; update_nz(val);
            print_log_line(inst_pc, "LSR Absolute", 3, abs_base, false); break;
        }
        case 0x2A: { // ROL A (Rotate Left Ackumulator via Carry)
            pc += 1; uint16_t carry_in = (p & 0x01) ? 1 : 0;
            p &= ~0x01; if (a & 0x80) p |= 0x01;
            a = (a << 1) | carry_in; update_nz(a);
            print_log_line(inst_pc, "ROL A", 1, 0, false); break;
        }
        case 0x26: { // ROL ZP
            pc += 2; uint16_t carry_in = (p & 0x01) ? 1 : 0;
            uint8_t val = mem[imm]; p &= ~0x01; if (val & 0x80) p |= 0x01;
            val = (val << 1) | carry_in; mem[imm] = val; update_nz(val);
            print_log_line(inst_pc, "ROL ZP", 2, imm, false); break;
        }
        case 0x6A: { // ROR A (Rotate Right Ackumulator via Carry)
            pc += 1; uint8_t carry_in = (p & 0x01) ? 0x80 : 0x00;
            p &= ~0x01; if (a & 0x01) p |= 0x01;
            a = (a >>= 1) | carry_in; update_nz(a);
            print_log_line(inst_pc, "ROR A", 1, 0, false); break;
        }
        case 0x66: { // ROR ZP
            pc += 2; uint8_t carry_in = (p & 0x01) ? 0x80 : 0x00;
            uint8_t val = mem[imm]; p &= ~0x01; if (val & 0x01) p |= 0x01;
            val = (val >>= 1) | carry_in; mem[imm] = val; update_nz(val);
            print_log_line(inst_pc, "ROR ZP", 2, imm, false); break;
        }
        case 0xC0: { // CPY Immediate (Jämför Y-register)
            pc += 2; uint16_t res = y - imm;
            p &= ~0x01; if (y >= imm) p |= 0x01; update_nz(res & 0xFF);
            print_log_line(inst_pc, "CPY Immediate", 2, 0, true); break;
        }
        case 0xC4: { // CPY ZP
            pc += 2; uint8_t val = mem[imm]; uint16_t res = y - val;
            p &= ~0x01; if (y >= val) p |= 0x01; update_nz(res & 0xFF);
            print_log_line(inst_pc, "CPY ZP", 2, imm, false); break;
        }
	  

        // === 🛠️ BLOCK 2: EOR, EXTRA MATEMATIK & MODERNA FLAGGOR ===

        case 0x2d: { // AND Absolute
            pc += 3; a &= mem[abs_base]; update_nz(a);
            print_log_line(inst_pc, "AND Absolute", 3, abs_base, false); break;
        }
        case 0x39: { // AND Absolute,Y
            pc += 3; a &= mem[abs_base + y]; update_nz(a);
            print_log_line(inst_pc, "AND Absolute,Y", 3, abs_base+y, false); break;
        }
        case 0x45: { // EOR ZP (Bitvis XOR)
            pc += 2; a ^= mem[imm]; update_nz(a);
            print_log_line(inst_pc, "EOR ZP", 2, imm, false); break;
        }
        case 0x49: { // EOR Immediate (Bitvis XOR)
            pc += 2; a ^= imm; update_nz(a);
            print_log_line(inst_pc, "EOR Immediate", 2, 0, true); break;
        }
        case 0x4d: { // EOR Absolute
            pc += 3; a ^= mem[abs_base]; update_nz(a);
            print_log_line(inst_pc, "EOR Absolute", 3, abs_base, false); break;
        }
        case 0x5d: { // EOR Absolute,X
            pc += 3; a ^= mem[abs_base + x]; update_nz(a);
            print_log_line(inst_pc, "EOR Absolute,X", 3, abs_base+x, false); break;
        }
        case 0x59: { // EOR Absolute,Y
            pc += 3; a ^= mem[abs_base + y]; update_nz(a);
            print_log_line(inst_pc, "EOR Absolute,Y", 3, abs_base+y, false); break;
        }
        case 0x70: { // BVS (Branch on Overflow Set)
            pc += 2; if ((p & 0x40) != 0) pc += static_cast<int8_t>(imm);
            print_log_line(inst_pc, "BVS", 2, pc, false); break;
        }
        case 0x7d: { // ADC Absolute,X
            pc += 3; uint16_t carry_in = (p & 0x01) ? 1 : 0;
            uint16_t sum = a + mem[abs_base + x] + carry_in;
            p &= ~0x01; if (sum > 0xFF) p |= 0x01;
            a = sum & 0xFF; update_nz(a);
            print_log_line(inst_pc, "ADC Absolute,X", 3, abs_base+x, false); break;
        }
        case 0x79: { // ADC Absolute,Y
            pc += 3; uint16_t carry_in = (p & 0x01) ? 1 : 0;
            uint16_t sum = a + mem[abs_base + y] + carry_in;
            p &= ~0x01; if (sum > 0xFF) p |= 0x01;
            a = sum & 0xFF; update_nz(a);
            print_log_line(inst_pc, "ADC Absolute,Y", 3, abs_base+y, false); break;
        }
        case 0xcc: { // CPY Absolute (Jämför Y-register)
            pc += 3; uint8_t val = mem[abs_base]; uint16_t res = y - val;
            p &= ~0x01; if (y >= val) p |= 0x01; update_nz(res & 0xFF);
            print_log_line(inst_pc, "CPY Absolute", 3, abs_base, false); break;
        }
        case 0xec: { // CPX Absolute (Jämför X-register)
            pc += 3; uint8_t val = mem[abs_base]; uint16_t res = x - val;
            p &= ~0x01; if (x >= val) p |= 0x01; update_nz(res & 0xFF);
            print_log_line(inst_pc, "CPX Absolute", 3, abs_base, false); break;
        }
        case 0xb4: { // LDY ZP,X
            pc += 2; y = mem[(imm + x) & 0xFF]; update_nz(y);
            print_log_line(inst_pc, "LDY ZP,X", 2, (imm+x)&0xFF, false); break;
        }
        case 0xbc: { // LDY Absolute,X
            pc += 3; y = mem[abs_base + x]; update_nz(y);
            print_log_line(inst_pc, "LDY Absolute,X", 3, abs_base+x, false); break;
        }
        case 0xf9: { // SBC Absolute,Y
            pc += 3; uint16_t borrow = (p & 0x01) ? 0 : 1;
            uint8_t val = mem[abs_base + y]; uint16_t diff = a - val - borrow;
            p &= ~0x01; if (a >= (val + borrow)) p |= 0x01;
            a = diff & 0xFF; update_nz(a);
            print_log_line(inst_pc, "SBC Absolute,Y", 3, abs_base+y, false); break;
        }
        case 0xb8: { // CLV (Clear Overflow Flag)
            pc += 1; p &= ~0x40;
            print_log_line(inst_pc, "CLV", 1, 0, false); break;
        }

        // === 🛠️ BLOCK 3: DE SISTA OFFICIELLA 6502-OPKODERNA ===

        case 0x01: { // ORA (Indirect,X)
            pc += 2; uint8_t target_zp = (imm + x) & 0xFF;
            uint16_t target = mem[target_zp] | (mem[(target_zp + 1) & 0xFF] << 8);
            a |= mem[target]; update_nz(a);
            print_log_line(inst_pc, "ORA (Indirect,X)", 2, target, false); break;
        }
        case 0x11: { // ORA (Indirect),Y
            pc += 2; uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y; a |= mem[target]; update_nz(a);
            print_log_line(inst_pc, "ORA (Indirect),Y", 2, target, false); break;
        }
        case 0x21: { // AND (Indirect,X)
            pc += 2; uint8_t target_zp = (imm + x) & 0xFF;
            uint16_t target = mem[target_zp] | (mem[(target_zp + 1) & 0xFF] << 8);
            a &= mem[target]; update_nz(a);
            print_log_line(inst_pc, "AND (Indirect,X)", 2, target, false); break;
        }
        case 0x31: { // AND (Indirect),Y
            pc += 2; uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y; a &= mem[target]; update_nz(a);
            print_log_line(inst_pc, "AND (Indirect),Y", 2, target, false); break;
        }
        case 0x41: { // EOR (Indirect,X)
            pc += 2; uint8_t target_zp = (imm + x) & 0xFF;
            uint16_t target = mem[target_zp] | (mem[(target_zp + 1) & 0xFF] << 8);
            a ^= mem[target]; update_nz(a);
            print_log_line(inst_pc, "EOR (Indirect,X)", 2, target, false); break;
        }
        case 0x51: { // EOR (Indirect),Y
            pc += 2; uint16_t zp_base = mem[imm] | (mem[(imm + 1) & 0xFF] << 8);
            uint16_t target = zp_base + y; a ^= mem[target]; update_nz(a);
            print_log_line(inst_pc, "EOR (Indirect),Y", 2, target, false); break;
        }
        case 0x40: { // RTI (Return from Interrupt)
            pc += 1; p = (pop_stack() & 0xEF) | 0x20; // Hämta statusregister
            uint16_t low = pop_stack(), high = pop_stack();
            pc = (high << 8) | low; // Återställ PC direkt från stacken
            print_log_line(inst_pc, "RTI", 1, 0, false); break;
        }
        case 0xCE: { // DEC Absolute (Minska ett minnesvärde direkt)
            pc += 3; mem[abs_base] = (mem[abs_base] - 1) & 0xFF; update_nz(mem[abs_base]);
            print_log_line(inst_pc, "DEC Absolute", 3, abs_base, false); break;
        }
        case 0xDE: { // DEC Absolute,X
            pc += 3; uint16_t target = abs_base + x;
            mem[target] = (mem[target] - 1) & 0xFF; update_nz(mem[target]);
            print_log_line(inst_pc, "DEC Absolute,X", 3, target, false); break;
        }
        case 0xEE: { // INC Absolute (Öka ett minnesvärde direkt)
            pc += 3; mem[abs_base] = (mem[abs_base] + 1) & 0xFF; update_nz(mem[abs_base]);
            print_log_line(inst_pc, "INC Absolute", 3, abs_base, false); break;
        }
        case 0xFE: { // INC Absolute,X
            pc += 3; uint16_t target = abs_base + x;
            mem[target] = (mem[target] + 1) & 0xFF; update_nz(mem[target]);
            print_log_line(inst_pc, "INC Absolute,X", 3, target, false); break;
        }
        case 0x6C: { // JMP Indirect - Den berömda JMP ($FFFF)
            pc += 3; // Slå upp den dolda adressen från minnespekaren
            uint16_t low = mem[abs_base];
            // Emulera den klassiska hårdvarubuggen på 6502 vid sidbrytningar
            uint16_t high_addr = (abs_base & 0xFF00) | ((abs_base + 1) & 0xFF);
            uint16_t high = mem[high_addr];
            pc = low | (high << 8);
            print_log_line(inst_pc, "JMP Indirect", 3, abs_base, false); break;
        }
	  
        case 0xBA: // 💡 TSX (Transfer Stack Pointer to X)
            pc += 1; x = sp; update_nz(x);
            print_log_line(inst_pc, "TSX", 1, 0, false); break;

        case 0xBE: { // 💡 LDX Absolute,Y
            pc += 3;
            uint16_t target = abs_base + y;
            x = mem[target];
            update_nz(x);
            print_log_line(inst_pc, "LDX Absolute,Y", 3, target, false);
            break;
        }


	    
	  // --- SYSTEMSTOPP ---
        case 0x00: print_log_line(inst_pc, "BRK", 1, 0, false); return false;

        default:
            print_log_line(inst_pc, "UNKNOWN_OP", 1, 0, false);
            std::cout << "⚠️ Okänd opkod $" << std::hex << (int)opcode << " vid PC: $" << inst_pc << std::endl;
            return false;
    }
    return true;
}

