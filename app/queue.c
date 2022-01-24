#include "app_bsp.h"
#include "queue.h"

void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    //Queue buffer initial values
    hqueue->Empty = 1; //Empty
    hqueue->Full = 0;  //Not full
    hqueue->Head = 0;  //Head at position 0
    hqueue->Tail = 0;  //Tail at position 0
}

uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t queueWrite = 0; //Queue write gets unsucceed value

    if(hqueue->Full == 0u) //If queue buffer not full
    {
        /* cppcheck misra deviation because this is a generic function that uses void pointer 
           and buffer's pointer needs to be increment to point the next address position */

        //data write in the queue buffer
        (void)memcpy(hqueue->Buffer+hqueue->Head, data, hqueue->Size); /* cppcheck-suppress misra-c2012-18.4*/
        hqueue->Empty = 0; //Queue not empty
        queueWrite = 1; //Queue write is succeed
        hqueue->Head += hqueue->Size; //Head increments to next position
        if(hqueue->Head == (hqueue->Size*hqueue->Elements)) //if head equals buffer's last position
        {
            hqueue->Head = 0; //head returns to zero
        }
        if(hqueue->Head == hqueue->Tail) //if head equals tail
        {
            hqueue->Full = 1; //Queue buffer is full
        }
    }

    return queueWrite; //returns 1 if write is succeed, 0 if unsucceed
}

uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t queueRead = 0; //Queue read is unsucceed

    if(hqueue->Empty == 0u) //If queue buffer not empty
    {
        /* cppcheck misra deviation because this is a generic function that uses void pointer 
           and buffer's pointer needs to be increment to point the next address position */

        //The value in queue buffer tail's position is copy to data
        (void)memcpy(data, hqueue->Buffer+hqueue->Tail, hqueue->Size); /* cppcheck-suppress misra-c2012-18.4*/
        hqueue->Full = 0; //buffer is not full
        queueRead = 1; //Queue read is succeed
        hqueue->Tail += hqueue->Size; //Tail increments to next position
        if(hqueue->Tail == (hqueue->Size*hqueue->Elements)) //if tail equals buffer's last position
        {
            hqueue->Tail = 0; //Tail returns to zero
        }
        if(hqueue->Tail == hqueue->Head) //if tail equals head
        {
            hqueue->Empty = 1; //Queue buffer is empty
        }
    }

    return queueRead; //returns 1 if read is succeed, 0 if unsucceed
}

uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    return hqueue->Empty; //return empty value
}