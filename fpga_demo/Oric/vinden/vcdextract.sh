#!/bin/bash

# 1. Skapa venv om den inte redan finns
if [ ! -d "venv" ]; then
    echo "Skapar venv..."
    python3 -m venv venv
    ./venv/bin/pip install vcdvcd
fi

# 2. Kör skriptet direkt med venv-Python (ingen manuell aktivering krävs!)
./venv/bin/python3 filter_vcd.py

