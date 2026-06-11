#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// --- Wi-Fi設定 ---
const char* ssid = "cancers";
const char* password = "cancer1234";

// 固定IP設定
IPAddress local_IP(192, 168, 100, 15);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);
Servo myServo;
const int servoPin = 18;

// ガチャを動かす関数
void executeGachaAction() {
    Serial.println("Gacha Start via Web Request!");
    myServo.write(180);
    delay(500);
    myServo.write(0);
    delay(500);
}

void setup() {
    Serial.begin(115200);

    // サーボ初期設定
    ESP32PWM::allocateTimer(0);
    myServo.setPeriodHertz(50);
    myServo.attach(servoPin, 500, 2400);
    myServo.write(0);

    // Wi-Fi固定IP設定
    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        String currentLine = "";
        String requestPath = ""; 

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        
                        // --- 連動の核心部分 ---
                        // PC側のJavaScriptから「/SPIN」へのアクセスを検知した場合
                        if (requestPath.indexOf("GET /SPIN") >= 0) {
                            executeGachaAction(); // モーターを回す
                            
                            // ブラウザへ「成功したよ」と伝えるレスポンス（CORS許可ヘッダー付き）
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-Type: text/plain");
                            client.println("Access-Control-Allow-Origin: *"); // 別端末からの通信を許可
                            client.println("Connection: close");
                            client.println();
                            client.print("OK");
                        } 
                        else {
                            // /SPIN 以外へのアクセスには、不要な画面は返さず404を返す
                            client.println("HTTP/1.1 404 Not Found");
                            client.println("Connection: close");
                            client.println();
                        }
                        break;
                    } else {
                        if (currentLine.startsWith("GET")) {
                            requestPath = currentLine;
                        }
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }
        delay(1);
        client.stop();
    }
}