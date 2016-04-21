// based on example from
// http://docs.openlighting.org/ola/doc/latest/dmx_cpp_client_tutorial.html

// build for local:
// g++ olamapper.cpp -o olamapper.out $(pkg-config --cflags --libs libola)
// build for aria board:
// arm-linux-gnueabi-g++  olamapper.cpp -o olamapper.aria $(pkg-config --cflags --libs libola)
// copy to board:
// scp olamapper.aria light@arai.loca:/home/light/OLA_channel_mapper_cpp


#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>
#include <string>

static const unsigned int UNIVERSE_IN = 1;
static const unsigned int UNIVERSE_OUT = 2;

ola::client::OlaClientWrapper wrapper;
ola::DmxBuffer channels_out;

static const unsigned int channel_count = 240;
int map[channel_count] = {
  127, 127, 127, 127, 158, 159, 127, 127, 127, 127, 150, 151,
  127, 127, 127, 127, 142, 143, 127, 127, 127, 127, 134, 135,
  156, 157, 154, 155, 152, 153, 148, 149, 146, 147, 144, 145,
  140, 141, 138, 139, 136, 137, 132, 133, 130, 131, 128, 129,
  95, 95, 95, 95, 126, 127, 95, 95, 95, 95, 118, 119,
  95, 95, 95, 95, 110, 111, 95, 95, 95, 95, 102, 103,
  124, 125, 122, 123, 120, 121, 116, 117, 114, 115, 112, 113,
  108, 109, 106, 107, 104, 105, 100, 101, 98, 99, 96, 97,
  63, 63, 63, 63, 94, 95, 63, 63, 63, 63, 86, 87,
  63, 63, 63, 63, 78, 79, 63, 63, 63, 63, 70, 71,
  92, 93, 90, 91, 88, 89, 84, 85, 82, 83, 80, 81,
  76, 77, 74, 75, 72, 73, 68, 69, 66, 67, 64, 65,
  31, 31, 31, 31, 62, 63, 31, 31, 31, 31, 54, 55,
  31, 31, 31, 31, 46, 47, 31, 31, 31, 31, 38, 39,
  60, 61, 58, 59, 56, 57, 52, 53, 50, 51, 48, 49,
  44, 45, 42, 43, 40, 41, 36, 37, 34, 35, 32, 33,
  -1, -1, -1, -1, 30, 31, -1, -1, -1, -1, 22, 23,
  -1, -1, -1, -1, 14, 15, -1, -1, -1, -1, 6, 7,
  28, 29, 26, 27, 24, 25, 20, 21, 18, 19, 16, 17,
  12, 13, 10, 11, 8, 9, 4, 5, 2, 3, 0, 1
};


// Called when universe registration completes.
void RegisterComplete(const ola::client::Result& result) {
  if (!result.Success()) {
    OLA_WARN << "Failed to register universe: " << result.Error();
  }
}


// map data to new channels and send frame
void map_channels(const ola::DmxBuffer &data) {
  for (
    size_t channel_output_index = 0;
    channel_output_index < channel_count;
    channel_output_index++
  ) {
    unsigned int map_value = map[channel_output_index];
    // check if map_value is in range of input channels
    if (map_value < data.Size()) {
      channels_out.SetChannel(
        channel_output_index,
        data.Get(map_value));
    }
  }
  // std::cout << "Send frame: " << std::endl << channels_out << std::endl;
  wrapper.GetClient()->SendDMX(
    UNIVERSE_OUT,
    channels_out,
    ola::client::SendDMXArgs());
}

// Called when new DMX data arrives.
void dmx_receive_frame(const ola::client::DMXMetadata &metadata,
  const ola::DmxBuffer &data) {
  // std::cout << "Received " << data.Size()
  // << " channels for universe " << metadata.universe
  // << ", priority " << static_cast<int>(metadata.priority)
  // << std::endl;
  map_channels(data);
}


int main() {
  ola::InitLogging(ola::OLA_LOG_INFO, ola::OLA_LOG_STDERR);
  // ola::client::OlaClientWrapper wrapper; now global.
  if (!wrapper.Setup())
  exit(1);
  ola::client::OlaClient *client = wrapper.GetClient();

  // clean channels_out buffer
  channels_out.Blackout();

  // Set the callback and register our interest in this universe
  client->SetDMXCallback(ola::NewCallback(&dmx_receive_frame));
  client->RegisterUniverse(
    UNIVERSE_IN,
    ola::client::REGISTER,
    ola::NewSingleCallback(&RegisterComplete));
  std::cout << "map incoming channels." << std::endl;
  wrapper.GetSelectServer()->Run();
}
