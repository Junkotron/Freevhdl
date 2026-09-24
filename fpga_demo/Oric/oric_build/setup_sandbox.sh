#!/bin/bash
# =================================================================
# 👑 Köpings Mekaniska Verkstad – Tidseffektiv Sandbox-Injektor
# =================================================================

NIKIIV_DIR="./nikiiv_src"
MISTER_REPO="https://github.com/nikiiv/Oric_MiSTer"
TEMP_DIR="./Oric_miSTer_temp"

# 1. Skapa den slutgiltiga källkodsmappen om den saknas
if [ ! -d "$NIKIIV_DIR" ]; then
    mkdir -p "$NIKIIV_DIR"
fi

# 2. EFFEKTIVITETS-VAKTEN: Om den centrala filen redan finns, avbryt omedelbart!
if [ -f "$NIKIIV_DIR/T65.vhd" ]; then
    echo "✅ Källkodsfiler hittades redan i $NIKIIV_DIR. Hoppar över nätverks-hämtning."
fi

# 3. Om filerna saknas, men vi har en gammal lokal temp-klon liggande, använd den!
if [ -d "$TEMP_DIR" ]; then
    echo "📂 Hittade en befintlig lokal utcheckning i $TEMP_DIR. Återanvänder denna!"
else
    echo "📦 Ingen lokal källkod hittad. Gör en blixtsnabb Git-clone..."
    git clone --depth 1 "$MISTER_REPO" "$TEMP_DIR"
fi

# 4. Trygg och snabb filskyffling från den lokala kopian
if [ -d "$TEMP_DIR" ]; then
    echo "🚚 Skyfflar över hårdvarufilerna till sandlådan..."
    
    cp "$TEMP_DIR"/sys/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/sys/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/sys/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/fpga/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/fpga/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/fpga/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/T65/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/T65/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/T65/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/rom/*.vhdl "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/rom/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/rom/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/rom/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/apple2_disk/*.vhd "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/apple2_disk/*.v    "$NIKIIV_DIR/" 2>/dev/null || true
    cp "$TEMP_DIR"/rtl/apple2_disk/*.sv   "$NIKIIV_DIR/" 2>/dev/null || true
    
    # 💡 Här raderar vi ingenting! Mappen ligger kvar tills du kör distclean.
    echo "✨ Sandlådan uppdaterad och helt redo!"
else
    echo "❌ FEL: Hittade eller kunde inte skapa källkodsdepån."
    exit 1
fi
