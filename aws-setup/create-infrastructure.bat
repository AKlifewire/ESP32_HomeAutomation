@echo off
REM AWS Infrastructure Setup for ESP32 Firmware CI/CD Pipeline
REM Run this script to create all required AWS resources

echo 🚀 Setting up AWS infrastructure for ESP32 firmware pipeline...

REM Configuration
set REGION=us-east-1
for /f %%i in ('powershell -command "Get-Date -UFormat %%s"') do set TIMESTAMP=%%i
set BUCKET_NAME=esp32-firmware-ota-%TIMESTAMP%
set LAMBDA_FUNCTION_NAME=firmware-register
set API_NAME=firmware-api

REM 1. Create S3 bucket for firmware storage
echo 📦 Creating S3 bucket: %BUCKET_NAME%
aws s3 mb s3://%BUCKET_NAME% --region %REGION%

REM Enable versioning
aws s3api put-bucket-versioning --bucket %BUCKET_NAME% --versioning-configuration Status=Enabled

REM 2. Create DynamoDB tables
echo 🗄️ Creating DynamoDB tables...

REM Firmware metadata table
aws dynamodb create-table --table-name firmware-metadata --attribute-definitions AttributeName=device_type,AttributeType=S AttributeName=version,AttributeType=S --key-schema AttributeName=device_type,KeyType=HASH AttributeName=version,KeyType=RANGE --billing-mode PAY_PER_REQUEST --region %REGION%

REM Devices table
aws dynamodb create-table --table-name devices --attribute-definitions AttributeName=device_id,AttributeType=S --key-schema AttributeName=device_id,KeyType=HASH --billing-mode PAY_PER_REQUEST --region %REGION%

REM 3. Create IAM role for Lambda
echo 🔐 Creating IAM role for Lambda function...
aws iam create-role --role-name firmware-register-role --assume-role-policy-document "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Effect\":\"Allow\",\"Principal\":{\"Service\":\"lambda.amazonaws.com\"},\"Action\":\"sts:AssumeRole\"}]}"

REM Attach policies
aws iam attach-role-policy --role-name firmware-register-role --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole

REM Get account ID
for /f "tokens=*" %%i in ('aws sts get-caller-identity --query Account --output text') do set ACCOUNT_ID=%%i

aws iam put-role-policy --role-name firmware-register-role --policy-name firmware-register-policy --policy-document "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Effect\":\"Allow\",\"Action\":[\"dynamodb:PutItem\",\"dynamodb:GetItem\",\"dynamodb:Scan\",\"dynamodb:Query\"],\"Resource\":[\"arn:aws:dynamodb:%REGION%:*:table/firmware-metadata\",\"arn:aws:dynamodb:%REGION%:*:table/devices\"]},{\"Effect\":\"Allow\",\"Action\":[\"s3:GetObject\",\"s3:GetObjectVersion\"],\"Resource\":\"arn:aws:s3:::%BUCKET_NAME%/*\"},{\"Effect\":\"Allow\",\"Action\":[\"iot:Publish\"],\"Resource\":\"*\"}]}"

REM 4. Package and deploy Lambda function
echo 📦 Packaging Lambda function...
cd ..\backend\firmware-register
powershell -command "Compress-Archive -Path index.js -DestinationPath function.zip -Force"
cd ..\..\aws-setup

REM Get IoT endpoint
for /f "tokens=*" %%i in ('aws iot describe-endpoint --endpoint-type iot:Data-ATS --query endpointAddress --output text') do set IOT_ENDPOINT=%%i

REM Create Lambda function
echo ⚡ Creating Lambda function...
aws lambda create-function --function-name %LAMBDA_FUNCTION_NAME% --runtime nodejs18.x --role arn:aws:iam::%ACCOUNT_ID%:role/firmware-register-role --handler index.handler --zip-file fileb://..\backend\firmware-register\function.zip --environment Variables="{IOT_ENDPOINT=%IOT_ENDPOINT%,FIRMWARE_TABLE=firmware-metadata,DEVICES_TABLE=devices}" --region %REGION%

REM 5. Create API Gateway
echo 🌐 Creating API Gateway...
for /f "tokens=*" %%i in ('aws apigatewayv2 create-api --name %API_NAME% --protocol-type HTTP --query ApiId --output text') do set API_ID=%%i

REM Create integration
for /f "tokens=*" %%i in ('aws apigatewayv2 create-integration --api-id %API_ID% --integration-type AWS_PROXY --integration-uri arn:aws:lambda:%REGION%:%ACCOUNT_ID%:function:%LAMBDA_FUNCTION_NAME% --payload-format-version 2.0 --query IntegrationId --output text') do set INTEGRATION_ID=%%i

REM Create route
aws apigatewayv2 create-route --api-id %API_ID% --route-key "POST /firmware-register" --target integrations/%INTEGRATION_ID%

REM Create stage
aws apigatewayv2 create-stage --api-id %API_ID% --stage-name prod --auto-deploy

REM Add Lambda permission for API Gateway
aws lambda add-permission --function-name %LAMBDA_FUNCTION_NAME% --statement-id api-gateway-invoke --action lambda:InvokeFunction --principal apigateway.amazonaws.com --source-arn "arn:aws:execute-api:%REGION%:%ACCOUNT_ID%:%API_ID%/*/*"

REM 6. Add sample canary device
echo 🔧 Adding sample canary device...
for /f "tokens=*" %%i in ('powershell -command "Get-Date -Format yyyy-MM-ddTHH:mm:ssZ"') do set CURRENT_TIME=%%i
aws dynamodb put-item --table-name devices --item "{\"device_id\":{\"S\":\"hub-346570\"},\"device_type\":{\"S\":\"SmartHomeHub\"},\"owner_id\":{\"S\":\"Akorede\"},\"canary_group\":{\"BOOL\":true},\"created_at\":{\"S\":\"%CURRENT_TIME%\"}}" --region %REGION%

REM Output configuration
echo.
echo ✅ AWS Infrastructure Setup Complete!
echo.
echo 📋 Configuration for GitHub Secrets:
echo FIRMWARE_BUCKET=%BUCKET_NAME%
echo BACKEND_API_URL=https://%API_ID%.execute-api.%REGION%.amazonaws.com/prod
echo.
echo 📋 Update your GitHub workflow with these values:
echo - S3 Bucket: %BUCKET_NAME%
echo - API Gateway URL: https://%API_ID%.execute-api.%REGION%.amazonaws.com/prod
echo.
echo 🔧 Next steps:
echo 1. Add AWS credentials to GitHub Secrets
echo 2. Update firmware-ota.yml with the bucket name and API URL
echo 3. Generate API token for backend authentication
echo 4. Test the pipeline by committing firmware changes