// based on example from
// http://docs.openlighting.org/ola/doc/latest/dmx_cpp_client_tutorial.html

// build for local:
// g++ -std=c++11 olamapper.cpp -o olamapper.out $(pkg-config --cflags --libs libola)
// build for aria board:
// only possible in build env.
// copy to board:
// scp olamapper.aria light@arai.loca:/home/light/OLA_channel_mapper_cpp


#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>

#include <string>
#include <iostream>
#include <fstream>

static const unsigned int UNIVERSE_IN = 1;
static const unsigned int UNIVERSE_OUT = 2;

ola::client::OlaClientWrapper wrapper;
ola::DmxBuffer channels_out;

static const unsigned int channel_count = 240;
int my_map[channel_count];
// int my_map[];
// int my_map[channel_count] = {
//   -1, -1, -1, -1, 158, 159, -1, -1, -1, -1, 150, 151,
//   -1, -1, -1, -1, 142, 143, -1, -1, -1, -1, 134, 135,
//   156, 157, 154, 155, 152, 153, 148, 149, 146, 147, 144, 145,
//   140, 141, 138, 139, 136, 137, 132, 133, 130, 131, 128, 129,
//   -1, -1, -1, -1, 126, 127, -1, -1, -1, -1, 118, 119,
//   -1, -1, -1, -1, 110, 111, -1, -1, -1, -1, 102, 103,
//   124, 125, 122, 123, 120, 121, 116, 117, 114, 115, 112, 113,
//   108, 109, 106, 107, 104, 105, 100, 101, 98, 99, 96, 97,
//   -1, -1, -1, -1, 94, 95, -1, -1, -1, -1, 86, 87,
//   -1, -1, -1, -1, 78, 79, -1, -1, -1, -1, 70, 71,
//   92, 93, 90, 91, 88, 89, 84, 85, 82, 83, 80, 81,
//   76, 77, 74, 75, 72, 73, 68, 69, 66, 67, 64, 65,
//   -1, -1, -1, -1, 62, 63, -1, -1, -1, -1, 54, 55,
//   -1, -1, -1, -1, 46, 47, -1, -1, -1, -1, 38, 39,
//   60, 61, 58, 59, 56, 57, 52, 53, 50, 51, 48, 49,
//   44, 45, 42, 43, 40, 41, 36, 37, 34, 35, 32, 33,
//   -1, -1, -1, -1, 30, 31, -1, -1, -1, -1, 22, 23,
//   -1, -1, -1, -1, 14, 15, -1, -1, -1, -1, 6, 7,
//   28, 29, 26, 27, 24, 25, 20, 21, 18, 19, 16, 17,
//   12, 13, 10, 11, 8, 9, 4, 5, 2, 3, 0, 1
// };


// void read_map_from_file(std::string filename) {
void read_map_from_file() {
  // std::string filename = "my_map.config";
  std::ifstream myfile("map.conf");
  if (myfile.is_open()) {
    std::string line;
    std::string raw_input;
    // read in all lines and combine theme.
    while ( std::getline(myfile, line) ) {
      std::cout << line << '\n';
      raw_input += line;
    }
    myfile.close();
    // now we can parse the input
    // std::cout << "raw_input: '" << raw_input << "'" << std::endl;
    // get part between []
    size_t start = raw_input.find('[');
    size_t end = raw_input.find(']');
    raw_input = raw_input.substr(start+1, (end-1)-start);
    // std::cout << "raw_input: '" << raw_input << "'" << std::endl;
    // split at every ', '
    start = 0;
    end = 0;
    size_t map_index = 0;
    while ( (end = raw_input.find(", ", start)) != std::string::npos ) {
      std::string element = raw_input.substr(start, end - start);
      start = end + 1;
      // std::cout << element << std::endl;
      // now we can convert the element string to a int
      int value;
      value = std::stoi(element);
      my_map[map_index] = value;
      map_index += 1;
      // std::cout << value << std::endl;
    }
    // handle last element
    std::string element = raw_input.substr(start);
    int value;
    value = std::stoi(element);
    my_map[map_index] = value;
    map_index += 1;
    // std::cout << value << std::endl;
    // std::cout << map_index << std::endl;
    // std::cout << my_map << std::endl;
  } else {
    std::cout << "Unable to open file."<< std::endl;
  }
}

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
    unsigned int map_value = my_map[channel_output_index];
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
  // read map from file:
  // read_map_from_file("my_map.config");
  read_map_from_file();

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
