// Copyright (c) 2016 Stefan Krüger
// released under MIT (see LICENSE for details)

// based on example from
// http://docs.openlighting.org/ola/doc/latest/dmx_cpp_client_tutorial.html

// build for local:
// g++ -std=c++11 olamapper.cpp -o olamapper.out $(pkg-config --cflags --libs libola)



#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/ClientWrapper.h>

#include <string>
#include <iostream>
#include <fstream>

unsigned int universe_in = 1;
unsigned int universe_out = 2;
unsigned int channel_count = 240;
static const unsigned int channel_count_MAX = 512;

ola::client::OlaClientWrapper wrapper;
ola::DmxBuffer channels_out;

int my_map[channel_count_MAX];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// configuration things

void parse_config_channels(std::string raw_channels) {
  // get part between []
  size_t start = raw_channels.find('[');
  size_t end = raw_channels.find(']');
  raw_channels = raw_channels.substr(start+1, (end-1)-start);
  // std::cout << "raw_channels: '" << raw_channels << "'" << std::endl;
  // split at every ', '
  start = 0;
  end = 0;
  size_t map_index = 0;
  while ( (end = raw_channels.find(", ", start)) != std::string::npos ) {
    std::string element = raw_channels.substr(start, end - start);
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
  std::string element = raw_channels.substr(start);
  int value;
  value = std::stoi(element);
  my_map[map_index] = value;
  map_index += 1;
  // std::cout << value << std::endl;
  // std::cout << map_index << std::endl;
  // std::cout << my_map << std::endl;
}

void parse_config_map(std::string raw_input) {
  // find "map": { "channels": [....]}
  size_t start = std::string::npos;
  start = raw_input.find("\"map\"");
  // std::cout << "map start: '" << start << "'" << std::endl;
  if (start != std::string::npos) {
    // std::cout << "map found." << std::endl;
    size_t start_channels = raw_input.find("\"channels\"", start);
    // std::cout << "channels start: '" << start_channels << "'" << std::endl;
    if (start_channels != std::string::npos) {
      // std::cout << "channels found." << std::endl;
      size_t end = raw_input.find('}', start_channels);
      if (end != std::string::npos) {
        // std::cout << "channels end found." << std::endl;
        std::string channels_list = raw_input.substr(start+1, (end-1)-start);
        // std::cout << "channels_list: '" << channels_list << "'" << std::endl;
        parse_config_channels(channels_list);
      } else {
        std::cout
          << "Error in Config Format: 'channels' section end not found."
          << std::endl;
      }
    } else {
      std::cout
        << "Error in Config Format: 'channels' section not found."
        << std::endl;
    }
  } else {
    std::cout
      << "Error in Config Format: 'map' section not found."
      << std::endl;
  }
}

int parse_value(std::string raw_input, std::string key_name) {
  size_t start = std::string::npos;
  size_t end = std::string::npos;
  std::string value_raw;
  int value = -1;

  start = raw_input.find("\"" + key_name + "\"");
  // std::cout << "start: '" << start << "'" << std::endl;
  if (start != std::string::npos) {
    end = raw_input.find_first_of(",}", start);
    if (end != std::string::npos) {
      // start + " + key_name + " + :
      size_t value_content_start = start + 1 + key_name.length() + 1 + 1;
      value_raw = raw_input.substr(
        value_content_start,
        (end)-value_content_start);
      // std::cout << "value_raw: '" << value_raw << "'" << std::endl;
      // now we can convert the element string to a int
      value = std::stoi(value_raw);
    } else {
      std::cout
        << "Error in Config Format: '"
        << key_name
        << "' value end not found."
        << std::endl;
    }
  } else {
    std::cout
      << "Error in Config Format: '"
      << key_name
      << "' value not found."
      << std::endl;
  }

  return value;
}

void parse_config_universe(std::string raw_input) {
  // find "universe": { }
  // std::cout << "raw_input: '" << raw_input << "'" << std::endl;
  std::string key_name = "universe";
  size_t start_section = raw_input.find("\""+key_name+"\"");
  // std::cout << "start_section: '" << start_section << "'" << std::endl;
  if (start_section != std::string::npos) {
    size_t section_content_start = start_section + 1 + key_name.length();
    std::string section_content = raw_input.substr(
      section_content_start);
    // find 'channel_count'
    int channel_count = parse_value(section_content, "channel_count");
    // find 'input'
    int input = parse_value(section_content, "input");
    // find 'output'
    int output = parse_value(section_content, "output");
    // done
    std::cout << "channel_count: " << channel_count << std::endl;
    std::cout << "input: " << input << std::endl;
    std::cout << "output: " << output << std::endl;
  } else {
    std::cout
      << "Error in Config Format: 'universe' section not found."
      << std::endl;
  }
}

// void read_config_from_file(std::string filename) {
void read_config_from_file() {
  // std::string filename = "my_map.config";
  std::ifstream myfile("map.conf");
  if (myfile.is_open()) {
    std::string line;
    std::string raw_input;
    // read in all lines and combine theme.
    while ( std::getline(myfile, line) ) {
      std::cout << line << std::endl;
      raw_input += line;
    }
    myfile.close();
    std::cout << std::endl;
    std::cout << std::endl;

    // ------------------------------------------
    // now we can parse the input
    // std::cout << "raw_input: '" << raw_input << "'" << std::endl;

    // search and extract map
    std::cout << "parse_config_map.."<< std::endl;
    parse_config_map(raw_input);

    // search and extract universe information
    std::cout << "parse_config_universe.."<< std::endl;
    parse_config_universe(raw_input);

    std::cout << "parsing done."<< std::endl;

  } else {
    std::cout << "Unable to open file."<< std::endl;
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// mapping

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
    universe_out,
    channels_out,
    ola::client::SendDMXArgs());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ola things

// Called when universe registration completes.
void RegisterComplete(const ola::client::Result& result) {
  if (!result.Success()) {
    OLA_WARN << "Failed to register universe: " << result.Error();
  }
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
  // read_config_from_file("my_map.config");
  read_config_from_file();

  // ola::client::OlaClientWrapper wrapper; now global.
  if (!wrapper.Setup())
  exit(1);
  ola::client::OlaClient *client = wrapper.GetClient();

  // clean channels_out buffer
  channels_out.Blackout();

  // Set the callback and register our interest in this universe
  client->SetDMXCallback(ola::NewCallback(&dmx_receive_frame));
  client->RegisterUniverse(
    universe_in,
    ola::client::REGISTER,
    ola::NewSingleCallback(&RegisterComplete));
  std::cout << "map incoming channels." << std::endl;
  wrapper.GetSelectServer()->Run();
}
