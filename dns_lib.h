#ifndef DNS_LIB_H
#define DNS_LIB_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <regex.h> // TODO: if not used delete

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define MAX_ARGC 9
#define MAX_PORT_NUMBER 65535
#define PORT_DEFAULT_VALUE 53

#define h_addr h_addr_list[0] /* for backward compatibility */

#define MAX_PACKET_LENGTH  512// max udp packet lentgh

#define IS_NAME_POINTER(x) (((u_int16_t)x & 0b00000011)==0b11)
#define NAME_POINTER_VALUE(x) (x & 0b11111100)
#define TIMEOUT_LENGTH 5 // timeout length in seconds

//value for QR field
enum flags_QR_values {
    QR_QUESTION,
    QR_ANSWER
};

//value for OPCODE field
enum flags_OPCODE_values{
    OPCODE_STANDARD,
    OPCODE_INVERSE,
    OPCODE_SERVERSTATUS
};

//value for RCODE field
enum flags_RCODE_values{
    RCODE_OK,
    RCODE_FORMAT_ERROR,
    RCODE_SERVER_FAILURE,
    RCODE_NOT_FOUND,
    RCODE_NOT_IMPLEMENTED,
    RCODE_REFUSED
};

//value for TYPE field
enum dns_TYPE_values{
    TYPE_A=1,
    TYPE_NS=2,
    TYPE_CNAME=5,
    TYPE_SOA=6,
    TYPE_PTR=12,
    TYPE_HINFO=13,
    TYPE_MX=15,
    TYPE_TXT=16,
    TYPE_AAAA=28
};

//value for CLASS field
enum dns_CLASS_value{
    CLASS_IN=1,
};

//structure representing pointer in the domain name
struct name_pointer{
    unsigned int value: 14;
    unsigned int flag: 2;
};

//structure representing header of the DNS message
struct dns_header{
    uint16_t id; // random number for question and is same for reply

    unsigned int RD:1; // recursion desired
    unsigned int TC:1; // if udp was truncatanated
    unsigned int AA : 1; // valid in responses, authority for the domain name in question section
    unsigned int OPCODE : 4; // kind of query 0-standard, 1-inverse, 2-serverstatus,
    unsigned int QR:1; // 0 is query, 1 is response

    unsigned int RCODE:4; // for responses, 0-ok, 1-format error, 2-server failure, 3-domain does not exist, 4-not implemented-server doesnt support this query, 5-refused
    unsigned int Z:3; // should be all zeros
    unsigned int RA:1; // recursion available


    uint16_t qdcount; // number of entries in the question section
    uint16_t ancount; // number of resource records in answer section
    uint16_t nscount; // number of name server resource records in the authority records section<???
    uint16_t arcount; // number of resource records in additional records section
};

//struture representing part of the question section right after the domain name field
struct dns_question_tailer{
    u_int16_t TYPE;
    u_int16_t CLASS;
};

//structure representing rr record
struct rr_record{
    char*name; // IS NOT RESOLVED while mapping!
    u_int16_t * TYPE;
    u_int16_t * CLASS;
    int32_t * TTL;
    u_int16_t * rd_length;
    void* r_data;
};

//structure representing whole dns response
struct dns_response{
    struct dns_header* header;
    char* q_name;
    struct dns_question_tailer* q_tail;
    u_int16_t record_count; //ready to be used, no need for  htons
    struct rr_record * record_arr;
};


//global variables for setting up query message
extern int recursion; // recursion desired
extern int inverse; // inverse query
extern int type_argument; //type of the query
extern char* server_arg; // name of the server
extern int server_port; // number of the server port
extern char* target_arg; // queried argument
extern int client_socket; //used socket for comunication
extern char* global_packet_buffer; // received dns response



/**
 * @brief Initialise struct dns_header structure
 * 
 * Sets struct dns_header attributes based on the global variables.
 * 
 * @param dns_header to be initialized structure
*/
void set_dns_header(struct dns_header* dns_header);

/**
 * @brief Get length of the label
 * 
 * Gets character length of the first label of the domain name.
 * 
 * @param str domain name
 * @return length of the label
*/
int get_label_length(char*str);

/**
 * @brief Get length of the domain name
 * 
 * Gets length of the whole domain name, in the case of pointer value, function of the pointer is ignored. Same function as strlen().
 * 
 * @param str domain name
 * @return length of the domain name
*/
int get_name_length(char*str);

/**
 * @brief Sets domain name in the format of dns.
 * 
 * Formats argument into format for dns domain name, where each label is preceded by its length and ends with zero byte.
 * This value is set to buffer at buffer_position and buffer_position is incremented by final length of the domain name.
 * @param buffer destination buffer
 * @param argument source domain name buffer
 * @param buffer_position current position of buffer
*/
void set_question_name(char*buffer, char*argument, int* buffer_position);

/**
 * @brief Create reversed ip string.
 * 
 * Reverses string containing ipv4 address as used for reverse query. Returned string must be later properly freed.
 * 
 * @param argument ipv4 string
 * @return pointer to allocated reversed ipv4 string
*/
char* reverse_ip_arg(char*argument);

/**
 * @brief Sets values for the dns question section.
 * 
 * Sets values for the question section of the dns message at the tart of the buffer argument
 * 
 * @param buffer start of the buffer where question section should be stored
 * @param argument queried name
 * @return length of the question section
*/
int set_dns_question_section(char*buffer, char*argument);

/**
 * @brief Test if character is start of the domain name pointer
 * 
 * @param p possible start of the pointer 
 * @return 1 if it is pointer, 0 if not
*/
int is_pointer(char*p);

/**
 * @brief Get pointer value.
 * 
 * Returns value of the pointer that tells us, to what byte of the message we should jump to.
 * 
 * @param p start byte of the pointer
 * @return value of the pointer
*/
u_int16_t get_pointer_value(char*p);

/**
 * @brief Creates and maps struct dns_response structue.
 * 
 * Allocates and maps dns_response structure on the buffer that contains dns response. returned value should be properly freed later on.
 * 
 * @param buffer original buffer that will be dns_response mapped onto
 * @return final already mapped dns_response structure
*/
struct dns_response * map_dns_response(char*buffer);

/**
 * @brief Prints values for dns_header structure.
 * 
 * @param header to be printed header structure
*/
void print_dns_header(struct dns_header * header);

/**
 * @brief Prints dns name.
 * 
 * @param name to be printed domain name
*/
void print_name(char*name);

/**
 * @brief Prints TYPE value.
 * 
 * Converts TYPE value to string and prints that to stdout.
 * 
 * @param type to be printed TYPE value
*/
void print_TYPE(u_int16_t type);

/**
 * @brief Prints CLASS value.
 * 
 * Converts CLASS value to string and prints that to stdout.
 * 
 * @param class_name to be printed CLASS value
*/
void print_CLASS(u_int16_t class_name);

/**
 * @brief Prints rr_record structure.
 * 
 * Prints content of the rr_record.
 * 
 * @param rr to be printed rr_record structure
*/
void print_rr_data(struct rr_record*rr);

/**
 * @brief Prints dns_response structure.
 * 
 * Prints content of the dns_response.
 * 
 * @param rr to be printed dns_response structure
*/
void print_dns_question(struct dns_response*r);

/**
 * @brief Prints rr_record structure.
 * 
 * Prints content of the rr_record.
 * 
 * @param rr to be printed rr_record structure
*/
void print_dns_resource(struct rr_record*rr);

/**
 * @brief Prints dns_response structure.
 * 
 * Prints content of the dns_response.
 * 
 * @param r to be printed dns_response structure
*/
void print_dns_response(struct dns_response * r);

/**
 * @brief Frees struct dns_response.
 * @param r to be freed dns_response structure
*/
void free_dns_response(struct dns_response* r);


/**
 * @brief Send dns query based on the arguments and global arguments
 * @param client_socket socket to send message to
 * @param address to send message to
 * @param address_size size of the address
*/
void send_dns_query(int client_socket,struct sockaddr *address, socklen_t address_size);

#endif