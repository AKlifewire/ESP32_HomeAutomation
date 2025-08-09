# GitHub Secrets Configuration

## Required Secrets

Add these secrets to your GitHub repository:
**Settings → Secrets and variables → Actions → New repository secret**

### AWS Credentials
```
AWS_ACCESS_KEY_ID=AKIA...
AWS_SECRET_ACCESS_KEY=your-secret-key
```

### Backend API Token
```
BACKEND_API_TOKEN=your-generated-token
```

### Auto-generated (GitHub provides)
```
GITHUB_TOKEN=automatically-available
```

## Generate API Token

Create a secure token for backend authentication:

```bash
# Generate random token
openssl rand -hex 32

# Or use Python
python3 -c "import secrets; print(secrets.token_hex(32))"
```

## Verify Setup

After adding secrets, check:
1. Repository → Settings → Secrets and variables → Actions
2. Should see 3 secrets: AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY, BACKEND_API_TOKEN
3. GITHUB_TOKEN is automatically available

## Test Pipeline

1. Update firmware version in `src/secrets.h`:
```cpp
#define FIRMWARE_VERSION "1.4.0"
```

2. Commit and push:
```bash
git add src/secrets.h
git commit -m "Update firmware to v1.4.0"
git push origin main
```

3. Check GitHub Actions tab for build progress