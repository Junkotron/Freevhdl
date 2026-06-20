
# 1. Skapa en tom ordlista för signalerna
signals = {}
current_time = "0"

# 2. Öppna och läs filen rad för rad
with open("filtered.vcd", "r") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue

        # Parsa definitioner av signaler
        if line.startswith("$var"):
            parts = line.split()
            if len(parts) >= 5:
                # Hittar symbol-ID (t.ex. !) och namnet (t.ex. addr_bus)
                symbol_id = parts[3]
                full_name = parts[4]
                base_name = full_name.split('[')[0]
                signals[symbol_id] = base_name
            continue

        # Uppdatera simuleringstiden
        if line.startswith('#'):
            current_time = line[1:]
            continue

        # Hantera flertrådiga bussar (t.ex. b0010 !)
        if line.startswith('b'):
            parts = line.split()
            if len(parts) == 2:
                val_str = parts[0][1:] # Ta bort 'b' i början
                symbol_id = parts[1]
                if symbol_id in signals:
                    name = signals[symbol_id]
                    try:
                        hex_val = f"0x{int(val_str, 2):X}"
                        print(f"Tid: {current_time:<8} | {name:<12} -> {hex_val}")
                    except ValueError:
                        print(f"Tid: {current_time:<8} | {name:<12} -> {val_str}")
            continue

        # Hantera enskilda bitar (t.ex. 1! eller 0!)
        if len(line) >= 2 and line[0] in ('0', '1', 'x', 'z', 'X', 'Z'):
            val_str = line[0]
            symbol_id = line[1:]
            if symbol_id in signals:
                name = signals[symbol_id]
                print(f"Tid: {current_time:<8} | {name:<12} -> {val_str}")

print("--- Skriptet har kört klart! ---")

