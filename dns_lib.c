#include "./dns_lib.h"


int recursion;
int inverse;
int type_argument;
char* server_arg;
int server_port;
char* target_arg;

int client_socket;

char* global_packet_buffer; // that dns message



//sets dns header attributes for question
void set_dns_header(struct dns_header* dns_header){
    dns_header->id=(uint16_t)getpid();
    dns_header->qdcount=htons(1);
    dns_header->ancount=0;
    dns_header->nscount=0;
    dns_header->arcount=0;

    dns_header->QR=QR_QUESTION;
    dns_header->OPCODE=OPCODE_STANDARD;
    dns_header->AA=0;
    dns_header->TC=0;
    dns_header->RD=recursion;
    dns_header->RA=0;
    dns_header->Z=0;
    dns_header->RCODE=0;
    return;
}
//returns length of the label from the dns name argument
int get_label_length(char*str){
    for(int i = 0; i <= strlen(str);i++){
        if(str[i]=='\0' || str[i]=='.'){

            return i;
        }
    }
    return 0;
}

//returns length of the name field -- strlen could be used but too late
int get_name_length(char*str){
    int counter = 0; 
    for(;str[counter]!='\0';counter++);
    return ++counter; // it always contains zero bit
}
//sets question name field into to be sent buffer based on the argument value
void set_question_name(char*buffer, char*argument, int * buffer_position){
    int i = 0;
        while(i < strlen(argument)){
        if((*buffer_position) == 0){
            buffer[*buffer_position]=get_label_length(argument);
            (*buffer_position)++;
        }
        else if(argument[i]=='.'){
            buffer[*buffer_position]=get_label_length(argument+i+1);
            i++;
            (*buffer_position)++;
        }
        else{
            buffer[*buffer_position]=argument[i];
            i++;
            (*buffer_position)++;
        }
    }
    buffer[*buffer_position]='\0';
}

//returns ip address in the format to be used in the reverse query - ip address is reversed and concatanated with in-addr.arpa
char* reverse_ip_arg(char*argument){

    
    int len = strlen(argument);
    char *tmp_buffer = calloc(len+strlen(".IN-ADDR.ARPA")+1,1);
    int buffer_pos=0;
    char*current_label=NULL;
    for(int i = len-1;i>0;i--){
        //printf("%c",argument[i]);
        if(argument[i]=='.'){
            current_label = argument+i+1;
            strcpy(tmp_buffer+buffer_pos,current_label);
            buffer_pos+=strlen(current_label);
            tmp_buffer[buffer_pos++]='.';
            argument[i]='\0';
        }
    }

    strcpy(tmp_buffer+buffer_pos,argument); // copy last label

    strcpy(tmp_buffer+len,".IN-ADDR.ARPA");

    return tmp_buffer;
}

//sets dns question section
int set_dns_question_section(char*buffer, char*argument){
    
    int buffer_position = 0;


    if(inverse==0){
        set_question_name(buffer, argument, &buffer_position);
    }
    else{
        char*inaddr_arpa_arg = reverse_ip_arg(argument);
        set_question_name(buffer, inaddr_arpa_arg, &buffer_position);
        free(inaddr_arpa_arg);
    }


    //set flags on the end of the question part
    buffer_position++;
    u_int16_t buff;
        
    buff = htons(type_argument);

    bcopy(&buff,buffer+buffer_position,2);
    buffer_position+=2;
    buff =  htons((uint16_t)CLASS_IN);
    bcopy(&buff,buffer+buffer_position,2);
    buffer_position+=2;
    return buffer_position;
}

//print number in reverse binary format - for debugging
void print_binary(u_int16_t n){
    printf("BINARY:");
    
    while (n) {
    if (n & 1)
        printf("1");
    else
        printf("0");

    n >>= 1;
}
printf("\n");
}

//checks if the string is the name pointer or not
int is_pointer(char * p){
    u_int16_t pointer = htons(*((u_int16_t*)p));
    struct name_pointer * a = (struct name_pointer*)(&pointer);
    return a->flag == 0b11;
}

//returns offset of the pointer
u_int16_t get_pointer_value(char * p){
    u_int16_t pointer = htons(*((u_int16_t*)p));
    struct name_pointer * a = (struct name_pointer*)(&pointer);
    return a->value;
}


//maps dns_response onto the buffer, doesnt map name correctly - name can still contain pointers
struct dns_response * map_dns_response(char*buffer){
     

    global_packet_buffer = buffer; //setting up global buffer pointer


    char*current_position = buffer;
    struct dns_response * dns_r = malloc(sizeof(struct dns_response));
    
    //map header

    dns_r->header = (struct dns_header*)current_position;
    current_position+=sizeof(struct dns_header);
    
    //map question
    dns_r->q_name = current_position;
    current_position+= get_name_length(current_position);

    dns_r->q_tail = (struct dns_question_tailer*)current_position;
    current_position+=sizeof(struct dns_question_tailer);

    //get total RR record count
    dns_r->record_count = htons(dns_r->header->ancount) + htons(dns_r->header->nscount) + htons(dns_r->header->arcount);
    dns_r->record_arr = malloc(sizeof(struct rr_record) * ntohs(dns_r->record_count));
 
    //map all RR records

    struct rr_record * current_record;

    for(int i = 0; i < dns_r->record_count;i++){
         
        current_record = &(dns_r->record_arr[i]);

        current_record->name = current_position;
        
        //resolve name pointers
        if(is_pointer(current_record->name)){
            current_position+=sizeof(u_int16_t); // we only skip pointer to the string and can continue with next byte
        }
        else{
            current_position+=get_name_length(current_record->name);
        }
                 
        //current_record->name = evaluate_dns_name(current_record->name, buffer);

        //map other fields

        dns_r->record_arr[i].TYPE = (u_int16_t *)current_position;
        current_position +=sizeof(u_int16_t);
 
        dns_r->record_arr[i].CLASS = (u_int16_t *)current_position;
        current_position +=sizeof(u_int16_t);
 

        dns_r->record_arr[i].TTL = (int32_t *)current_position;
        current_position +=sizeof(int32_t);
 

        dns_r->record_arr[i].rd_length = (u_int16_t *)current_position;
        current_position +=sizeof(u_int16_t);
 

        dns_r->record_arr[i].r_data = (void*)current_position;
        current_position +=htons(*(dns_r->record_arr[i].rd_length));
        //continues with next reponse record
    }
 
    return dns_r;
}

//prints dns_header info
void print_dns_header(struct dns_header * header){
    printf("Authoritative: %s, ",header->AA?"Yes":"No");
    printf("Recursive: %s, ",header->RA&&header->RD?"Yes":"No");
    printf("Truncated: %s, ",header->TC?"Yes":"No");
        
    switch (header->RCODE)
    {
    case RCODE_OK:
        fprintf(stderr,"MESSAGE OK\n");
        break;
    case RCODE_FORMAT_ERROR:
        fprintf(stderr,"ERROR: FORMAT ERROR\n");
        break;
    case RCODE_SERVER_FAILURE:
        fprintf(stderr,"ERROR: SERVER FAILURE\n");
        break;
    case RCODE_NOT_FOUND:
        fprintf(stderr,"ERROR: ADRESS DOES NOT EXIST\n");
        break;
    case RCODE_NOT_IMPLEMENTED:
        fprintf(stderr,"ERROR: NOT IMPLEMENTED\n");
        break;
    case RCODE_REFUSED:
        fprintf(stderr,"ERROR: RCODE_REFUSED\n");
        break;
    default:
        break;
    }
    return;
}
//prints dns nameprint_name
void print_name(char*name){
    int cur_label_length = 0;
    char * current_position = name;
    if(current_position[0]=='\0'){
        printf("<ROOT>");
        return;
    }
    while(current_position[0]!='\0'){
        if(is_pointer(current_position)){
            current_position = global_packet_buffer + get_pointer_value(current_position);
            continue;
        }

        cur_label_length = current_position[0];
        current_position++;
        for(;cur_label_length>0;cur_label_length--){
            putchar(current_position[0]);
            current_position++;
        }
        putchar('.');
    }
}

//prints TYPE field - value must be already in host format
void print_TYPE(u_int16_t type){
    switch ((type))
    {
    case TYPE_A:
        printf("A");
        break;
    case TYPE_NS:
        printf("NS");
        break;
    case TYPE_CNAME:
        printf("CNAME");
        break;
    case TYPE_SOA:
        printf("SOA");
        break;
    case TYPE_PTR:
        printf("PTR");
        break;
    case TYPE_HINFO:
        printf("HINFO");
        break;
    case TYPE_MX:
        printf("MX");
        break;
    case TYPE_TXT:
        printf("TXT");
        break;
    case TYPE_AAAA:
        printf("AAAA");
        break;
    default:
        printf("NOT IMPLEMENTED");
        break;
    }
    printf(", ");
}

//prints CLASS field - value must be already in host format
void print_CLASS(u_int16_t class_name){
    switch ((class_name))
    {
    case CLASS_IN:
        printf("IN");
        break;
    
    default:
        printf("UNKNOWN");
        break;
    }
    printf(", ");
}

//print character string
//string_count = -1 => all
int print_character_string(char*buffer,u_int16_t total_length,int string_count){
    int i;
    int len;
    for(i = 0,len = buffer[i];i<total_length;i++){
        if(len == 0){
            if(string_count == 0){
                break;
            }
            putchar(' ');
            string_count--;
            len = buffer[i];
            continue;
        }
        else{
            len--;
            putchar(buffer[i]);
        }
    }
    return i;
}

//prints RR record's fields
void print_rr_data(struct rr_record*rr){
    char str_ip6[INET6_ADDRSTRLEN+1] = {};
    char str_ip4[INET_ADDRSTRLEN+1] = {};

    switch (ntohs(*(rr->TYPE)))
    {
    case TYPE_A:
        inet_ntop(AF_INET, rr->r_data, str_ip4, INET_ADDRSTRLEN);
        printf("%s",str_ip4);
        break;
    case TYPE_NS:
        print_name(rr->r_data);
        break;
    case TYPE_CNAME:
        print_name(rr->r_data);
        break;
    case TYPE_SOA:
        printf("NOT IMPLEMENTED");
        break;
    case TYPE_PTR:
        print_name(rr->r_data);
        break;
    case TYPE_HINFO:
        ;int jump;
        printf("CPU:");
        jump = print_character_string((char*)(rr->r_data),ntohs(*rr->rd_length),1);
        printf(", OS:");
        print_character_string((char*)(rr->r_data)+jump,ntohs(*rr->rd_length)-jump,1);
    case TYPE_MX:
        printf("preference: %u",ntohs(*((u_int16_t*)(rr->r_data))));
        print_name((char*)rr->r_data+sizeof(uint16_t));
        break;
    case TYPE_TXT:
        print_character_string((char*)(rr->r_data),ntohs(*rr->rd_length),-1);
    case TYPE_AAAA:
        inet_ntop(AF_INET6, rr->r_data, str_ip6, INET6_ADDRSTRLEN);
        printf("%s",str_ip6);
        break;
    default:
        printf("NOT IMPLEMENTED");
        break;
    }
}

//prints info about dns_response question part
void print_dns_question(struct dns_response*r){
    print_name(r->q_name);
    printf(" ");
    print_TYPE(htons(r->q_tail->TYPE));
    print_CLASS(htons(r->q_tail->CLASS));
    printf("\n");
}
//prints info about values of struct rr_record
void print_dns_resource(struct rr_record*rr){

    print_name(rr->name);
    printf(" ");
    print_TYPE(htons(*(rr->TYPE)));
    print_CLASS(htons(*(rr->CLASS)));
    printf("%d, ", htonl(*(rr->TTL)));
    print_rr_data(rr);
    printf("\n");
    return;
}

//prints returned dns_response structure and info about its fields
void print_dns_response(struct dns_response * r){
    print_dns_header(r->header);

    printf("\nQuestion section (%d)\n", htons(r->header->qdcount));
    print_dns_question(r);
    
    int i = 0; // index of the record
    struct rr_record * current_record;

    printf("\nAnswer section (%d)\n", htons(r->header->ancount));
    for(int j = 0;j<htons(r->header->ancount);i++,j++){
        current_record = &(r->record_arr[i]);
        print_dns_resource(current_record);
    }
    printf("\nAuthority section (%d)\n", htons(r->header->nscount));
    for(int j = 0;j<htons(r->header->nscount);i++,j++){
        current_record = &(r->record_arr[i]);
        print_dns_resource(current_record);
    }
    printf("\nAdditional section (%d)\n", htons(r->header->arcount));
    for(int j = 0;j<htons(r->header->arcount);i++,j++){
        current_record = &(r->record_arr[i]);
        print_dns_resource(current_record);
    }
}

//frees dns_reponse structure and all its attributes
void free_dns_response(struct dns_response* r){
    if(r==NULL){
        return;
    }
    if(r->record_arr!=NULL){
        free(r->record_arr);
    }
    free(r);
}

//sends dns_query and receives its response
void send_dns_query(int client_socket,struct sockaddr *address, socklen_t address_size) {

    //SEND DATA
    int data_size = sizeof(struct dns_header);
    char buffer[MAX_PACKET_LENGTH];
    bzero(buffer, MAX_PACKET_LENGTH);

    // set dns header
    set_dns_header((struct dns_header *)buffer);
    // set q data
    data_size +=set_dns_question_section(buffer+sizeof(struct dns_header),target_arg);

    int bytes_rx = -1;
    int bytes_tx = -1;
    //SEND DATA

    bytes_tx = sendto(client_socket, buffer, data_size, 0, address, address_size);
    if (bytes_tx < 0)
    {
        perror("ERROR: sendto");
            exit(-1);
    }

    bzero(buffer, MAX_PACKET_LENGTH);
    //RECEIVE DATA - LOOP

    struct dns_response* dns_r = NULL; // dns_response structure that will be mapped onto the returned buffer
    
    // for dns message receive timeout, all responses with invalid id will be refused and client will be waiting for TIMEOUT_LENGTH seconds only, then end of program
    clock_t begin = clock();
    while((double)(clock() - begin) / CLOCKS_PER_SEC < TIMEOUT_LENGTH)
    {
        bytes_rx = recvfrom(client_socket, buffer, MAX_PACKET_LENGTH,0, address, &address_size);
        if (bytes_rx < 0)
        {
            perror("ERROR: message timeout or other error\n");
            exit(-1);
        }
        dns_r = map_dns_response(buffer);
        if(dns_r->header->id == (uint16_t)getpid() ){
            break;
        }
        bytes_rx = -1;
    }
    if (bytes_rx < 0){
        perror("ERROR: message timeout or other error\n");
        exit(-1);
    }

    print_dns_response(dns_r);
    free_dns_response(dns_r);
}	