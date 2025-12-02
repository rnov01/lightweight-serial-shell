#pragma once

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "pin.h"

/*! \name USART Initializers 
 *  \details Configuration options used with \ref USART::init() \n 
 * @{
 */
/*! \enum rx_int_mode
 *  \brief Enable or disable the Rx interrupt 
 *  \details  */
enum class rx_int_mode : uint8_t { dis = 0, en = 1 };
/*! \enum tx_int_mode
 *  \brief Enable or disable the Tx interrupt 
 *  \details  */
enum class tx_int_mode : uint8_t { dis = 0, en = 1 };
/*! \enum data_int_mode
 *  \brief Enable or disable the UDR data interrupt 
 *  \details  */
enum class data_int_mode : uint8_t { dis = 0, en = 1 };
/*! \enum rx_mode
 *  \brief Enable or disable the USART receiver
 *  \details  */
enum class rx_mode : uint8_t { dis = 0, en = 1 };
/*! \enum tx_mode
 *  \brief Enable or disable the USART transmitter
 *  \details  */
enum class tx_mode : uint8_t { dis = 0, en = 1 };
/*! \enum spd_mode
 *  \brief Select USART normal or double-speed mode
 *  \details  */
enum class spd_mode : uint8_t { norm, doub };
///@}

/*! \class USART 
 *  \brief Interface for AVR USART operation
 *  \details This class encapsulates register-level access and provides functions for
 *  initialization, transmission and reception. USART objects are static and never destroyed.
 */
class USART {

    public:

    /*! \fn USART
        \brief Construct a USART module instance
     *  \param tx_x Tx pin defined in \link pin.h \endlink (const struct pin)
     *  \param rx_x Rx pin defined in \link pin.h \endlink (const struct pin)
     *  \param ucsrnA_addr Address of the UCSRnA register 
     *  \param ucsrnB_addr Address of the UCSRnB register  
     *  \param ubrrn_addr Address of the UBRRn register 
     *  \param udrn_addr Address of the UDRn register */
    USART(const pin& tx_x,
        const pin& rx_x,
        uint16_t ucsrnA_addr, 
        uint16_t ucsrnB_addr,
        uint16_t ubrrn_addr,
        uint16_t udrn_addr);

        /*! \fn void init(unsigned long baud_rate, rx_int_mode rxInt, tx_int_mode txInt, 
                data_int_mode datInt, rx_mode rxEn, tx_mode txEn, spd_mode speed)
         *  \brief Configures USART module parameters
         *  \param baud_rate Desired baud rate 
         *  \param rxInt Enable/disable rx interrupt with \link rx_int_mode enum \endlink 
         *  \param txInt Enable/disable tx interrupt with \link tx_int_mode enum \endlink 
         *  \param datInt Enable/disable UDR data interrupt with \link data_int_mode enum \endlink 
         *  \param rxEn Enable/disable rx interface with \link rx_mode enum \endlink 
         *  \param txEn Enable/disable tx interface with \link tx_mode enum \endlink 
         *  \param speed Select baud rate prescaler with \link spd_mode enum \endlink */
        void init(unsigned long baud_rate, rx_int_mode rxInt, tx_int_mode txInt, data_int_mode datInt,
                rx_mode rxEn,tx_mode txEn, spd_mode speed);
        
        /*! \fn ready(void)
         *  \brief Indicates if USART is ready to transmit
         *  \returns True if ready, False if not ready */
        bool ready(void);

        /*! \fn void write(const uint8_t data)
         *  \brief Writes one byte of binary data
         *  \param data One data byte to send 
         *  \warning This function blocks until data register is empty*/
        void write(uint8_t data);

        /*! \fn receive(void)
         *  \brief Returns one data byte from the data register
         *  \returns Data byte 
         *  \warning This function blocks until data is received */
        uint8_t receive(void);

        //void putc(char c);
        
        /*! \name PUTC definitions
         *  \brief These are defined as inline so they can be overloaded, taking in either a char or a uint8_t. 
         *   This ensures a proper unsigned value is transmitted. 
         *  @{ */
        /*! \fn inline void putc(char c)
         *  \brief Sends one data byte of char type by static casting to uint8_t
         *  \param c Data byte */
        inline void putc(char c) {write(static_cast<uint8_t>(c));}
        /*! \fn inline void putc(uint8_t c)
         *  \brief Sends one data byte of uint8_t type with \ref write()
         *  \param c Data byte */
        inline void putc(uint8_t c) {write(c);}
        ///@}

         /*! \fn void put_string(char str[], uint8_t STR_LEN)
         *  \brief Transmits a string stored in an array
         *  \details Use this function with a dynamic string buffer
         *  \note Buffer will be automatically flushed after it is transmitted
         *  \param str Array-stored string
         */
        void put_string(const char* str);

        /*! \fn void put_stringP(PGM_P str)
         *  \brief Transmits a string literal 
         *  \details Use this function with string literals. Wrap the literal in PSTR() to guarantee flash storage
         *  \param str String literal wrapped in PSTR()
         */
        void put_stringP(PGM_P str);
        
    private:
        const pin& tx_pin;
        const pin& rx_pin;
        volatile uint8_t* const UCSRnA;
        volatile uint8_t* const UCSRnB;
        //volatile uint8_t* const UCSRnC;
        //volatile uint8_t* const UBRRnH;
        volatile uint16_t* const UBRRn;
        volatile uint8_t* const UDRn;
        volatile uint8_t BAUD_prescaler = 16;
};

inline void clear_buffer(char* str) { while(*str) {*str++ = '\0';}}
extern USART USART0;
extern USART USART3;

void putc_wrapper(char c, void* self);
void puts_wrapper(const char* c, void* self);
void putsP_wrapper(const char* c, void* self);