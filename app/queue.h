#ifndef QUEUE_H_
    #define QUEUE_H_

    #include <stdint.h>
    #include <string.h>

    typedef struct
    {
        void  	    *Buffer;
        uint32_t	Elements;
        uint8_t     Size;     //size of the type of element
        uint32_t	Head;
        uint32_t	Tail;
        uint8_t	    Empty;
        uint8_t	    Full;
        //add more elements if require
    }QUEUE_HandleTypeDef;
    
    //Queue function prototypes
    void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue );
    uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data );
    uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue );

#endif