# GitHub Secrets Setup

## 🔐 Required Secrets

Add these to your GitHub repository:
**Repository → Settings → Secrets and variables → Actions → New repository secret**

### 1. AWS_ACCESS_KEY_ID
```
Value: Your AWS Access Key ID
```

### 2. AWS_SECRET_ACCESS_KEY  
```
Value: Your AWS Secret Access Key
```

### 3. BACKEND_API_TOKEN
```
Value: ZhrijB3GOMnE7XRHmefikqQ6iPSwcs3RmBJKR/l+dhU=
```

## 🚀 Test Pipeline

Update firmware version to trigger the pipeline:

```cpp
#define FIRMWARE_VERSION "1.5.0"
```

Then commit and push:
```bash
git add src/secrets.h
git commit -m "Test CI/CD pipeline with v1.5.0"
git push origin main
```

## 📊 Monitor Pipeline

1. **GitHub Actions**: Repository → Actions tab
2. **AWS S3**: Check `esp32-firmware-ota-1754784838.95337` bucket
3. **DynamoDB**: Verify `firmware-metadata` table entries
4. **Device Logs**: Monitor ESP32 serial output for OTA notifications

## ✅ Expected Results

- GitHub Actions builds firmware successfully
- Firmware uploaded to S3 with checksum
- Backend registers new version in DynamoDB
- Canary device (hub-346570) receives OTA notification
- Device downloads and installs update

Your complete CI/CD pipeline is ready! 🎉