import io
import uvicorn
from fastapi import FastAPI, Request, Response
from google import genai
from google.genai import types
from gtts import gTTS
from pydub import AudioSegment

# CONFIGURATION
API_KEY = "API KEY"  # Paste your key here

# Initialize the new Client
client = genai.Client(api_key=API_KEY)

app = FastAPI()

# System instruction for the bot
sys_instruct = "You are an AI voice assistant. Keep answers short (1 sentence)."

@app.post("/chat")
async def audio_chat(request: Request):
    # 1. Receive Audio from ESP32
    audio_bytes = await request.body()
    print(f"🎤 Received {len(audio_bytes)} bytes")

    try:
        # 2. Send to Gemini (New SDK Syntax)
        # The new SDK handles audio bytes directly via the 'types.Part' object
        response = client.models.generate_content(
            model="gemini-2.5-flash", # Use 2.0 Flash if available, it's faster/better
            contents=[
                types.Part.from_bytes(data=audio_bytes, mime_type="audio/wav"),
                "Reply to this audio."
            ],
            config=types.GenerateContentConfig(
                system_instruction=sys_instruct
            )
        )
        
        text_reply = response.text
        print(f"🤖 Gemini: {text_reply}")

        # 3. Text-to-Speech (TTS)
        tts = gTTS(text=text_reply, lang='en')
        mp3_fp = io.BytesIO()
        tts.write_to_fp(mp3_fp)
        mp3_fp.seek(0)

        # Convert MP3 to WAV (16kHz, Mono, 16-bit)
        sound = AudioSegment.from_file(mp3_fp, format="mp3")
        sound = sound.set_frame_rate(16000).set_channels(1).set_sample_width(2)
        
        wav_fp = io.BytesIO()
        sound.export(wav_fp, format="wav")
        wav_data = wav_fp.getvalue()

        # 4. Send WAV bytes back
        print(f"🔊 Sending {len(wav_data)} bytes of audio")
        return Response(content=wav_data, media_type="audio/wav")

    except Exception as e:
        print(f"Error: {e}")
        return Response(content=str(e), status_code=500)

if __name__ == "__main__":
    uvicorn.run(app, host="192.168.10.101", port=8000)
