# GeminiESP: A Gemini-Powered ESP32 Voice Assistant

[![Watch the Video](https://customer-xesrjxb9wcc9tf16.cloudflarestream.com/54e3b813c08a8aa028cdb6bf75f0008a/thumbnails/thumbnail.jpg?time=&height=600)](https://customer-xesrjxb9wcc9tf16.cloudflarestream.com/54e3b813c08a8aa028cdb6bf75f0008a/iframe)

A DIY AI voice assistant inspired by the "Xiaozhi" robot, but powered by Google's **Gemini 1.5/2.0 Flash** models for ultra-fast conversational responses.

This project replaces the traditional multi-stage cloud pipeline (ASR -> LLM -> TTS) with a streamlined **Audio-to-Audio** loop using Gemini's multimodal capabilities.

## 📸 Overview

* **The Body:** ESP32-S3 (Freenove S3 Touch Display / AtomS3) handling audio I/O.
* **The Brain:** A local Python server (FastAPI) that bridges the ESP32 to the Gemini API.
* **The Voice:** Uses Google TTS (gTTS) or Gemini's native audio generation (future) to speak back.

## 🚀 Features

* **Push-to-Talk / Wake-Word Ready:** Records audio on the device and streams it to the server.
* **Fast Inference:** Uses `gemini-2.5-flash` or `gemini-3.0-flash` for near-instant responses.
* **Memory:** Maintains conversational context (it remembers your name and previous questions).
* **Customizable Personality:** You can change the system prompt to make the bot sassy, helpful, or technical.
* **Open Source Stack:** Built with Arduino (C++) and Python.

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32-S3 Board with PSRAM (Required for audio buffering).
    * *Recommended:* [Freenove ESP32-S3 Touch Display](https://github.com/Freenove/Freenove_ESP32_S3_Touch_Display_Video_Example) or M5Stack AtomS3 + Atomic Echo Base.
* **Audio:**
    * I2S Microphone (e.g., INMP441 or built-in).
    * I2S Amplifier + Speaker (e.g., MAX98357A or built-in).
* **Computer:** To run the Python bridge server (can be a PC, Mac, or Raspberry Pi).

---

## 🔧 Software Setup

### 1. The Python Brain (Server)

This server receives raw audio from the ESP32, sends it to Gemini, and returns a WAV file.

**Prerequisites:**
* Python 3.10+
* FFmpeg (required for `pydub` audio conversion)

**Installation:**

```bash
# 1. Clone the repo
git clone [https://github.com/yourusername/gemini-zhi.git](https://github.com/yourusername/gemini-zhi.git)
cd GeminiESP/talkback.py

# 2. Create a virtual environment
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

# 3. Install dependencies
pip install fastapi uvicorn google-genai gTTS pydub pyaudioop python-multipart
