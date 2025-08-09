# Complete CI/CD Pipeline Setup

## Architecture Overview

```
Local Dev (PlatformIO) → GitHub Actions → AWS Lambda → OTA Rollout
```

## Files Created

### 1. GitHub Actions Workflow
- `.github/workflows/firmware-ota.yml` - Automated build and deployment

### 2. Backend Lambda Function  
- `backend/firmware-register/index.js` - Firmware registration and rollout

### 3. PlatformIO Template
- `src/secrets.h` - Configuration and version management
- `src/working-main.cpp` - Firmware with backend sync

## Setup Instructions

### 1. GitHub Repository Setup

**Required Secrets:**
```bash
AWS_ACCESS_KEY_ID=your-access-key
AWS_SECRET_ACCESS_KEY=your-secret-key
BACKEND_API_TOKEN=your-api-token
GITHUB_TOKEN=auto-generated
```

**Environment Variables (in workflow):**
```yaml
FIRMWARE_BUCKET: your-firmware-ota-bucket
BACKEND_API_URL: https://your-api-gateway-url.amazonaws.com/prod
```

### 2. AWS Infrastructure

**DynamoDB Tables:**
- `firmware-metadata` - Stores firmware versions and metadata
- `devices` - Device registry with canary group flags

**S3 Bucket:**
- `your-firmware-ota-bucket` - Stores firmware binaries

**Lambda Function:**
- Deploy `backend/firmware-register/index.js`
- Environment variables: `IOT_ENDPOINT`, `FIRMWARE_TABLE`, `DEVICES_TABLE`

**API Gateway:**
- POST `/firmware-register` → Lambda function

### 3. Workflow Process

**1. Local Development:**
```bash
# Edit firmware version
vim src/secrets.h  # Update FIRMWARE_VERSION

# Test locally
pio run --target upload
pio device monitor

# Commit changes
git add .
git commit -m "Update firmware to v1.4.0"
git push origin main
```

**2. Automated CI/CD:**
- GitHub Actions detects changes
- Builds firmware with PlatformIO
- Generates SHA256 checksum
- Uploads `.bin` to S3
- Calls backend API to register firmware
- Creates GitHub release

**3. Backend Processing:**
- Stores firmware metadata in DynamoDB
- Identifies canary devices (5% of fleet)
- Sends OTA notifications via MQTT
- Monitors rollout success rates

**4. OTA Rollout:**
- Canary devices download and install
- Backend monitors telemetry for failures
- Gradual rollout: 5% → 25% → 50% → 100%

## Canary Device Setup

**Mark devices as canary in DynamoDB:**
```json
{
  "device_id": "hub-346570",
  "device_type": "SmartHomeHub", 
  "canary_group": true,
  "owner_id": "Akorede"
}
```

## Monitoring

**GitHub Actions:**
- Build status and artifacts
- Release creation

**AWS CloudWatch:**
- Lambda function logs
- OTA success/failure rates

**Device Telemetry:**
- Firmware version reporting
- Update success confirmation

## Benefits

✅ **Automated Builds** - No manual compilation
✅ **Safe Rollouts** - Canary testing prevents mass failures  
✅ **Version Tracking** - Complete audit trail
✅ **Rollback Capability** - Quick revert to previous versions
✅ **AI Integration** - Telemetry analysis for optimization

Your complete firmware CI/CD pipeline! 🚀