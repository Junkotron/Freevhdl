import vcdvcd

INPUT_FILE = "waveforms.vcd"
OUTPUT_FILE = "filtered.vcd"

# Din lista (Skriv bara basnamnen precis som i loggen, inga klamrar)
signals_to_keep = [
    "CLK_24MHz",
    "alow",
    "ahigh",
    "oric.alow",
    "oric.ahigh",
    "oric.inst_cpu.clk",
    "oric.inst_cpu.do",     # 8 bitar
    "oric.inst_cpu.di",     # 8 bitar
    "oric.inst_cpu.res_n",
    "oric.resetn"
]

print(f"Fas 1: Skannar råa bit-storlekar från {INPUT_FILE}...")

# Skanna originalfilen rad för rad för att hitta de exakta bit-bredderna
real_sizes = {}
with open(INPUT_FILE, "r") as f:
    current_scope = []
    for line in f:
        line_strip = line.strip()
        
        if line_strip.startswith("$scope"):
            parts = line_strip.split()
            if len(parts) >= 3:
                current_scope.append(parts[2])
        elif line_strip.startswith("$upscope"):
            if current_scope:
                current_scope.pop()
                
        elif line_strip.startswith("$var"):
            parts = line_strip.split()
            if len(parts) >= 5:
                size_val = int(parts[2]) 
                raw_name = parts[4]      
                base_name = raw_name.split("[")[0].strip()
                
                # Bygg den fulla sökvägen
                if current_scope:
                    full_path = ".".join(current_scope) + "." + base_name
                else:
                    full_path = base_name # Om den ligger på absolut rot-nivå
                
                real_sizes[full_path] = size_val
                # Spara även utan "top."-prefix som en säkerhetsåtgärd
                real_sizes[full_path.replace("top.", "")] = size_val
                
        if "$enddefinitions" in line_strip:
            break

# Kör vcdvcd för att parsa källfilens metadata och ID-koder
meta_vcd = vcdvcd.VCDVCD(INPUT_FILE, store_tvs=False)

old_id_to_new_ids = {}
wanted_old_ids = set()
signals_metadata = []
ascii_counter = 33 # Starta på ASCII 33 ('!')

for name, old_id in meta_vcd.references_to_ids.items():
    if name in signals_to_keep:
        wanted_old_ids.add(old_id)
        
        new_id = chr(ascii_counter)
        ascii_counter += 1
        
        if old_id not in old_id_to_new_ids:
            old_id_to_new_ids[old_id] = []
        old_id_to_new_ids[old_id].append(new_id)
        
        # Hämta den sanna storleken
        bit_size = real_sizes.get(name, 1)
        
        # --- FIX FÖR TOPPNIVÅN ---
        # Om namnet saknar punkt (t.ex. bara "a"), tvingar vi in den i "top"-scopet 
        # så att trädgeneratorn inte kraschar eller skapar tomma mappar.
        if "." not in name:
            parts = ["top", name]
        else:
            parts = name.split(".")
            
        signals_metadata.append({
            "full_name": name,
            "new_id": new_id,
            "parts": parts,
            "size": bit_size
        })
        print(f"   -> Signal verifierad: {name} (ID: {new_id}, Storlek: {bit_size} bit)")

print(f"Fas 2: Genererar {OUTPUT_FILE} till svcd...")

with open(OUTPUT_FILE, "w") as out:
    out.write("$comment Generated with Top-Level Vector Fix for svcd $end\n")
    out.write(f"$timescale {meta_vcd.timescale['timescale']} $end\n")
    
    # Sortera trädet snyggt
    signals_metadata.sort(key=lambda x: len(x["parts"]))
    current_open_scopes = []
    
    for sig in signals_metadata:
        parts = sig["parts"]
        new_id = sig["new_id"]
        signal_name = parts[-1]
        target_scopes = parts[:-1]
        bit_size = sig["size"]
        
        while current_open_scopes and (len(current_open_scopes) > len(target_scopes) or current_open_scopes != target_scopes[:len(current_open_scopes)]):
            out.write("$upscope $end\n")
            current_open_scopes.pop()
            
        while len(current_open_scopes) < len(target_scopes):
            next_scope = target_scopes[len(current_open_scopes)]
            out.write(f"$scope module {next_scope} $end\n")
            current_open_scopes.append(next_scope)
            
        # Skriv ut på rent klammerlöst format med rätt storlek!
        out.write(f"$var wire {bit_size} {new_id} {signal_name} $end\n")
        
    while current_open_scopes:
        out.write("$upscope $end\n")
        current_open_scopes.pop()
        
    out.write("$enddefinitions $end\n")

    # Fas 3: Strömma och duplicera värdeförändringarna i realtid
    class FilterCallbacks(vcdvcd.StreamParserCallbacks):
        def __init__(self):
            self.last_time = -1

        def value(self, vcd_obj, time, value, identifier_code, cur_sig_vals):
            if identifier_code in wanted_old_ids:
                if time != self.last_time:
                    out.write(f"#{time}\n")
                    self.last_time = time
                
                list_of_target_ids = old_id_to_new_ids[identifier_code]
                for translated_new_id in list_of_target_ids:
                    if len(value) > 1 and not value.startswith(('b', 'B', 'r', 'R')):
                        out.write(f"b{value} {translated_new_id}\n")
                    else:
                        out.write(f"{value}{translated_new_id}\n")

    vcdvcd.VCDVCD(INPUT_FILE, callbacks=FilterCallbacks(), store_tvs=False)

print("Klart! Toppnivå-vektorer hanteras nu stabilt.")
