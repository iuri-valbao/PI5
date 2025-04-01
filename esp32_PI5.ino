#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// Definições do Wi-Fi
#define SSID_WIFI "XXXX"
#define SENHA_WIFI "XXXX"

// Configurações do AWS IoT Core
#define ENDPOINT_AWS_IOT "endpoint.iot.sa-east-1.amazonaws.com"
#define TOPICO_MQTT "horta/sensores"

#define DHTPIN 4         // Pino do sensor DHT
#define DHTTYPE DHT22    // Tipo do sensor (DHT11 ou DHT22)

DHT dht(DHTPIN, DHTTYPE);

// Certificados da AWS IoT (substituir pelo certificado)
const char CERTIFICADO[] = R"(
-----BEGIN CERTIFICATE-----
CERTIFICADO_AQUI
-----END CERTIFICATE-----
)";

const char CHAVE_PRIVADA[] = R"(
-----BEGIN RSA PRIVATE KEY-----
CHAVE_PRIVADA_AQUI
-----END RSA PRIVATE KEY-----
)";

const char CERTIFICADO_CA[] = R"(
-----BEGIN CERTIFICATE-----
CERTIFICADO_ROOT_CA_AWS
-----END CERTIFICATE-----
)";

WiFiClientSecure net;
PubSubClient client(net);

void conectarWiFi() {
    Serial.print("Conectando ao Wi-Fi...");
    WiFi.begin(SSID_WIFI, SENHA_WIFI);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" Conectado!");
}

void conectarAWSIoT() {
    net.setCACert(CERTIFICADO_CA);
    net.setCertificate(CERTIFICADO);
    net.setPrivateKey(CHAVE_PRIVADA);
    
    client.setServer(ENDPOINT_AWS_IOT, 8883);

    Serial.print("Conectando ao AWS IoT Core...");
    while (!client.connect("ESP32_Horta")) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" Conectado!");
}

void enviarDados() {
    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();

    if (isnan(temperatura) || isnan(umidade)) {
        Serial.println("Erro ao ler sensor!");
        return;
    }

    String payload = "{";
    payload += "\"sensor_id\": \"sensor_001\",";
    payload += "\"temperatura\": " + String(temperatura, 2) + ",";
    payload += "\"umidade\": " + String(umidade, 2) + ",";
    payload += "\"timestamp\": " + String(millis());
    payload += "}";

    Serial.print("Publicando no MQTT: ");
    Serial.println(payload);

    if (client.publish(TOPICO_MQTT, payload.c_str())) {
        Serial.println("Dados enviados com sucesso!");
    } else {
        Serial.println("Falha ao enviar os dados!");
    }
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    conectarWiFi();
    conectarAWSIoT();
}

void loop() {
    if (!client.connected()) {
        conectarAWSIoT();
    }
    client.loop();

    enviarDados();
    delay(5000); // Envia os dados a cada 5 segundos
}
