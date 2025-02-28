#include <winsock2.h>
#include <string.h>
#include <stdio.h>


//define constants
#define PORT 8080
#define BUFFER 2048

// Function to read the HTML file
char* read_html_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening HTML file: %s\n", filename);
        return NULL; // Return NULL on error
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET); // Rewind

    // Allocate memory for the file content (+1 for null terminator)
    char* content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(file);
        return NULL;
    }

    // Read the file content
    size_t bytes_read = fread(content, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "Error reading file content.\n");
        free(content);
        fclose(file);
        return NULL;
    }

    content[file_size] = '\0';
    fclose(file);
    return content;
}

int main(){
    //declare variables
    WSADATA wsa;
    struct sockaddr_in server_addr,client_addr;
    SOCKET server_fd,client_fd;
    char buffer[BUFFER];
    int client_addr_len=sizeof(client_addr);

    int iResult=WSAStartup(MAKEWORD(2,2),&wsa);
    //init winsock
    if (iResult!=0){
        fprintf(stderr,"WSA Startup Error %d\n",WSAGetLastError());
        return 1;
    }

    //create server socket
    server_fd=socket(AF_INET,SOCK_STREAM,0);
    //check socket
    if (server_fd==INVALID_SOCKET){
        fprintf(stderr,"Server Socket Error %d\n",WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //assign server
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_family=AF_INET;
    //chanage server sock options
    int reuse=1;
    if (setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(const char*)&reuse,sizeof(reuse))<0){
        perror("reuse addr failed");
        WSACleanup();
        return 1;
    }
    //now bind the socket to the server
    if (bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==SOCKET_ERROR){
        fprintf(stderr,"Error Binding Server Socket %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //now listen on socket
    if (listen(server_fd,SOMAXCONN)==SOCKET_ERROR){
        fprintf(stderr,"Error Listening on Socket %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    while (1){
        //accept connection and define client
        client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);
        if (client_fd==INVALID_SOCKET){
            fprintf(stderr,"Error Accepting Client Connection %d\n",WSAGetLastError());
            return 1;
        }
        //revice data
        int Bytes_recv=recv(client_fd,buffer,BUFFER,0);
        if (Bytes_recv==SOCKET_ERROR){
            fprintf(stderr,"Error Reciving Information %d\n",WSAGetLastError());
            closesocket(client_fd);
            continue;
        }
        if (Bytes_recv > 0){
            buffer[Bytes_recv]='\0';
            printf("Recived Buffer %s\n",buffer);
            //send response
            char* html_content=read_html_file("index.html");
            //check html content
            if (html_content==NULL){
                const char* error_response="HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nError reading HTML file.";
                send(client_fd,error_response,strlen(error_response),0);
                closesocket(client_fd);
                continue;
            }
            char* response_headers="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
            char content_length_str[50];
            _snprintf(content_length_str,sizeof(content_length_str),"%zu",strlen(html_content));
            char* response_end="\r\n\r\n";
            // Calculate total response length
            // Calculate total response length
            size_t total_length = strlen(response_headers) + strlen(content_length_str) + strlen(response_end) + strlen(html_content);

            // Allocate memory for the full response
            char* response = (char*)malloc(total_length + 1);  // +1 for null terminator
            if (response == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            free(html_content);
            closesocket(client_fd);
            continue;
            }


            // Build the full response string using sprintf (or snprintf for safety)
            snprintf(response, total_length + 1, "%s%s%s%s", response_headers, content_length_str, response_end, html_content);
            response[total_length] = '\0'; // Null terminate

            // Send the response
            send(client_fd, response, strlen(response), 0);

            free(html_content); // Free allocated memory for HTML content
            free(response);     // Free allocated memory for the full response
            closesocket(client_fd);
        } else if (Bytes_recv==0){
            printf("Client Disconnected");
        } else{
            fprintf(stderr,"Recive Error %d\n", WSAGetLastError());
        }
        closesocket(client_fd);
    }
    closesocket(server_fd);
    WSACleanup();
    return 0;
}