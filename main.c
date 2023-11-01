#include "system_sam3x.h"
#include "at91sam3x8.h"

#include "kernel_functions.h"


unsigned int g0=0, g1=0, g2=0, g3=1, g5 = 0, g6 = 0; 
/* gate flags for various stages of unit test */

unsigned int low_deadline  = 1000;    
unsigned int high_deadline = 100000;


mailbox *intMbox; 
mailbox *floatMbox;


void task_body_1 (){

  set_deadline(3000);


  }

void task_Mailbox_2 (){

  int retValt1 ;
 char varChart1 ;

  mailbox *charMbox = create_mailbox( 3 , sizeof(char) );
  varChart1 = 'K';
  
  retValt1 = send_no_wait(charMbox, &varChart1);
  
  if ( retValt1 != OK ) 
  {    
      g1 = FAIL; 
      while(1) {}
  }

   retValt1 = receive_no_wait(charMbox, &varChart1);
  if ( retValt1 != OK ) { g2 = FAIL; while(1) {}}
  
  
  remove_mailbox(charMbox);
  
  terminate();
  

}

void main()
{
  SystemInit(); 
  SysTick_Config(100000); 
  SCB->SHP[((uint32_t)(SysTick_IRQn) & 0xF)-4] =  (0xE0);      
  isr_off();
  
  g0 = OK;
  exception retVal = init_kernel(); 
  if ( retVal != OK ) { g0 = FAIL; while(1) { /* no use going further */  } }
  
  if ( ReadyList->pHead != ReadyList->pTail ) { g0 = FAIL ;}
  if ( WaitingList->pHead != WaitingList->pTail )    { g0 = FAIL ;}
  if (   TimerList->pHead !=   TimerList->pTail )    { g0 = FAIL ;}
    
  if ( g0 != OK ) { while(1) { /* no use going further */  } }
  
   retVal = create_task( task_body_1 , low_deadline );
  if ( retVal !=  OK ) { while(1) { /* no use going further */  } }
  
  retVal = create_task( task_Mailbox_2 , 2*low_deadline );
  if ( retVal !=  OK ) { while(1) { /* no use going further */  } }
  
 
  run();
  
  while(1){ /* something is wrong with run */}
}
