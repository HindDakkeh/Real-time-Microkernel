#include "system_sam3x.h"
#include "at91sam3x8.h"
#include "kernel_functions.h"
#include <stdlib.h> 
#include <stdio.h>
#include "stdbool.h"
#include <limits.h> 


 TCB *PreviousTask;
 TCB *NextTask;
list *ReadyList;
list *WaitingList;
list *TimerList;

unsigned int Ticks;
 unsigned int KernelMode;

 
 // LAB 1




void Idle_Task(){
  
while(1);

}




// insert ascending by deadline.
exception insertTask(list *pList, TCB *pTask) {
  

  listobj *newTask = (listobj*) malloc(sizeof(listobj));
   
  if (newTask == NULL){
        
    return FAIL;
  
  }
     
  else{
    
    newTask->pTask = pTask;
    newTask->pPrevious = NULL;
    newTask->pNext = NULL;
    newTask->nTCnt = NULL;
    

    if (pList->pHead == NULL) {
        pList->pHead = newTask;
        pList->pTail = newTask;
        
        
          
        return OK;
    }

    if (pList->pHead->pTask->Deadline >= pTask->Deadline) {
        newTask->pNext = pList->pHead;
        pList->pHead->pPrevious = newTask;
        pList->pHead = newTask;
        return OK;
    }

    listobj *current = pList->pHead;
    while (current->pNext != NULL && current->pNext->pTask->Deadline < pTask->Deadline) {
        current = current->pNext;
    }

    
    if (current->pNext == NULL){
    
      newTask->pPrevious = current;
      current->pNext = newTask;
      pList->pTail = newTask;
    
    }
    
   
    else{ 
      
    newTask->pNext = current->pNext;
    newTask->pPrevious = current;
     current->pNext->pPrevious = newTask;
      current->pNext = newTask;
      
    }
     
  }
  
  return OK;
}



//remove head from readylist.
listobj* extract(listobj* head) {
  
  
  listobj* running_task = head;
  
  if(ReadyList->pHead != ReadyList->pTail){
   
         ReadyList->pHead = running_task->pNext;
        running_task->pNext->pPrevious = NULL;
        
        
          running_task->pNext = NULL;
    running_task->pPrevious = NULL;
    
    return running_task;
         }
  
 
      return NULL;

  
}




exception init_kernel(void){

     set_ticks(0);
     
   ReadyList = (list *)malloc(sizeof(list));
       
       if(ReadyList == NULL){
          
          return FAIL;

 } else {
  
   ReadyList->pHead = NULL ;
   ReadyList->pTail = NULL;
 }
 
      
  WaitingList = (list *)malloc(sizeof(list));
                               
         if(WaitingList == NULL){
          return FAIL;

 } else {
                          
     WaitingList->pHead = NULL;
     WaitingList->pTail = NULL;}
     
     
    TimerList = (list *)malloc(sizeof(list)); 
                               
           if(TimerList == NULL){
              return FAIL;

 } else {  
   
       TimerList->pHead= NULL;        
       TimerList->pTail=NULL;
 }
 
 
 create_task( Idle_Task , UINT_MAX);
 
  KernelMode = INIT;
  
  

  
 return OK;
}




exception create_task( void (* task_body)(), uint deadline){

TCB *new_tcb;

   new_tcb = (TCB *) calloc (1, sizeof(TCB));
   /* check if calloc was successful or not! */
   
   if (new_tcb == NULL){
   
     return FAIL;
   }
   
   else {
     
    new_tcb->PC = task_body;
   new_tcb->SPSR = 0x21000000;
    new_tcb->Deadline = deadline;

   new_tcb->StackSeg [STACK_SIZE - 2] = 0x21000000;
   new_tcb->StackSeg [STACK_SIZE - 3] = (unsigned int)task_body;
   new_tcb->SP = &(new_tcb->StackSeg [STACK_SIZE - 9]);
   
   
   }
   
   if (KernelMode == INIT){
      
    insertTask(ReadyList, new_tcb);
     
     return OK;
   }
   
   else {
   
     isr_off();
     
     PreviousTask = ReadyList->pHead->pTask; 
     insertTask(ReadyList, new_tcb);
     NextTask = ReadyList->pHead->pTask;
     
     SwitchContext();
   }
   
   return OK;
}



void run (void) {
  
  
     set_ticks(0);
  KernelMode = RUNNING;
  NextTask = ReadyList->pHead->pTask; 
  
   //restores registers from saved values from the TCB of NextTask
  
  LoadContext_In_Run ();
  
}



void terminate( void ){
  
  listobj *leavingObj;
  isr_off();
  
   leavingObj = extract(ReadyList->pHead);
    
    /* extract() detaches the head node from the ReadyList and
    * returns the list object of the running task */
   
   NextTask = ReadyList->pHead->pTask;
    switch_to_stack_of_next_task();


  free(leavingObj->pTask);
   free(leavingObj);
   LoadContext_In_Terminate();
   
 /* supplied to you in the assembly file
 * does not save any of the registers. Specifically, does not save the
    * process stack pointer (psp), but
    * simply restores registers from saved values from the TCB of NextTask
     * note: the stack pointer is restored from NextTask->SP
 */
}


// LAB 2
// create the mailbox
mailbox* create_mailbox( uint nMessages, uint nDataSize ){
  
//Allocate memory for the mailbox
 mailbox* mb = (mailbox*)malloc(sizeof(mailbox));
 
   if (mb == NULL){
  
     return NULL;}
   
   else {
     
   //Initialize mailbox structure
     mb->pHead = NULL;
        mb->pTail = NULL;
        mb->nDataSize = nDataSize;
       mb->nMaxMessages = nMessages;
        mb->nMessages =NULL;
        mb->nBlockedMsg = NULL;
   
   }
   
  return mb;
}  


//remove mailbox 
exception remove_mailbox(mailbox* mBox){

  //IF mailbox is empty THEN
  if (mBox->pHead == NULL){
  //Free the memory for the mailbox 
    free(mBox);
   return OK;
  }
  
  
  else {
    
    return NOT_EMPTY;
   
  }
                                   
}






// remove first message from the mailbox
msg* remove_first(mailbox*mBox){

     msg* H;
     
  if (mBox->pHead == NULL) {
  
    return NULL;
    
  }
  
  
  if(mBox->pHead != mBox->pTail){
   H = mBox->pHead;  
  mBox->pHead = H->pNext;
  H->pNext->pPrevious = NULL;}
  
  else {
  
    H = mBox->pHead; 
  mBox->pHead = NULL; 
    mBox->pTail = NULL;
  
  }
 
  mBox->nMessages--;
  
 
  
    return H;
}


// insert last message from the mailbox 
void insert_last(mailbox* sBox, msg* ms){
      
    
  
  
  if (sBox->pHead == NULL){
  
    sBox->pHead = ms;
    sBox->pTail = ms;
  
  }
 
  else {

  sBox->pTail->pNext = ms;
  
   ms->pPrevious = sBox->pTail;
   
      sBox->pTail = ms ;
   
  }

}

//search after the specified task in the list and return it and remove it then 
listobj* find_Task(list *pList, listobj* p){

  listobj* find = pList->pHead;
  
  while (find != NULL){
  
    if (find->pTask == p->pTask){
     
      listobj* finded = find;
      
     //remove it 
      
      if (find == pList->pHead && find == pList->pTail){
      
         pList->pHead = NULL;
         pList->pTail = NULL;
         
        
      }
      
      if (find == pList->pHead){
      
        pList->pHead = find->pNext;
        pList->pHead->pPrevious = NULL;
       
      }
      
       if (find == pList->pTail){
      
        pList->pTail = find->pPrevious;
        pList->pTail->pNext = NULL;
        
       
      }
      
      
      if(find->pPrevious!= NULL  &&  find->pNext != NULL){
      
         find->pPrevious->pNext = find->pNext;
          find->pNext->pPrevious = find->pPrevious;
      
      }
      
      
      
      finded->pNext = NULL;
      finded->pPrevious = NULL;
      
      
      return finded;
    
    }
    
    else {

    
    find = find->pNext;
  
    }}


 return NULL;

}


exception insert_Task_list(list *pList,  listobj *new_time ) {
  

  listobj *newTask = (listobj*) malloc(sizeof(listobj));
   
  if (newTask == NULL){
        
    return FAIL;
  
  }
     
  else{
    
    newTask->pTask = new_time->pTask;
    newTask->pPrevious = NULL;
    newTask->pNext = NULL;
    newTask->nTCnt = new_time->nTCnt;
    newTask->pMessage = new_time->pMessage;
    

    if (pList->pHead == NULL) {
        pList->pHead = newTask;
        pList->pTail = newTask;
        
        
          
        return OK;
    }

    if (pList->pHead->pTask->Deadline >= new_time->pTask->Deadline) {
        newTask->pNext = pList->pHead;
        pList->pHead->pPrevious = newTask;
        pList->pHead = newTask;
        return OK;
    }

    listobj *current = pList->pHead;
    while (current->pNext != NULL && current->pNext->pTask->Deadline < new_time->pTask->Deadline) {
        current = current->pNext;
    }

    
    if (current->pNext == NULL){
    
      newTask->pPrevious = current;
      current->pNext = newTask;
      pList->pTail = newTask;
    
    }
    
   
    else{ 
      
    newTask->pNext = current->pNext;
    newTask->pPrevious = current;
     current->pNext->pPrevious = newTask;
      current->pNext = newTask;
      
    }
     
  }
  
  return OK;
}



exception send_wait( mailbox* mBox, void* pData ){
  
  
   listobj* f;

   isr_off();
   
  // IF receiving task is waiting THEN
   
   if(mBox->pHead->Status == RECEIVER ){
   
     //Copy sender’s data to the data area of the receivers Message
     
     memcpy(mBox->pHead->pData,pData , mBox->nDataSize);
     
   //  Remove receiving task’s Message struct from the mailbox
    msg* M = remove_first(mBox);
    
   
     //Update PreviousTask
      PreviousTask = ReadyList->pHead->pTask;
      
      listobj* fin_rec_w = find_Task(WaitingList, M->pBlock);
      
      //Insert receiving task in ReadyList
      insert_Task_list(ReadyList, fin_rec_w);
      
     // Update NextTask
      NextTask = ReadyList->pHead->pTask;
   
   }
   
   
   else{
   
     //Allocate a Message structure Set data pointer
       msg* massage = (msg*)malloc(sizeof(msg));
       
      
       if (massage == NULL){
  
         return FAIL;}
       
       massage->pData = (char*)malloc(sizeof(char));
       
        if (massage->pData == NULL){
  
         return FAIL;}
        
        else {

          memcpy(massage->pData,pData , mBox->nDataSize);}
        
       
       massage->Status= SENDER;
       massage->pPrevious = NULL;
       massage->pNext = NULL;
       massage->pBlock = NULL;
            
          // check if the mailbox is full   
          if(mBox->nMessages == mBox->nMaxMessages){
           // remove the old message
             msg* S = remove_first(mBox);
             
           }
          
            // Add Message to the mailbox 
    
            insert_last(mBox, massage); 
            
            mBox->nMessages++;
            
            mBox->nBlockedMsg++;
            
     // Update PreviousTask      
     PreviousTask = ReadyList->pHead->pTask;
     
     //Move sending task from ReadyList to WaitingList
     f = extract(ReadyList->pHead);
     
     // Update NextTask
     insert_Task_list(WaitingList, f);
     
     
     massage->pBlock = f; 
    
     // Update NextTask

      NextTask = ReadyList->pHead->pTask;
   
   }
   
     
    SwitchContext();
   //IF deadline is reached THEN
   
   if(Ticks >= NextTask->Deadline){
        
        isr_off();
      
        //  Remove send Message &&&&&&&
        
         mBox->pHead->pBlock = NULL;
        
        remove_first(mBox);
        
          mBox->nBlockedMsg--;
        
        
        isr_on();
   
        return DEADLINE_REACHED;}
        
   else{ return OK;
   }    

}


exception receive_wait( mailbox* mBox, void* pData ){;
  
    
  listobj* Find_receive;
    
  isr_off();
  
   // IF send Message is waiting THEN   (if send and not receive)
 
  if(mBox->pHead->Status == SENDER){
    
  //Copy sender’s data to receiving task’s data area
   memcpy( pData , mBox->pHead->pData, mBox->nDataSize);
   
   //Remove sending task’s Message struct from the mailbox
    msg* M1 = remove_first(mBox);
     
    //IF Message was of wait type THEN (send_wait)

     if (M1->pBlock != NULL){
     
     PreviousTask = ReadyList->pHead->pTask;
     
     //Move sending task to ReadyList
     listobj* Find_send = find_Task(WaitingList, M1->pBlock );
     
     // insert the message in the readylist 
      insert_Task_list(ReadyList, Find_send);
      
       NextTask = ReadyList->pHead->pTask;
     
     }
     
     else {
     //Free senders data area     
     free(M1->pData);    //*********
         
     } 
  }
  
  
  else {
    
  // if the message is receiving
    
    //Allocate a Message structure
    
   msg* receive_message = (msg*)malloc(sizeof(msg));
      
      
       if (receive_message== NULL){
  
         return FAIL;}
       
       receive_message->pData = NULL;
       receive_message->Status= RECEIVER;
       receive_message->pPrevious = NULL;
       receive_message->pNext = NULL;
       receive_message->pBlock = NULL;
       
       
          if(mBox->nMessages == mBox->nMaxMessages){
           // remove the old message
            
            msg* S = remove_first(mBox);
             
        
                             
    }
    
          // Add Message to the mailbox
          insert_last(mBox, receive_message);
            
          mBox->nMessages++;
            
           mBox->nBlockedMsg++;
          
               PreviousTask = ReadyList->pHead->pTask;
               
             //Move receiving task from ReadyList to WaitingList
               
               Find_receive = extract(ReadyList->pHead);
               
               // insert the message in waitinglist.
               insert_Task_list(WaitingList, Find_receive);
               
               receive_message->pBlock = Find_receive;
                
                NextTask = ReadyList->pHead->pTask;
            
         }
         
            SwitchContext();
  
  //IF deadline is reached THEN

         if(Ticks >= NextTask->Deadline){
        
        isr_off();
        
         mBox->pHead->pBlock = NULL;
      // Remove receive Message
        
        remove_first(mBox);
        
        mBox->nBlockedMsg--;
      
     
        
        isr_on();
   
        return DEADLINE_REACHED;}
        
   else{ return OK;
   }    
           
                 
}


exception send_no_wait( mailbox* mBox, void* pData ){

   isr_off();
   
   if(mBox->pHead->Status == RECEIVER ){
   
      memcpy(mBox->pHead->pData, pData , mBox->nDataSize);
      
       //  Remove receiving task’s Message struct from the mailbox
      msg* REC = remove_first(mBox);
    
     //Update PreviousTask
      PreviousTask = ReadyList->pHead->pTask;
      
      listobj* fin_rec1_w = find_Task(WaitingList, REC->pBlock);
      
      //Insert receiving task in ReadyList
      insert_Task_list(ReadyList, fin_rec1_w);
      
     // Update NextTask
      NextTask = ReadyList->pHead->pTask;
      
       SwitchContext();
   
   
   } 
 
   else{
   
     msg* Send_no_message = (msg*)malloc(sizeof(msg));
      
      
       if (Send_no_message== NULL){
  
         return FAIL;}
       
        Send_no_message->pData = (char*)malloc(sizeof(char));
       
        if (Send_no_message->pData == NULL){
  
         return FAIL;}
        
        else {

          memcpy(Send_no_message->pData,pData , mBox->nDataSize);}
        
       
       
       Send_no_message->Status= SENDER;
       Send_no_message->pPrevious = NULL;
       Send_no_message->pNext = NULL;
       Send_no_message->pBlock = NULL;
       
       
          if(mBox->nMessages == mBox->nMaxMessages){
           // remove the old message
             msg* S = remove_first(mBox);
                                       
    }
    
          // Add Message to the mailbox
          insert_last(mBox, Send_no_message);
            
          mBox->nMessages++;
   
   
   }
   
  return OK;

}


exception receive_no_wait( mailbox* mBox, void* pData ){
  
   isr_off();
   
   
   if(mBox->pHead->Status == SENDER){
      
     memcpy( pData ,mBox->pHead->pData, mBox->nDataSize);
   
       msg* sen = remove_first(mBox);
       
       if (sen->pBlock != NULL){
     
     PreviousTask = ReadyList->pHead->pTask;
     
     //Move sending task to ReadyList
     listobj* Find_sen = find_Task(WaitingList, sen->pBlock );
     
     // insert the message in the readylist 
      insert_Task_list(ReadyList, Find_sen);
      
       NextTask = ReadyList->pHead->pTask;
   
     SwitchContext();
   }
   
   else {
     
     free(sen->pData);
   }
  
    return OK;
  
}
        
   else{
   
     return FAIL;
   }
   
 
  
}



 // LAB 3

listobj* find_Task_Timer(list *pList){

  listobj* find = pList->pHead;
  
  while (find != NULL){
  
    if (Ticks >=find->nTCnt|| Ticks >=find->pTask->Deadline ){
     
      listobj* finded = find;
      
     //remove it 
      
      if (find == pList->pHead && find == pList->pTail){
      
         pList->pHead = NULL;
         pList->pTail = NULL;
         
        
      }
      
      if (find == pList->pHead){
      
        pList->pHead = find->pNext;
        pList->pHead->pPrevious = NULL;
       
      }
      
       if (find == pList->pTail){
      
        pList->pTail = find->pPrevious;
        pList->pTail->pNext = NULL;
        
      }
      
      
      if(find->pPrevious != NULL  &&  find->pNext != NULL){
      
         find->pPrevious->pNext = find->pNext;
          find->pNext->pPrevious = find->pPrevious;
      
      }
      
      
      
      finded->pNext = NULL;
      finded->pPrevious = NULL;
      
      
      return finded;
    
    }
    
    

    
    find = find->pNext;
  
    }


 return NULL;

}



listobj* find_Task_Wait(list *pList){

  listobj* find = pList->pHead;
  
  while (find != NULL){
  
    if ( Ticks >=find->pTask->Deadline){
     
      listobj* finded = find;
      
     //remove it 
      
      if (find == pList->pHead && find == pList->pTail){
      
         pList->pHead = NULL;
         pList->pTail = NULL;
         
        
      }
      
      if (find == pList->pHead){
      
        pList->pHead = find->pNext;
        pList->pHead->pPrevious = NULL;
       
      }
      
       if (find == pList->pTail){
      
        pList->pTail = find->pPrevious;
        pList->pTail->pNext = NULL;
        
      }
      
      
      if(find->pPrevious!= NULL  &&  find->pNext != NULL){
      
         find->pPrevious->pNext = find->pNext;
          find->pNext->pPrevious = find->pPrevious;
      
      }
      
      
      
      finded->pNext = NULL;
      finded->pPrevious = NULL;
      
      
      return finded;
    
    }
    
    

    
    find = find->pNext;
  
    }


 return NULL;

}






exception wait(uint nTicks){


    isr_off();
    
    
    PreviousTask = ReadyList->pHead->pTask;

   
   listobj*  running = extract(ReadyList->pHead);
   
    running->nTCnt = nTicks;

   insert_Task_list(TimerList ,running);
   
  
   NextTask = ReadyList->pHead->pTask;
   
     SwitchContext();
    
     if (NextTask->Deadline<=Ticks){
     
       return DEADLINE_REACHED;
    
     }else{
     
       return OK;}
     
   
}




void set_ticks( uint nTicks ){

Ticks = nTicks;

}


uint ticks( void ){


return Ticks;
  
}



uint deadline( void ){


return ReadyList->pHead->pTask->Deadline;

}


void set_deadline( uint deadline ){


 isr_off();
 
 // update the deadline.
 ReadyList->pHead->pTask->Deadline = deadline;
 
  PreviousTask = ReadyList->pHead->pTask;

   // reschedule readylist
   listobj*  new_deadline = extract(ReadyList->pHead);
   
   
   insert_Task_list(ReadyList,new_deadline);


   NextTask = ReadyList->pHead->pTask;
   
     SwitchContext();

}

void TimerInt(){

  
  Ticks ++;
  
 int flag = 1;
 int flag2 = 1;
  
  
   while (flag == 1 ){
   
 
     listobj* n = find_Task_Timer(TimerList);
    
  
      if(n != NULL){
   
        PreviousTask = ReadyList->pHead->pTask;
     
        insert_Task_list(ReadyList, n);
        
       NextTask=  ReadyList->pHead->pTask;
      }
      
      else{
      
        flag = 0;
      }
      
   }
 
 
 
      
  
   //Check the WaitingList for tasks that have expired deadlines, move these to ReadyList
      
      while(flag2 == 1){
        
        
       listobj* m = find_Task_Wait(WaitingList);
       
       if (m != NULL){
      
         PreviousTask = ReadyList->pHead->pTask;
      
         insert_Task_list(ReadyList,m);
       
         NextTask=  ReadyList->pHead->pTask;
       }
         // SwitchContext();
    
       else { flag2 = 0;}

                                       }  
     
  
}

