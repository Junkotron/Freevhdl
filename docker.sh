#!/bin/bash
set -e

CONTAINER_NAME="fpga-dev"
IMAGE="ubuntu:25.10"

echo "=== 1. Rensar gammal container om den finns ==="
docker rm -f "$CONTAINER_NAME" 2>/dev/null || true

echo "=== 2. Startar ny container i bakgrunden ==="
docker run -d --name "$CONTAINER_NAME" -it "$IMAGE" bash

echo "=== 3. Kör grundinstallation och paketresurs ==="
docker exec -i "$CONTAINER_NAME" bash -s << 'EOF'
export DEBIAN_FRONTEND=noninteractive

# Grundläggande paket
apt update
apt install -y git sudo cmake psmisc software-properties-common mailutils

# Skapa användaren 'pi' med hemkatalog och bash som skal
useradd -m -s /bin/bash pi

# Lägg till 'pi' i sudo-gruppen
usermod -aG sudo pi

# Konfigurera lösenordsfri sudo för 'pi'
echo "pi ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/pi
chmod 0440 /etc/sudoers.d/pi
EOF

echo "=== 4. Sätt lösenord för användaren 'pi' manuellt ==="
docker exec -it "$CONTAINER_NAME" passwd pi

echo "=== 5. Klonar repo och kör installation som användare 'pi' ==="
# Byt ut URL:en till ditt faktiska git-repo och skript
docker exec -i "$CONTAINER_NAME" su - pi -c '
git clone https://github.com/din-anvandare/ditt-repo.git workspace
cd workspace
# Exempel: om du vill köra i bakgrunden med nohup som du brukar:
# nohup ./1_install.sh > build.log 2>&1 &
'

echo "=== Klart! Anslut till miljön med: ==="
echo "docker exec -it $CONTAINER_NAME su - pi"
