import streamlit as st
import paho.mqtt.client as mqtt

BROKER   = "7373e0ca890b45a789270b5e6cba0c77.s1.eu.hivemq.cloud"
PORT     = 8883
USERNAME = "KIM_STAGE"
PASSWORD = "7142Hanmom!!"

connected = False

def on_connect(cli, userdata, flags, rc):
    global connected
    connected = True

def on_disconnect(cli, userdata, rc):
    global connected
    connected = False

cli = mqtt.Client()
cli.username_pw_set(USERNAME, PASSWORD)
cli.tls_set()
cli.on_connect = on_connect
cli.on_disconnect = on_disconnect
cli.connect(BROKER, PORT, 60)
cli.loop_start()

st.set_page_config("Stage Lighting Control", layout="wide")
st.title("🎭 Stage Lighting Control - HiveMQ Cloud")

# 연결 상태 표시
if connected:
    st.markdown("🔵 **Broker Connected**")
else:
    st.markdown("🔴 **Broker Disconnected**")

# 그룹별 슬라이더 제어
def send_rgb(group, r, g, b):
    msg = f"RGB {group} {r} {g} {b}"
    cli.publish("stage/rgb", msg)

for g in range(1,5):
    st.subheader(f"Group {g}")
    r = st.slider(f"R{g}", 0, 255, 0, key=f"r{g}")
    gval = st.slider(f"G{g}", 0, 255, 0, key=f"g{g}")
    b = st.slider(f"B{g}", 0, 255, 0, key=f"b{g}")
    if st.button(f"Apply Group {g}"):
        send_rgb(g, r, gval, b)

# 통합 ON/OFF 버튼
st.subheader("통합 제어 버튼")
col1, col2, col3 = st.columns(3)

with col1:
    if st.button("ON 1+2"): cli.publish("stage/rgb", "ON12")
    if st.button("OFF 1+2"): cli.publish("stage/rgb", "OFF12")
with col2:
    if st.button("ON 1+2+3"): cli.publish("stage/rgb", "ON123")
    if st.button("OFF 1+2+3"): cli.publish("stage/rgb", "OFF123")
with col3:
    if st.button("ON 1+2+3+4"): cli.publish("stage/rgb", "ON1234")
    if st.button("OFF 1+2+3+4"): cli.publish("stage/rgb", "OFF1234")
