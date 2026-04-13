# 🍃 Self-Hosted Indoor Air Quality (AQI) Monitor

A private, local-first air quality monitoring system designed for high-resolution tracking of PM2.5, PM10, PM1.0, and Volatile Organic Compounds (VOCs). This project utilizes an ESP32-based sensor node, MQTT for data transport, and a full InfluxDB/Grafana stack for visualization.

## 🏗️ Architecture
- **Hardware:** ESP32 with PMS5003 (Laser Dust Sensor) and MQ135 (Gas Sensor).
- **Communication:** MQTT (via Mosquitto) over local Wi-Fi.
- **Backend:**
  - **Telegraf:** Consumes MQTT topics and writes to InfluxDB.
  - **InfluxDB 2.x:** High-performance time-series database.
  - **Grafana:** Dashboarding with a custom "mobile-first" kiosk wrapper.
- **Hosting:** Docker Compose on a Proxmox-based homelab environment.

## 📊 Visualizations
The system provides two primary views:
1. **Real-time Gauges:** Instantaneous look at air quality metrics.
2. **Time-Series Analysis:** Historical trends (1hr, 24hr, 7d) to identify spikes caused by cooking, cleaning, or external environmental factors.

## 📱 Mobile Kiosk
The project includes a custom `index.html` wrapper designed to be "Installed" as a Web App on iOS/Android. It features:
- A responsive 2x2 gauge grid.
- Smooth-interpolated historical graphs.
- Custom CSS to hide Grafana UI elements for a "native app" feel.
- Automated health-refresh logic to ensure 24/7 uptime on wall-mounted displays.

## 🚀 Deployment

### 1. Backend Stack
Deploy the stack using the provided `docker-compose.yml`:

```yaml
services:
  influxdb:
    image: influxdb:2.7
    container_name: influxdb
    restart: always

  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mosquitto
    restart: always

  telegraf:
    image: telegraf:latest
    container_name: telegraf
    depends_on: [influxdb, mosquitto]

  grafana:
    image: grafana/grafana-oss:latest
    container_name: grafana
    environment:
      - GF_SECURITY_ALLOW_EMBEDDING=true
      - GF_AUTH_ANONYMOUS_ENABLED=true
    restart: always

  kiosk-web:
    image: nginx:alpine
    container_name: kiosk-web
    volumes:
      - ./www:/usr/share/nginx/html:ro
