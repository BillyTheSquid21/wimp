#include <wimp_reciever.h>

/*
* Starts by establishing connection on the specified port with a handshake
*/
void wimp_reciever_recieve(RecieverArgs* args)
{
    uint8_t recieve_buffer[WIMP_RECIEVER_MESSAGE_BUFFER_SIZE];
    uint8_t send_buffer[WIMP_RECIEVER_MESSAGE_BUFFER_SIZE];

    PSocket* reciever_socket;
    PSocketAddress* reciever_address;

    //Construct address for client, which should be listening
    reciever_address = p_socket_address_new(args->domain, args->target_port_number);
    if (reciever_address == NULL)
    {
        p_uthread_exit(WIMP_RECIEVER_FAIL);
        return;
    }

    //Create the main listen/recieve socket - currently hard coded
    reciever_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL);
    if (reciever_socket == NULL)
    {
        p_uthread_exit(WIMP_RECIEVER_FAIL);
        return;
    }

    //Connect to end process, which should be waiting to accept
    if (!p_socket_connect(reciever_socket, reciever_address, NULL))
    {
        p_socket_address_free(reciever_address);
        p_socket_free(reciever_socket);
        printf("END PROCESS NOT WAITING!\n");
        p_uthread_exit(WIMP_RECIEVER_FAIL);
        return;
    }

    //Recieve the message, which should be the handshake code
    int handshake_size = p_socket_receive(reciever_socket, recieve_buffer, WIMP_RECIEVER_MESSAGE_BUFFER_SIZE, NULL);

    //Check the equality - as is the first 4 bytes convert to uint
    uint32_t potential_handshake = *((uint32_t*)((void*)recieve_buffer));
    if (potential_handshake != WIMP_RECIEVER_HANDSHAKE)
    {
        printf("HANDSHAKE FAILED!\n");
        p_socket_address_free(reciever_address);
        p_socket_close(reciever_socket, NULL);
        p_uthread_exit(WIMP_RECIEVER_FAIL);
        return;
    }
    printf("HANDSHAKE SUCCESS!\n");
    
    //Send a handshake back to the client so it knows to continue
    p_socket_send(reciever_socket, recieve_buffer, WIMP_RECIEVER_HANDSHAKE_LENGTH, NULL);

    //Now sit around waiting for commands, to write them to the table and loop
    bool disconnect = false;
    while (!disconnect)
    {
        int32_t recieve_length = p_socket_receive(reciever_socket, recieve_buffer, WIMP_RECIEVER_MESSAGE_BUFFER_SIZE, NULL);
        if (recieve_length > 0)
        {
            //WRITE INSTRUCTIONS ALSO CHECK IF SHUTDOWN OF END THREAD HAS HAPPENED
            *args->writebuff = 1; //TEMPORARY CHECK TO SHOW IS WORKING
            disconnect = true;
        }
    }

    p_socket_address_free(reciever_address);
    p_socket_close(reciever_socket, NULL);

    printf("Reciever thread closed: %d\n", reciever_socket);
    p_uthread_exit(WIMP_RECIEVER_SUCCESS);
    return;
}







bool check_buffer_command(const char* buffer, const char* command, size_t buffer_len, size_t command_len)
{
    //Check the buffer is at least the length of the command
    if (buffer_len < command_len)
    {
        return false;
    }

    //Check the buffer up to the last character, ignoring the command null terminator
    if (strncmp(buffer, command, command_len - 1) != 0)
    {
        return false;
    }

    return true;
}