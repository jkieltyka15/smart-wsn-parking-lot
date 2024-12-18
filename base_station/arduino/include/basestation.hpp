/**
* @brief: Contains the prototype of the BaseStation class.
* @file: basestation.hpp
*
* @author: jkieltyka15
*/

#ifndef _BASE_STATION_HPP_
#define _BASE_STATION_HPP_

// standard libraries
#include <Arduino.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>

// local libraries
#include <Message.h>

#define SENSOR_NODE_NUM 10  // number of sensor nodes

#define RF24_CE_PIN 6   // NRF24L01 CE pin assignment
#define RF24_CSN_PIN 8  // NRF24L01 CSN pin assignment

// width in bytes of the radio's address
#define RF24_ADDRESS_WIDTH 4


class BaseStation {

    private:

        // unique id of the base station
        uint8_t node_id = 0;

        // array to track sensor node statuses
        bool node_status[SENSOR_NODE_NUM] = {0};

        // NRF24L01 transciever radio
        RF24 radio = RF24(RF24_CE_PIN, RF24_CSN_PIN);
        uint32_t radio_address = 0;
        uint8_t radio_channel = 0;

        /**
         * @brief Calculates a given node's radio address based on the node ID
         * 
         * @param node_id: ID of node to calculate address for
         * @return The calculated radio address for the node
         */
        uint32_t calculate_radio_address(uint8_t node_id);

        /**
         * @brief Calculates a given node's radio channel based on the node ID
         * 
         * @param node_id: ID of node to calculate channel for
         * @return The calculated radio channel for the node (0-125)
         */
        uint8_t calculate_radio_channel(uint8_t node_id);


    public:

        /**
         * @brief Constructs a BaseStation object
         */
        BaseStation(uint8_t node_id);

        /**
         * @brief Initializes all variables and objects
         * 
         * Initializes all variables and objects for a BaseStation
         * object including the ToF sensor and transciever.
         * 
         * @return true on success. Otherwise false
         */
        bool init();

        /**
         * @brief Determine if there is a message available to read
         * 
         * @return True if a message is available. Otherwise false
         */
        bool is_message();

        /**
         * @brief Gets a message from the message queue
         * 
         * @param buffer: buffer to hold message
         * @param size: size of buffer
         * @return Number of bytes read
         */
        bool read_message(uint8_t** buffer, uint8_t size);

        /**
         * @brief Get the ID of the node
         * 
         * @return ID of the node
         */
        uint8_t get_id();

        /**
         * @brief Determines if the provided node ID is valid
         * 
         * @param node_id: node ID to be evaluated
         * @return True if node ID is valid. Otherwise false
         */
        bool is_valid_sensor_node(uint8_t node_id);

        /**
         * @brief Update the vacancy status of a node
         * 
         * @param node_id: ID of node to update vacancy status
         * @param is_vacant: New vacancy status of node
         * @return True if successfully updated. Otherwise false
         */
        bool update_node_status(uint8_t node_id, bool is_vacant);

        /**
         * @brief Get the vacancy status of a node
         * 
         * @param node_id: ID of node to get vacancy status
         * @return The node's vacancy status
         */
        bool get_node_status(uint8_t node_id);

        /**
         * @brief Counts the number of nodes with vacant status
         * 
         * @return Number of nodes with vacant status
         */
        uint8_t num_vacant();
};

#endif /* _BASE_STATION_HPP_ */
