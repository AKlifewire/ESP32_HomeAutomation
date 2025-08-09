# PlatformIO Setup - Based on Your Working Firmware

## Files Created
- `secrets.h` - Your backend config (update FIRMWARE_VERSION here)
- `working-main.cpp` - Your exact working firmware + minimal backend sync
- `platformio.ini` - PlatformIO configuration

## Quick Setup

1. **Create PlatformIO project:**
```bash
cd device-firmware
pio init --board esp32dev
```

2. **Copy files:**
```bash
copy working-main.cpp src\main.cpp
copy secrets.h src\secrets.h
```

3. **Install libraries:**
```bash
pio lib install "ArduinoJson" "PubSubClient"
```

4. **Flash firmware:**
```bash
pio run --target upload
pio device monitor
```

## What's Added to Your Working Firmware

**Minimal additions:**
- `#include "secrets.h"` - Backend config
- `reportBootStatus()` - Reports version on boot
- `sendTelemetry()` - Sends relay states every 60 seconds
- Boot report in `connectToMQTT()`

**Your exact relay logic preserved:**
- Active LOW relays (state ? LOW : HIGH)
- Same pin assignments (4, 5, 21, 22)
- Same MQTT callback structure
- Same connection handling

## Backend Sync

**On boot, device sends:**
```json
{
  "device_id": "hub-346570",
  "fw_version": "1.3.0",
  "build_timestamp": "Jan 8 2025 15:30:22",
  "owner_id": "Akorede",
  "ip_address": "192.168.1.100"
}
```

**For version updates:**
1. Edit `FIRMWARE_VERSION` in `secrets.h`
2. Flash with `pio run --target upload`
3. Device auto-reports new version to backend

## Expected Boot Output
```
🚀 SmartHomeHub v1.3.0
📋 Device ID: hub-346570
👤 Owner: Akorede
🔨 Built: Jan 8 2025 15:30:22
🌐 Connecting to WiFi: iPhone 11 Ak
✅ WiFi connected!
📍 IP address: 192.168.1.100
🔗 Connecting to AWS IoT...
✅ MQTT connected!
📡 Subscribed to: hub-346570/digital/control
📤 Online status sent
✅ Boot report sent: v1.3.0
🎉 Setup complete! Ready for commands.
```

## CI/CD Pipeline Integration

**This PlatformIO template works alongside your complete firmware CI/CD pipeline:**

### Component Architecture
| Component | Where it Runs | Purpose |
|-----------|---------------|----------|
| **PlatformIO** | Local dev machine | Development & local flashing |
| **GitHub Actions** | GitHub CI/CD server | Automated build, S3 upload, version registration |
| **Backend Lambda** | AWS Lambda | Updates DynamoDB, triggers OTA rollout |

### Workflow
1. **Local Development (PlatformIO)**
   - Edit `FIRMWARE_VERSION` in `secrets.h`
   - Test locally with `pio run --target upload`
   - Commit changes to trigger CI/CD

2. **CI/CD Build (firmware-ota.yml)**
   - Detects firmware changes
   - Uses PlatformIO CLI to build `.bin` files
   - Uploads to S3 with checksum
   - Calls `firmware-register` API

3. **Backend Update (firmware-register/index.js)**
   - Saves metadata to DynamoDB
   - Triggers canary rollout (5% of devices)
   - Monitors telemetry for stability

4. **OTA Rollout**
   - Canary devices update first
   - Gradual rollout to 100% if stable
   - Backend tracks success/failure rates

### Benefits
✅ **Local & Remote Sync** - Manual flashing still updates backend
✅ **Safety** - Canary rollouts prevent mass failures
✅ **AI Learning** - Telemetry comparison for firmware optimization
✅ **Fully Automated** - No manual S3 uploads or metadata updates

Your working firmware + complete CI/CD pipeline! 🚀