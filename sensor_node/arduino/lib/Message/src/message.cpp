/**
* @brief: Contains the implementation of the Message class.
* @file: message.cpp
*
* @author: jkieltyka15
*/

// standard libraries
#include <Arduino.h>

// local dependencies
#include "message.hpp"


Message::Message(uint8_t rx_id, uint8_t tx_id, message_t msg_type) {

    this->rx_id = rx_id;
    this->tx_id = tx_id;
    this->msg_type = msg_type;
}


uint8_t Message::get_rx_id() {

    return this->rx_id;
}


uint8_t Message::get_tx_id() {

    return this->tx_id;
}


message_t Message::get_type() {

    return this->get_type();
}
