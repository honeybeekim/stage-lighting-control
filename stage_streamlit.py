import streamlit as st
import paho.mqtt.client as mqtt

BROKER   = "7373e0ca890b45a789270b5e6cba0c77.s1.eu.hivemq.cloud"
PORT     = 8883
USERNAME = "KIM_STAGE"
PASSWORD = "7142Hanmom!!"

cli = mqtt.Client()
cli.username_pw_set(USERNAME, PASSWORD)
cli.tls_set()   # TLS 보안
cli.connect(BROKER, PORT, 60)

st.set_page_config("Stage Lighting Control", layout="wide")
st.title("🎭 Stage Lighting Control - HiveMQ Cloud")

def send(msg):
    cli.publish("stage/rgb", msg)

col1, col2 = st.columns(2)

with col1:
    if st.button("ON 1"):
        send("ON 1")
    if st.button("OFF 1"):
        send("OFF 1")

with col2:
    if st.button("SCENE SHOW"):
        send("SCENE SHOW")
    if st.button("SCENE REST"):
        send("SCENE REST")
