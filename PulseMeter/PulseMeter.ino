// Joost Heijne 2021 - MIT license
// Utility pulse meter for Arduino

#define DEV_ID            10054 // don't really care
#define N_CHANNELS        5
#define SAMPLE_INTERVAL   (20UL / N_CHANNELS)
#define REPORT_INTERVAL   10000UL

const byte pins [N_CHANNELS] = { 
  0, // M1 pin
  1, // M2 pin
  2, // M3 pin
  3, // M4 pin
  7, // M5 pin
};

struct {
  byte sample;
  byte value;
  unsigned long pulse_count;
  unsigned long total_pulse_count;
} channels [N_CHANNELS] = {
  { 1, 1, 0, 0 },
  { 1, 1, 0, 0 },
  { 1, 1, 0, 0 },
  { 1, 1, 0, 0 },
  { 1, 1, 0, 0 },
};

void setup()
{
  char buf[32];

  Serial.begin(9600);
  sprintf(buf, "/%u:S0 Pulse Meter", DEV_ID);
  Serial.println(buf);

  for (unsigned int i = 0; i < N_CHANNELS; i++)
    pinMode (pins[i], INPUT_PULLUP);

}

void sample()
{
  static unsigned int channel = 0;
  
  // read pin once
  byte sample = digitalRead(pins[channel]);
  
  // compare to last sample
  if (channels[channel].sample == sample) {
    
    // 2 equal samples, consider valid
    if (channels[channel].value != sample && !sample) {

      // falling edge detected
      channels[channel].pulse_count++;      
      channels[channel].total_pulse_count++;      
    }

    // store validated value
    channels[channel].value = sample;
  }
  
  // save sample for next run
  channels[channel].sample = sample;

  if (++channel >= N_CHANNELS)
    channel = 0;
}

void report()
{
  char buf[64];
  sprintf(buf, "ID:%u:I:%lu", DEV_ID, REPORT_INTERVAL/1000UL);
  for (unsigned int i = 0; i < N_CHANNELS; i++) {
    char small_buf[16];
    sprintf(small_buf, ":M%d:%lu:%lu", i+1, channels[i].pulse_count, channels[i].total_pulse_count);
    strcat(buf, small_buf);
    channels[i].pulse_count = 0;
  }
  Serial.println(buf);
}

void loop() 
{
  static unsigned long last_sample = 0;
  static unsigned long last_report = 0;

  const unsigned long now = millis();

  if ((now - last_sample) > SAMPLE_INTERVAL) {
    last_sample += SAMPLE_INTERVAL;
    sample();
  } else if ((now - last_report) > REPORT_INTERVAL) {
    last_report += REPORT_INTERVAL;
    report();
  }
}
