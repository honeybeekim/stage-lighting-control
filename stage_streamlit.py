import streamlit as st
import paho.mqtt.client as mqtt
import threading, time

BROKER = "192.168.0.10"
TOPIC = "stage/rgb"
STATUS = "stage/status"

mqtt_status = "OFFLINE"

def on_connect(client, userdata, flags, rc):
    client.subscribe(STATUS)

def on_message(client, userdata, msg):
    global mqtt_status
    if msg.topic == STATUS:
        mqtt_status = msg.payload.decode()

cli = mqtt.Client()
cli.on_connect = on_connect
cli.on_message = on_message
cli.connect(BROKER, 1883, 60)

# MQTT 백그라운드 루프 실행
def mqtt_loop():
    cli.loop_forever()

thread = threading.Thread(target=mqtt_loop, daemon=True)
thread.start()

st.set_page_config("Stage Lighting Control", layout="wide")
st.title("🎭 소극장 무대 조명 제어")

# === 통신 상태 표시 ===
if mqtt_status == "ONLINE":
    st.markdown("### 🔵 MQTT Connected")
else:
    st.markdown("### 🔴 MQTT Disconnected")

def send(msg):
    cli.publish(TOPIC, msg)

# === 개별 그룹 컨트롤 ===
for i in range(1,5):
    st.subheader(f"Group {i}")
    col1, col2 = st.columns([1,2])
    with col1:
        if st.button(f"ON {i}"):
            send(f"ON {i}")
        if st.button(f"OFF {i}"):
            send(f"OFF {i}")
    with col2:
        r = st.slider(f"R{i}", 0,255,128)
        g = st.slider(f"G{i}", 0,255,128)
        b = st.slider(f"B{i}", 0,255,128)
        if st.button(f"SET {i}"):
            send(f"SET {i} {r} {g} {b}")

# === 그룹 조합 버튼 ===
st.subheader("그룹 조합 제어")
if st.button("ON 1+2"):
    send("ON 1+2")
if st.button("OFF 1+2"):
    send("OFF 1+2")
if st.button("ON 1+2+3"):
    send("ON 1+2+3")
if st.button("OFF 1+2+3"):
    send("OFF 1+2+3")
if st.button("ON 1+2+3+4"):
    send("ON 1+2+3+4")
if st.button("OFF 1+2+3+4"):
    send("OFF 1+2+3+4")
