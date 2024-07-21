#include "./dns_lib.h"



void print_help(){
    printf(
    " ===========================\n"
    "     DNS PROGRAM - HELP\n"
    " ===========================\n"
    "Usage: dns [-r] [type] -s server [-p port] address\n"
    "\n"
    "-r: Recursion Desired, else without recursion.\n"
    "type: specifies type of the query, by default A type record is queried\n"
    "    -6 for AAAA,\n"
    "    -x for reverse IPv4 query\n"
    "    -mx for MX\n"
    "    -cname for CNAME\n"
    "    -ns for NS\n"
    "    -txt for TXT\n"
    "    -hinfo for HINFO\n"
    "-s: IP address or domain name of server where should be query send.\n"
    "-p port: Destination port number, default is 53\n"
    "address: Queried address.\n"
    );
    fprintf(stderr,"INVALID PROGRAM ARGUMENTS\n");
    exit(0);
}


// for parsing values into global variables representing arguments
void parse_arguments(int argc, char**argv){

    inverse = 0;
    target_arg = NULL;
    server_arg = NULL;
    server_port = PORT_DEFAULT_VALUE; 
    type_argument = TYPE_A;

    for(int i = 1; i<argc;i++){
        if(strcmp(argv[i],"-r")==0){
            recursion = 1;
        }
        else if(strcmp(argv[i],"-x")==0){
            inverse = 1;
            type_argument = TYPE_PTR;
        }
        else if(strcmp(argv[i],"-6")==0){
            type_argument = TYPE_AAAA;
        }
        else if(strcmp(argv[i],"-mx")==0){
            type_argument = TYPE_MX;
        }
        else if(strcmp(argv[i],"-cname")==0){
            type_argument = TYPE_CNAME;
        }
        else if(strcmp(argv[i],"-ns")==0){
            type_argument = TYPE_NS;
        }
        else if(strcmp(argv[i],"-txt")==0){
            type_argument = TYPE_TXT;
        }
        else if(strcmp(argv[i],"-hinfo")==0){
            type_argument = TYPE_HINFO;
        }

        else if(strcmp(argv[i],"-s")==0){
            i++;
            if(i==argc){
                print_help();
            }
            server_arg = argv[i];
        }
        else if(strcmp(argv[i],"-p")==0){
            i++;
            if(i==argc || argv[i][0]=='-'){
                print_help();
            }
            server_port = atoi(argv[i]);
        }
        else{
            if(target_arg!=NULL){
                print_help();
            }
            target_arg = argv[i];
        }
    }

    //list of error conditions
    if(
        server_arg == NULL ||
        target_arg == NULL ||
        strlen(target_arg) > 256 ||
        target_arg[0]=='-' || 
        server_arg[0]=='-' ||
        server_port>MAX_PORT_NUMBER
    ){
        print_help();
    }
}

int main(int argc, char**argv){
    
    parse_arguments(argc, argv);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket <= 0 ){
        perror("ERROR: socket\n");
        exit(EXIT_FAILURE);
    }


    //GET DNS SERVER ADDRESS
    struct hostent *server = NULL;
    server = gethostbyname(server_arg);
    
    if(server == NULL){
        fprintf(stderr, "ERROR: no such host\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); 

    server_address.sin_family = server->h_addrtype;
    server_address.sin_port = htons(server_port); 
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    //ADDRESS SETUP
    struct sockaddr *address = (struct sockaddr *) &server_address;
    socklen_t address_size = sizeof(server_address);

    //timeout setup
    struct timeval socket_timeout_interval= {5 , 0}; //set timeout for 2 seconds
    setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&socket_timeout_interval,sizeof(struct timeval));
    
    //send dns query
    send_dns_query(client_socket, address, address_size);

    close(client_socket);
}
