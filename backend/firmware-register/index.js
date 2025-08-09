const AWS = require('aws-sdk');
const dynamodb = new AWS.DynamoDB.DocumentClient();
const iot = new AWS.IoTData({ endpoint: process.env.IOT_ENDPOINT });

exports.handler = async (event) => {
    try {
        const { 
            device_type, 
            version, 
            checksum, 
            s3_url, 
            build_info, 
            release_notes 
        } = JSON.parse(event.body);

        // Store firmware metadata in DynamoDB
        const firmwareRecord = {
            TableName: process.env.FIRMWARE_TABLE,
            Item: {
                device_type,
                version,
                checksum,
                s3_url,
                build_info,
                release_notes,
                created_at: new Date().toISOString(),
                status: 'available',
                rollout_stage: 'canary',
                rollout_percentage: 5
            }
        };

        await dynamodb.put(firmwareRecord).promise();

        // Get canary devices (5% of fleet)
        const canaryDevices = await getCanaryDevices(device_type);
        
        // Send OTA notification to canary devices
        const otaMessage = {
            type: 'ota_available',
            firmware: {
                version,
                checksum,
                download_url: await generatePresignedUrl(s3_url),
                size: await getFirmwareSize(s3_url)
            },
            rollout_info: {
                stage: 'canary',
                priority: 'normal'
            }
        };

        // Publish to canary devices
        for (const device of canaryDevices) {
            const topic = `devices/${device.device_id}/ota`;
            await iot.publish({
                topic,
                payload: JSON.stringify(otaMessage)
            }).promise();
        }

        return {
            statusCode: 200,
            headers: {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            body: JSON.stringify({
                success: true,
                message: `Firmware v${version} registered and deployed to ${canaryDevices.length} canary devices`,
                firmware_id: `${device_type}-${version}`,
                rollout_stage: 'canary'
            })
        };

    } catch (error) {
        console.error('Firmware registration error:', error);
        
        return {
            statusCode: 500,
            headers: {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            body: JSON.stringify({
                success: false,
                error: error.message
            })
        };
    }
};

async function getCanaryDevices(deviceType) {
    const params = {
        TableName: process.env.DEVICES_TABLE,
        FilterExpression: 'device_type = :type AND canary_group = :canary',
        ExpressionAttributeValues: {
            ':type': deviceType,
            ':canary': true
        }
    };

    const result = await dynamodb.scan(params).promise();
    return result.Items || [];
}

async function generatePresignedUrl(s3Url) {
    const s3 = new AWS.S3();
    const urlParts = s3Url.replace('s3://', '').split('/');
    const bucket = urlParts[0];
    const key = urlParts.slice(1).join('/');

    return s3.getSignedUrl('getObject', {
        Bucket: bucket,
        Key: key,
        Expires: 3600 // 1 hour
    });
}

async function getFirmwareSize(s3Url) {
    const s3 = new AWS.S3();
    const urlParts = s3Url.replace('s3://', '').split('/');
    const bucket = urlParts[0];
    const key = urlParts.slice(1).join('/');

    try {
        const headResult = await s3.headObject({
            Bucket: bucket,
            Key: key
        }).promise();
        return headResult.ContentLength;
    } catch (error) {
        console.error('Error getting firmware size:', error);
        return 0;
    }
}