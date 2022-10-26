#include <si5351.h>
#include <Wire.h>

Si5351 si5351;

/*
  This reads a wave file from an SD card and plays it using the I2S interface to
  a MAX98357 I2S Amp Breakout board.

  Circuit:
   Arduino Zero, MKR Zero or MKR1000 board
   SD breakout or shield connected
   MAX98357:
     GND connected GND
     VIN connected 5V
     LRC connected to pin 0 (Zero) or pin 3 (MKR1000, MKR Zero)
     BCLK connected to pin 1 (Zero) or pin 2 (MKR1000, MKR Zero)
     DIN connected to pin 9 (Zero) or pin A6 (MKR1000, MKR Zero)

  created 15 November 2016
  by Sandeep Mistry
*/

#include <SD.h>
#include <ArduinoSound.h>

// filename of wave file to play
const char filename[] = "001.WAV";

// variable representing the Wave File
SDWaveFile waveFile;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  bool i2c_found;
  // Start serial and initialize the Si5351
  i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if (!i2c_found)
  {
    Serial.println("Device not found on I2C bus!");
  }

  si5351.set_ref_freq(2500000000ULL, SI5351_PLL_INPUT_CLKIN);

  // Set CLK0 to output 14 MHz
  si5351.set_freq(4800000ULL, SI5351_CLK0);
  si5351.set_freq(307200000ULL, SI5351_CLK1);
  //si5351.set_freq(4800000ULL, SI5351_CLK0);
  //si5351.set_freq(153600000ULL, SI5351_CLK1);
  //si5351.set_freq(75000ULL, SI5351_CLK2);
  // Query a status update and wait a bit to let the Si5351 populate the
  // status flags correctly.
  //si5351.update_status();
  delay(500);
  /*Serial.print("SYS_INIT: ");
    Serial.print(si5351.dev_status.SYS_INIT);
    Serial.print("  LOL_A: ");
    Serial.print(si5351.dev_status.LOL_A);
    Serial.print("  LOL_B: ");
    Serial.print(si5351.dev_status.LOL_B);
    Serial.print("  LOS: ");
    Serial.print(si5351.dev_status.LOS);
    Serial.print("  REVID: ");
    Serial.println(si5351.dev_status.REVID);*/

  // setup the SD card, depending on your shield of breakout board
  // you may need to pass a pin number in begin for SS
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // create a SDWaveFile
  waveFile = SDWaveFile(filename);

  // check if the WaveFile is valid
  if (!waveFile) {
    Serial.println("wave file is invalid!");
    while (1); // do nothing
  }

  // print out some info. about the wave file
  Serial.print("Bits per sample = ");
  Serial.println(waveFile.bitsPerSample());

  long channels = waveFile.channels();
  Serial.print("Channels = ");
  Serial.println(channels);

  long sampleRate = waveFile.sampleRate();
  Serial.print("Sample rate = ");
  Serial.print(sampleRate);
  Serial.println(" Hz");

  long duration = waveFile.duration();
  Serial.print("Duration = ");
  Serial.print(duration);
  Serial.println(" seconds");

  // adjust the playback volume
  AudioOutI2S.volume(100);

  // check if the I2S output can play the wave file
  if (!AudioOutI2S.canPlay(waveFile)) {
    Serial.println("unable to play wave file using I2S!");
    while (1); // do nothing
  }


}

void loop() {
  // start playback
  while (Serial.read() == -1);
  Serial.println("starting playback");
  AudioOutI2S.play(waveFile);
  delay(1);
  si5351.update_status();
  // check if playback is still going on
  while (AudioOutI2S.isPlaying());
  Serial.println("playback stopped");
}
