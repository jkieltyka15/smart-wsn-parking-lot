/**
* @brief: Contains the implementation of the SensorNode class.
* @file: sensornode.cpp
*
* @author: jkieltyka15
*/

// standard libraries
#include <Arduino.h>
#include <stdlib.h>
#include <Wire.h>
#include <Adafruit_VL6180X.h>
#include <nRF24L01.h>
#include <RF24.h>

// local libraries
#include <Log.h>
#include <Message.h>

// local dependencies
#include "sensornode.hpp"


// base station's node ID
#define BASE_STATION_ID 0

// special base station address since 0x00000000 is not a valid address
#define BASE_STATION_ADDRESS 0xBAD1DEA5

#define RF24_CHANNEL_SPACING 5  // number of channels between a valid node channel
#define RF24_READING_PIPE 1     // reading pipe for the NRF24L01

#define MAX_SEND_ATTEMPTS 15    // maximum number of attempts to send a message
#define FAILED_SEND_DELAY 15    // minimum delay between sending message attempts

// number of attempts to wait for the channel to be open if it is busy
#define CHANNEL_CHECKS_MAX  10

// minimum time to wait if the channel is busy before sending in milliseconds
#define CHANNEL_BUSY_DELAY_MIN_MS 25

// maximum time to wait if the channel is busy before sending in milliseconds
#define CHANNEL_BUSY_DELAY_MAX_MS 100


SensorNode::SensorNode(uint8_t node_id) {

    this->node_id = node_id;

    this->radio_address = this->calculate_radio_address(node_id);
    this->radio_channel = this->calculate_radio_channel(node_id);
}


uint32_t SensorNode::calculate_radio_address(uint8_t node_id) {

    // base station has special non-calculated address
    if (BASE_STATION_ID == node_id) {
        return BASE_STATION_ADDRESS;
    }

    // generate address as byte array
    const byte address_bytes[RF24_ADDRESS_WIDTH] = { node_id, node_id, node_id, node_id };

    // convert byte array to 32 bit integer
    uint32_t address = 0;
    memcpy(&address, address_bytes, sizeof(address));
    
    return address; 
}


uint8_t SensorNode::calculate_radio_channel(uint8_t node_id) {

    return node_id * RF24_CHANNEL_SPACING;
}


bool SensorNode::init() {

    // start ToF sensor
    if (false == sensor.begin()) {

        ERROR("Failed to start ToF sensor")
        return false;
    }

    // start radio
    if (false == radio.begin()) {

        ERROR("Failed to start radio")
        return false;
    }

    // configure radio
    radio.enableDynamicPayloads();
    radio.setAutoAck(true);
    radio.setRetries(FAILED_SEND_DELAY, MAX_SEND_ATTEMPTS);
    radio.setAddressWidth(RF24_ADDRESS_WIDTH);
    radio.setPALevel(RF24_PA_MAX);
    radio.setChannel(this->radio_channel);
    radio.openReadingPipe(RF24_READING_PIPE, this->radio_address);

    // start listening on radio
    radio.startListening();

    return true;
}


tof_sensor_status_t SensorNode::get_sensor_status() {

    return this->sensor_status;
}


bool SensorNode::is_sensor_status_changed() {

    // get range from sensor
    (void) this->sensor.readRange();
    uint8_t status = this->sensor.readRangeStatus();

    // sensor read occupied
    if (VL6180X_ERROR_NONE == status) {

        // status of parking spot did not change
        if (OCCUPIED == this->sensor_status) {
            return false;
        }

        // status of parking space changed
        INFO("parking space is now occupied")
        this->sensor_status = OCCUPIED;
    }

    // sensor read vacant
    else if (VL6180X_ERROR_NOCONVERGE == status) {
        
        // status of parking spot did not change
        if (VACANT == this->sensor_status) {
            return false;
        }

        // status of parking space changed
        INFO("parking space is now vacant")
        this->sensor_status = VACANT;
    }

    // sensor error occured
    else {
        WARN("ToF sensor read error");
        return false;
    }

    return true;
}


bool SensorNode::transmit_update(UpdateMessage* msg) {

    // calculate receiver node's radio configuration
    uint8_t rx_id = msg->get_rx_id();
    uint32_t rx_address = this->calculate_radio_address(rx_id);
    uint8_t rx_channel = this->calculate_radio_channel(rx_id);

    // switch to receiver node's channel
    this->radio.setChannel(rx_channel);

    // wait for there to be no traffic on receiver's channel or timeout occurs
    bool is_channel_open = false;
    for (uint8_t i = 0; i < CHANNEL_CHECKS_MAX; i++) {

        // check if channel is open
        is_channel_open = (false == this->radio.testCarrier());

        // channel is open
        if (true == is_channel_open) {
            break;
        }

        // delay a random amount of time to avoid collisions
        uint32_t channel_delay = random(CHANNEL_BUSY_DELAY_MIN_MS, CHANNEL_BUSY_DELAY_MAX_MS);
        INFO("Channel " + rx_channel + " is busy. Waiting " + channel_delay + " ms")
        delay(channel_delay);
    }

    // do not send message since channel has too much traffic
    if (false == is_channel_open) {

        // switch back to this node's radio configuration
        this->radio.setChannel(this->radio_channel);
        radio.openReadingPipe(RF24_READING_PIPE, this->radio_address);

        return false;
    }

    // stop listening
    this->radio.stopListening();
    this->radio.closeReadingPipe(RF24_READING_PIPE);

    // create pipe to receiver node
    radio.openWritingPipe(rx_address);

    // create a copy of the message to send
    uint8_t buffer[sizeof(*msg)];
    memcpy(buffer, msg, sizeof(buffer));

    // attempt to transmit message
    bool is_sent = this->radio.write(&buffer, sizeof(buffer));

    // switch back to this node's radio configuration
    this->radio.setChannel(this->radio_channel);
    radio.openReadingPipe(RF24_READING_PIPE, this->radio_address);

    // start listening again
    this->radio.startListening();

    return is_sent;
}


bool SensorNode::transmit_update(uint8_t rx_node_id) {

    // create update message
    bool is_vacant = (this->sensor_status == VACANT);
    UpdateMessage msg = UpdateMessage(rx_node_id, this->node_id, this->node_id, is_vacant);

    // attempt to transmit message
    return this->transmit_update(&msg);
}


bool SensorNode::is_message() {

    return this->radio.available();
}


bool SensorNode::read_message(uint8_t** buffer, uint8_t len) {

    if (false == this->radio.available()) {
        return false;
    }

    this->radio.read(buffer, len);
    return true;
}


uint8_t SensorNode::get_id() {

    return this->node_id;
}
