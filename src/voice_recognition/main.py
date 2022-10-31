#julius -C ./am-gmm.jconf -nostrip -gram ./lagopus/lagopus -input mic
import socket
import whisper
import pyaudio
import wave

model = whisper.load_model("base")

HOST = '127.0.0.1'
PORT = 10500
DATESIZE = 1024

class Julius:

    def __init__(self):

        self.sock = None

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as self.sock:
            self.sock.connect((HOST, PORT))

            strTemp = "" 
            fin_flag = False 

            while True:

                data = self.sock.recv(DATESIZE).decode('utf-8')
                
                for line in data.split('\n'):
                    # 受信データから、<WORD>の後に書かれている言葉を抽出して変数に格納する。
                    # <WORD>の後に、話した言葉が記載されている。
                    index = line.find('WORD="')
                    if index != -1:
                        # strTempに話した言葉を格納
                        strTemp = strTemp + line[index+6:line.find('"',index+6)]
                        
                    # 受信データに</RECOGOUT>'があれば、話終わり ⇒ フラグをTrue
                    if '</RECOGOUT>' in line:
                        fin_flag = True
                # 話した言葉毎に、print文を実行
                if fin_flag == True:
                    print(strTemp)
                    #self.record_mic()
                    #self.recog()
                    fin_flag = False
                    strTemp = ""


    def record_mic(self):
        CHUNK = 1024
        FORMAT = pyaudio.paInt16
        CHANNELS = 1
        RATE = 44100
        WAVE_OUTPUT_FILENAME = "output.wav"

        p = pyaudio.PyAudio()

        stream = p.open(format=FORMAT,
                        channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK)

        print("* recording")

        frames = []

        while True:
            try:
                # Record
                d = stream.read(CHUNK)
                frames.append(d)

            except KeyboardInterrupt:
                # Ctrl - c
                break

        print("* done recording")

        stream.stop_stream()
        stream.close()
        p.terminate()

        with wave.open(WAVE_OUTPUT_FILENAME, 'wb') as wf:
            wf.setnchannels(CHANNELS)
            wf.setsampwidth(p.get_sample_size(FORMAT))
            wf.setframerate(RATE)
            wf.writeframes(b''.join(frames))
            wf.close()


    def recog(self):
        result = model.transcribe("output.wav")
        print(result["text"])


    def recognize(self):
        # load audio and pad/trim it to fit 30 seconds
        audio = whisper.load_audio("output.wav")
        audio = whisper.pad_or_trim(audio)

        # make log-Mel spectrogram and move to the same device as the model
        mel = whisper.log_mel_spectrogram(audio).to(model.device)

        # detect the spoken language
        _, probs = model.detect_language(mel)
        print(f"Detected language: {max(probs, key=probs.get)}")

        # decode the audio
        options = whisper.DecodingOptions(fp16 = False)
        result = whisper.decode(model, mel, options)

        # print the recognized text
        print(result.text)


if __name__ == "__main__":
    julius = Julius()
    julius.run()