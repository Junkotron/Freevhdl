import re

def print_collapsed_vcd_hierarchy(vcd_path):
    current_hierarchy = []
    # Ordlista för att samla array-index per unik sökväg och bit-suffix
    # Struktur: { ("top.inst_oricram.ram", "[7:0]"): [42882, 42883, ...] }
    array_signals = {}
    ordered_outputs = []

    # Nytt robust regex som hittar [index] även när det följs av ett [bit_range] i slutet
    array_pattern = re.compile(r"^(.*)\[(\d+)\](.*)$")

    with open(vcd_path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("$enddefinitions"):
                break
                
            if line.startswith("$scope"):
                parts = line.split()
                if len(parts) >= 3:
                    current_hierarchy.append(parts[2])
                    
            elif line.startswith("$upscope"):
                if current_hierarchy:
                    current_hierarchy.pop()
                    
            elif line.startswith("$var"):
                parts = line.split()
                if len(parts) >= 5:
                    signal_name = "".join(parts[4:-1])
                    full_path = ".".join(current_hierarchy) + "." + signal_name
                    
                    # Kontrollera om signalen matchar array-mönstret
                    match = array_pattern.match(full_path)
                    if match:
                        base_path = match.group(1)
                        index = int(match.group(2))
                        suffix = match.group(3) # Fångar upp t.ex. "[7:0]"
                        
                        key = (base_path, suffix)
                        if key not in array_signals:
                            array_signals[key] = []
                            ordered_outputs.append(("ARRAY", key))
                        
                        array_signals[key].append(index)
                    else:
                        ordered_outputs.append(("SIGNAL", full_path))

    print(f"--- Kollapsad signalhierarki i {vcd_path} ---")
    
    printed_arrays = set()
    for item_type, key in ordered_outputs:
        if item_type == "SIGNAL":
            print(key)
        elif item_type == "ARRAY" and key not in printed_arrays:
            base_path, suffix = key
            indices = sorted(array_signals[key])
            min_idx = indices[0]
            max_idx = indices[-1]
            
            # Skriver ut i det önskade formatet [max:min][bitar]
            print(f"{base_path}[{max_idx}:{min_idx}]{suffix}")
            printed_arrays.add(key)

    print("--------------------------------------------------")

if __name__ == "__main__":
    # Kör direkt mot din fil
    print_collapsed_vcd_hierarchy("filtered.vcd")


