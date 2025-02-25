// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef Lynx /* LynxOS */
#include <unistd.h>
#include <pwd.h>
#include <timeb.h>
#endif

#ifdef Linux /* Linux */
#include <unistd.h>
#include <pwd.h>
#endif

#ifdef Solaris /* Solaris */
#include <unistd.h>
#include <pwd.h>
#endif

#ifdef Darwin /* Max OS X */
#include <unistd.h>
#include <pwd.h>
#define fgetpos64 fgetpos
#define fopen64 fopen
#define fseeko64 fseek
#define fpos64_t fpos_t
#endif

#ifdef _MSC_VER
#include <WTypes.h>
#include <wchar.h>

#define fgetpos64 fgetpos
#define fopen64 fopen
#define fseeko64 fseek
#define fpos64_t fpos_t
#endif

#include "fLmd.h"
#include "f_ut_time.h"


int32_t  fLmdWriteBuffer(sLmdControl *, char *, uint32_t);
uint32_t fLmdCleanup(sLmdControl *);
void     fLmdOffsetResize(sLmdControl *, uint32_t);
uint32_t fLmdOffsetSet(sLmdControl *, uint32_t );
uint32_t fLmdOffsetRead(sLmdControl *);
uint32_t fLmdOffsetWrite(sLmdControl *);
lmdoff_t fLmdOffsetGet(sLmdControl *, uint32_t);
void     fLmdOffsetElements(sLmdControl *, uint32_t, uint32_t *, uint32_t *);
#define OFFSET__ENTRIES 250000

//===============================================================
uint32_t fLmdPutOpen(sLmdControl *pLmdControl,
                     char    *Filename,
                     sMbsFileHeader *pBuffHead, // LMD__STANDARD_HEADER (NULL) or address
                     uint32_t iBytes,           // LMD__NO_BUFFER (0) or buffer size
                     uint32_t iOver,            // LMD__[NO_]OVERWRITE
                     uint32_t iUseOffset,       // LMD__[NO_]INDEX
                     uint32_t iLargeFile)      // LMD__[NO_]LARGE_FILE
{
   int32_t iReturn;
   struct timespec clock;

   memset(pLmdControl,0,sizeof(sLmdControl));

   // allocate header or take extern
   if(pBuffHead == LMD__STANDARD_HEADER){
      pLmdControl->pMbsFileHeader = (sMbsFileHeader *)malloc(sizeof(sMbsFileHeader));
      if (!pLmdControl->pMbsFileHeader) {
         printf("fLmdPutOpen: memory allocation error\n");
         return(LMD__FAILURE);
      }
      memset(pLmdControl->pMbsFileHeader,0,sizeof(sMbsFileHeader));
      pLmdControl->iInternHeader=1;
   } else {
      pLmdControl->pMbsFileHeader= pBuffHead;
      pLmdControl->iInternHeader=0;
   }

   clock_gettime(CLOCK_REALTIME,&clock);
   pLmdControl->pMbsFileHeader->iTimeSpecSec=clock.tv_sec;
   pLmdControl->pMbsFileHeader->iTimeSpecNanoSec=clock.tv_nsec;

   pLmdControl->pMbsFileHeader->iType=LMD__TYPE_FILE_HEADER_101_1;
   pLmdControl->pMbsFileHeader->iEndian=1;
   strcpy(pLmdControl->cFile,Filename);

   // optionally allocate buffer
   if(iBytes > 0){
      pLmdControl->pBuffer = (int16_t*) malloc(iBytes);
      pLmdControl->iInternBuffer = 1;
   }
   pLmdControl->iBufferWords=iBytes/2;
   pLmdControl->iLeftWords=iBytes/2;
   // open file
   if(iOver == LMD__NO_OVERWRITE){  // do not overwrite
      if((pLmdControl->fFile=(FILE *)fopen64(Filename,"r") )!=NULL){
         printf("fLmdPutOpen: File exists: %s\n",Filename);
         fLmdCleanup(pLmdControl);
         fclose(pLmdControl->fFile);
         return(PUTLMD__FILE_EXIST);
      }
   }

   if((pLmdControl->fFile=(FILE *)fopen64(Filename,"w+") )== NULL){
      printf("fLmdPutOpen: Error open file %s\n",Filename);
      fLmdCleanup(pLmdControl);
      return(PUTLMD__OPEN_ERR);
   }

   if(iLargeFile == LMD__LARGE_FILE)pLmdControl->iOffsetSize=8;
   else                             pLmdControl->iOffsetSize=4;
   pLmdControl->pMbsFileHeader->iOffsetSize=pLmdControl->iOffsetSize;

   // write header
   iReturn=fLmdWriteBuffer(pLmdControl,(char *)pLmdControl->pMbsFileHeader,
         (pLmdControl->pMbsFileHeader->iUsedWords)*2+sizeof(sMbsFileHeader));
   pLmdControl->iBytes+=iReturn;

   if(iUseOffset == LMD__INDEX)fLmdOffsetResize(pLmdControl,iReturn/4); // create and set first value
   printf("fLmdPutOpen: %s. Bytes:%d over:%d table:%d large:%d.\n",
         Filename,iBytes,iOver,iUseOffset,iLargeFile);
   return(LMD__SUCCESS);
}

//===============================================================
uint32_t fLmdPutElement(sLmdControl *pLmdControl,sMbsHeader *pHeader)
{
  uint32_t *ps, *pd, i, elements;
  int64_t fileleft,used;
  int32_t iReturn;

  // enough space left?
  if(pLmdControl->iOffsetEntries && (pLmdControl->iOffsetSize == 4)){
    elements=pLmdControl->iElements+2;
    used=pLmdControl->iBytes/4;
    fileleft=0xffffffff - used - (4+elements); // size of table
    if((int64_t)(pHeader->iWords/2+2) > fileleft){
      printf("fLmdPutElement: File size exceed\n");
      return(PUTLMD__EXCEED);
    }
  }
  // save largest size in header
  if((pHeader->iWords+4) > pLmdControl->pMbsFileHeader->iMaxWords)
      pLmdControl->pMbsFileHeader->iMaxWords=pHeader->iWords+4;
  // no buffer, write element directly
  if(pLmdControl->iBufferWords == 0){
    pLmdControl->pMbsHeader=pHeader;
    iReturn=fLmdWriteBuffer(pLmdControl,(char *)pHeader,(pHeader->iWords+4)*2);
    pLmdControl->iBytes+=iReturn;
    if((uint32_t) iReturn != (pHeader->iWords+4)*2){
       printf("fLmdPutElement: Write error \n");
       return LMD__FAILURE;
    }
    pLmdControl->pMbsFileHeader->iElements++;
    pLmdControl->iElements++;
    if(pLmdControl->iOffsetEntries)fLmdOffsetSet(pLmdControl,iReturn/4);
    return(LMD__SUCCESS);

  } // end no buffer
  if((pHeader->iWords+4) > pLmdControl->iLeftWords){ // flash buffer to file
    iReturn = fLmdWriteBuffer(pLmdControl, (char *)pLmdControl->pBuffer,
                             (pLmdControl->iBufferWords-pLmdControl->iLeftWords)*2);
    pLmdControl->iBytes+=iReturn;
    if((uint32_t) iReturn != (pLmdControl->iBufferWords-pLmdControl->iLeftWords)*2)
       return LMD__FAILURE;
    pLmdControl->iLeftWords=pLmdControl->iBufferWords; // buffer free
  }
  if((pHeader->iWords+4) > pLmdControl->iLeftWords){ // element too big for buffer
    printf("fLmdPutElement: Element too big: %d words\n",pHeader->iWords+4);
    return(PUTLMD__TOOBIG);
  }
  // copy to buffer
  ps=(uint32_t *)pHeader;
  pd=(uint32_t *)pLmdControl->pBuffer+(pLmdControl->iBufferWords-pLmdControl->iLeftWords)/2;
  iReturn=(pHeader->iWords+4)/2; // here 32b words
  for(i=0;i<iReturn;i++) *pd++ = *ps++;
  pLmdControl->pMbsFileHeader->iElements++;
  pLmdControl->iElements++;
  pLmdControl->iLeftWords -= (pHeader->iWords+4);
  if(pLmdControl->iOffsetEntries)
     fLmdOffsetSet(pLmdControl,iReturn);
  return (LMD__SUCCESS);
}

//===============================================================
uint32_t fLmdPutBuffer(sLmdControl *pLmdControl, sMbsHeader *pHeader, uint32_t Items)
{
  sMbsHeader *pH;
  uint32_t Bytes=0,TotalBytes=0,i, elements;
  int64_t fileleft,used;
  int32_t iReturn;

  // check if total buffer fits in file
  if(pLmdControl->iOffsetEntries && (pLmdControl->iOffsetSize == 4)){
     pH=pHeader; // SL 16.11.2009 - pH was not initialized in this branch
    elements=pLmdControl->iElements+Items+2;
    used=pLmdControl->iBytes/4;
    fileleft=0xffffffff - used - (4+elements); // size of table
    for(i=0;i<Items;i++) {
       Bytes = (4+pH->iWords)*2;
       TotalBytes += Bytes;
       pH=(sMbsHeader *)((int16_t*)pH + Bytes/2);
    }
    if((int64_t)TotalBytes/4 > fileleft){
      printf("fLmdPutElement: File size exceed\n");
      return(PUTLMD__EXCEED);
    }
    Bytes=0;
    TotalBytes=0;
  }
  pH = pHeader;
  for(i=0;i<Items;i++)
    {
      pLmdControl->iElements++;
      Bytes = (4+pH->iWords)*2;
      TotalBytes += Bytes;
      if(pLmdControl->iOffsetEntries)fLmdOffsetSet(pLmdControl,Bytes/4);
      if((pH->iWords+4) > pLmdControl->pMbsFileHeader->iMaxWords)
          pLmdControl->pMbsFileHeader->iMaxWords=pH->iWords+4;
      pH=(sMbsHeader *)((int16_t *)pH+Bytes/2);
    }
  iReturn = fLmdWriteBuffer(pLmdControl,(char *)pHeader,TotalBytes);
  pLmdControl->iBytes+=iReturn;
  if((uint32_t) iReturn != TotalBytes)
     return LMD__FAILURE;

  pLmdControl->pMbsFileHeader->iElements += Items;
  return(LMD__SUCCESS);
}

//===============================================================
uint32_t fLmdPutClose(sLmdControl *pLmdControl)
{
  int32_t iReturn;

  if(pLmdControl->iBufferWords > pLmdControl->iLeftWords){ // write last buffer
    iReturn = fLmdWriteBuffer(pLmdControl, (char *)pLmdControl->pBuffer,
                             (pLmdControl->iBufferWords-pLmdControl->iLeftWords)*2);
    pLmdControl->iBytes+=iReturn;
    if((uint32_t)iReturn != (pLmdControl->iBufferWords-pLmdControl->iLeftWords)*2) {
      printf("fLmdPutClose: Error writing last buffer. Closing file.\n");
      // rewind file and rewrite header
      rewind(pLmdControl->fFile);  /* rewind file, rewrite header */
      fLmdWriteBuffer(pLmdControl, (char *)pLmdControl->pMbsFileHeader,
            sizeof(sMbsFileHeader));
      fLmdCleanup(pLmdControl);
      return(LMD__FAILURE);
    }
  }
  if(pLmdControl->iOffsetEntries)
    if(fLmdOffsetWrite(pLmdControl) != LMD__SUCCESS)
      pLmdControl->pMbsFileHeader->iTableOffset=0; // table could not be written

  // rewind file and rewrite header
  rewind(pLmdControl->fFile);  /* rewind file, rewrite header */
  fLmdWriteBuffer(pLmdControl, (char *)pLmdControl->pMbsFileHeader,
        sizeof(sMbsFileHeader));
  return(fLmdGetClose(pLmdControl));
}

#ifndef FILEONLY
//===============================================================
uint32_t fLmdInitMbs(sLmdControl *pLmdControl,
                     char    *Nodename,
                     uint32_t iMaxBytes,
                     uint32_t iBuffers,
                     uint32_t iStreams,
                     uint32_t iPort,
                     uint32_t iTimeout)
{
  if(iBuffers > 1){printf("fLmdInitMbs: Event spanning not supported!\n");return(LMD__FAILURE);}
  if(iStreams > 0){printf("fLmdInitMbs: MBS not in DABC mode!\n");return(LMD__FAILURE);}
  pLmdControl->iPort=iPort;
  strcpy(pLmdControl->cFile,Nodename);
  if(pLmdControl->pBuffer == NULL) pLmdControl->pBuffer = (int16_t*) malloc(iMaxBytes);
  pLmdControl->iBufferWords=iMaxBytes/2;
  pLmdControl->iInternBuffer=1;
  pLmdControl->iTCP=pLmdControl->pTCP->socket;
  pLmdControl->iTcpTimeout=iTimeout;
  pLmdControl->iTCPowner=0;
  return(LMD__SUCCESS);
}
//===============================================================
uint32_t fLmdCloseMbs(sLmdControl *pLmdControl)
{

  int32_t stat;
  char cClose[12];
  // send request buffer for stream server
  if(pLmdControl->iPort == PORT__STREAM) {
     memset(cClose,0,sizeof(cClose));
     strcpy(cClose, "CLOSE");
     stat=f_stc_write(cClose,12,pLmdControl->iTCP);
  }
  stat=f_stc_close(pLmdControl->pTCP);
  pLmdControl->pMbsFileHeader = NULL; // was reference only
  if(pLmdControl->iTCPowner == 0) pLmdControl->pTCP=NULL; // was reference only
  fLmdCleanup(pLmdControl);
  return(stat);
}
//===============================================================
uint32_t fLmdGetMbsEvent(sLmdControl *pLmdControl, sMbsHeader** event)
{
   uint32_t stat;
   sMbsHeader *pM;
   *event=NULL;
   if(pLmdControl->iLeftWords == 0){ // get new buffer
      stat=fLmdGetMbsBuffer(pLmdControl,NULL,0,NULL,NULL);
      if(stat != LMD__SUCCESS){
         return(stat);
      }
      // first event behind header:
      pLmdControl->pMbsHeader=(sMbsHeader *)(pLmdControl->pBuffer+sizeof(sMbsBufferHeader)/2);
   }
   pM=pLmdControl->pMbsHeader; // current to be returned
   pLmdControl->iLeftWords -= (pLmdControl->pMbsHeader->iWords+4);
   pLmdControl->pMbsHeader =
         (sMbsHeader *)((int16_t *)pLmdControl->pMbsHeader +
               pLmdControl->pMbsHeader->iWords+4);
   pLmdControl->iElements++;
   *event=pM;
   return(LMD__SUCCESS);
}
//===============================================================
uint32_t fLmdGetMbsBuffer(sLmdControl *pLmdControl, sMbsBufferHeader *pBuffer, uint32_t iBytes, uint32_t *iElements, uint32_t *iBytesUsed)
{

  sMbsBufferHeader *pBuf;
  uint32_t usedBytes = 0, leftBytes = 0;
  int32_t iReturn;
  char cRequest[12];

  leftBytes = iBytes;
  pBuf = pBuffer;
  if(pBuf == NULL){
    pBuf = (sMbsBufferHeader *)pLmdControl->pBuffer; // internal buffer
    leftBytes = pLmdControl->iBufferWords*2; // size of this buffer
  }
  if(pBuf == NULL){
    printf("fLmdGetMbsBuffer: Need buffer to read\n");
    return(LMD__FAILURE);
  }
  if (leftBytes < sizeof(sMbsBufferHeader)) {
     printf("fLmdGetMbsBuffer: %s buffer size %d too small for %d bytes\n", pLmdControl->cFile, leftBytes,
            (int)sizeof(sMbsBufferHeader));
     return (LMD__FAILURE);
  }
  // send request buffer for stream server
  if(pLmdControl->iPort == PORT__STREAM) {
     memset(cRequest,0,sizeof(cRequest));
     strcpy(cRequest, "GETEVT");
     iReturn=f_stc_write(cRequest,12,pLmdControl->iTCP);
  }
  iReturn=f_stc_read((int32_t *)pBuf,sizeof(sMbsBufferHeader),pLmdControl->iTCP,pLmdControl->iTcpTimeout);
  if(iReturn == STC__TIMEOUT) return(LMD__TIMEOUT);
  if(iReturn != STC__SUCCESS) return(LMD__FAILURE);
  if(pLmdControl->iSwap)fLmdSwap4((uint32_t *)pBuf,sizeof(sMbsBufferHeader)/4);
  if(leftBytes < (sizeof(sMbsBufferHeader)+2*pBuf->iUsedWords)){
     printf("fLmdGetMbsBuffer: %s buffer size %d too small for %lu bytes\n",
     pLmdControl->cFile,leftBytes,(long unsigned) (sizeof(sMbsBufferHeader)+2*pBuf->iMaxWords));
     return(LMD__FAILURE);
  }
  usedBytes = pBuf->iUsedWords*2;
  if((pBuf->iType & 0xffff) == 100)
    iReturn=f_stc_read((int32_t *)(pBuf+1),usedBytes,pLmdControl->iTCP,-1);
  if(iReturn == STC__TIMEOUT) return(LMD__TIMEOUT);
  if(iReturn != STC__SUCCESS) return(LMD__FAILURE);
  if(pLmdControl->iSwap)fLmdSwap4((uint32_t *)(pBuf+1),usedBytes/4);
  if(iBytesUsed != NULL)*iBytesUsed =usedBytes+sizeof(sMbsBufferHeader);
  if(iElements  != NULL)*iElements  =pBuf->iElements;
  pLmdControl->iBytes += usedBytes;
  pLmdControl->iLeftWords = usedBytes/2; // without header
  pLmdControl->pMbsFileHeader = (sMbsFileHeader *)pBuf;
  return(LMD__SUCCESS);
}
#endif // FILEONLY

//===============================================================
uint32_t fLmdGetOpen(sLmdControl *pLmdControl,
                     char    *Filename,
                     sMbsFileHeader *pBuffHead, // LMD__INTERNAL_HEADER (NULL) or address of file header
                     uint32_t iBytes,           // LMD__NO_BUFFER (0) or LMD__MIN_BUFFER or internal buffersize
                     uint32_t iUseOffset)      // LMD__[NO_]INDEX
{
   int32_t iReturn;
   uint32_t bufferBytes=0;

   memset(pLmdControl,0,sizeof(sLmdControl));
   if(pBuffHead == LMD__INTERNAL_HEADER){
      pLmdControl->pMbsFileHeader = (sMbsFileHeader *)malloc(sizeof(sMbsFileHeader));
      pLmdControl->iInternHeader=1;
   } else {
      pLmdControl->pMbsFileHeader = pBuffHead;
      pLmdControl->iInternHeader=0;
   }
   memset(pLmdControl->pMbsFileHeader,0,sizeof(sMbsFileHeader));

   // copy file name to control structure
   strcpy(pLmdControl->cFile,Filename);
   if((pLmdControl->fFile=(FILE *)fopen64(Filename,"r"))== NULL)
   {
      printf("fLmdGetOpen: File not found: %s\n",Filename);
      fLmdCleanup(pLmdControl);
      return(GETLMD__NOFILE);
   }
   /* read header */
   iReturn=fLmdReadBuffer(pLmdControl,
         (char *)pLmdControl->pMbsFileHeader,
         sizeof(sMbsFileHeader));
   if(iReturn!=sizeof(sMbsFileHeader)) {
      printf("fLmdGetOpen: LMD format error: no LMD file: %s\n",Filename);
      fLmdGetClose(pLmdControl);
      return(GETLMD__NOLMDFILE);
   }
   // check type and subtype, and endian
   if(pLmdControl->pMbsFileHeader->iEndian != 1) pLmdControl->iSwap=1;
   if(pLmdControl->iSwap){
      printf("do swap !!!\n");
      fLmdSwap4((uint32_t *)pLmdControl->pMbsFileHeader,sizeof(sMbsFileHeader)/4);
      fLmdSwap8((uint64_t *)&pLmdControl->pMbsFileHeader->iTableOffset,1);
   }
   if(pLmdControl->pMbsFileHeader->iType != LMD__TYPE_FILE_HEADER_101_1){
      printf("fLmdGetOpen: LMD format error: no LMD file: %s, type is %0x\n",
            Filename,pLmdControl->pMbsFileHeader->iType);
      fLmdGetClose(pLmdControl);
      return(GETLMD__NOLMDFILE);
   }

   if((iUseOffset == LMD__INDEX)&&(pLmdControl->pMbsFileHeader->iTableOffset > 0)){
      // printf("fLmdGetOpen: use table in file: %s\n",Filename);
      pLmdControl->iOffsetSize=pLmdControl->pMbsFileHeader->iOffsetSize;
      iReturn=fLmdOffsetRead(pLmdControl); // read offset table
      if(iReturn != LMD__SUCCESS){
         printf("fLmdGetOpen: Index format error: %s\n",Filename);
         fLmdGetClose(pLmdControl);
         return(iReturn);
      }
   }

   pLmdControl->iBytes+=iReturn;
   // more of header?
   if(pLmdControl->pMbsFileHeader->iUsedWords > 0)
   {
      // Read this additional information without swapping.
      // Could be mostly strings. Caller must know.
      pLmdControl->cHeader = malloc(pLmdControl->pMbsFileHeader->iUsedWords*2);
      iReturn = fLmdReadBuffer(pLmdControl,pLmdControl->cHeader,
                               pLmdControl->pMbsFileHeader->iUsedWords*2 );
      if((uint32_t) iReturn != pLmdControl->pMbsFileHeader->iUsedWords*2) {
         printf("fLmdGetOpen: LMD format error: no LMD file: %s\n",Filename);
         fLmdGetClose(pLmdControl);
         return(GETLMD__NOLMDFILE);
      }
   }

   bufferBytes = iBytes;
   if(bufferBytes < pLmdControl->pMbsFileHeader->iMaxWords*2)
      bufferBytes = pLmdControl->pMbsFileHeader->iMaxWords*2;
   fLmdPrintFileHeader(1,pLmdControl->pMbsFileHeader);
   pLmdControl->pBuffer = (int16_t *)malloc(bufferBytes);
   pLmdControl->iInternBuffer = 1;
   pLmdControl->iBufferWords=bufferBytes/2; // will be increased if necessary

   printf("fLmdGetOpen: %s words %u\n", Filename, pLmdControl->iBufferWords);

   pLmdControl->iLeftWords = 0; // buffer empty, read with first fLmdGetElement
   pLmdControl->pMbsHeader = NULL;
   return(LMD__SUCCESS);
}

//===============================================================
uint32_t fLmdGetBuffer(sLmdControl *pLmdControl, sMbsHeader *pMbsHeader, uint32_t iBytes, uint32_t *iElements,
                       uint32_t *iBytesUsed)
{

   sMbsHeader *pm;
   uint32_t elem = 0, leftBytes = 0, used, elem_sz;
   int32_t iReturn;

   if (iBytes < pLmdControl->pMbsFileHeader->iMaxWords) {
      printf("fLmdGetBuffer: %s buffer size %d too small for %d bytes\n", pLmdControl->cFile, iBytes,
             pLmdControl->pMbsFileHeader->iMaxWords);
      return (LMD__FAILURE);
   }
   if (pMbsHeader == NULL) {
      printf("fLmdGetBuffer: Need buffer to read\n");
      return (LMD__FAILURE);
   }
   *iBytesUsed = 0;
   *iElements = 0;
   if (pLmdControl->iElements == pLmdControl->pMbsFileHeader->iElements)
      return (GETLMD__EOFILE);

   // Offset table
   if (pLmdControl->iOffsetEntries) { // use offsets to read elements fitting in buffer
      fLmdOffsetElements(pLmdControl, iBytes, &elem, &used);
      // printf("Read %d bytes of %d, elements %d\n",used,iBytes,elem);
      iReturn = fLmdReadBuffer(pLmdControl, (char *)pMbsHeader, used);
      if (iReturn <= 0) {
         printf("fLmdGetBuffer: EOF: %s\n", pLmdControl->cFile);
         return (GETLMD__EOFILE);
      }
      if ((uint32_t) iReturn != used) {
         printf("fLmdGetBuffer: LMD read error: unexpected EOF: %s %u %u\n", pLmdControl->cFile, iReturn, used);
         return (GETLMD__NOLMDFILE);
      }
      *iBytesUsed = used;
      *iElements = elem;
      if (pLmdControl->iSwap)
         fLmdSwap4((uint32_t *)pMbsHeader, iReturn / 4);
      pLmdControl->iBytes += iReturn;
      return (LMD__SUCCESS);
   }
   // no offset table
   // do we have fragment stored?
   leftBytes = pLmdControl->iLeftWords * 2;
   if (leftBytes > 0) {
      if (leftBytes > iBytes) {
         printf("fLmdGetBuffer: stored piece of data (%u) larger than provided buffer (%u)\n",
                    leftBytes, iBytes);
         return(LMD__FAILURE);
      }

      if (pLmdControl->pMbsHeader == 0) {
         printf("fLmdGetBuffer: Internal error pMbsHeader==0\n");
         return(LMD__FAILURE);
      }

      memcpy(pMbsHeader, pLmdControl->pMbsHeader, leftBytes);
  }
  iReturn = fLmdReadBuffer(pLmdControl,(char *)pMbsHeader+leftBytes, iBytes-leftBytes);
  if(iReturn <= 0) {
    printf("fLmdGetBuffer: EOF: %s\n",pLmdControl->cFile);
    if (leftBytes>0)
       printf("fLmdGetBuffer: EOF while we have some rest data (%u)\n", leftBytes);
    else
       return(GETLMD__EOFILE);
  }

  if(iReturn > (iBytes-leftBytes)) {
      printf("fLmdGetBuffer: LMD read error %s - too many bytes read %u wants %u",
            pLmdControl->cFile, iReturn, iBytes-leftBytes);
      return(GETLMD__NOLMDFILE);
  }

  if(pLmdControl->iSwap)fLmdSwap4((uint32_t *)pMbsHeader+leftBytes/4,iReturn/4);
  pLmdControl->iBytes += iReturn;
  leftBytes += iReturn; // thats what is in the buffer
  // step through buffer to get number of elements and size
  pm=pMbsHeader;
  while(leftBytes >=8){
     if(pm->iType == LMD__TYPE_FILE_INDEX_101_2) break; // file index is last
     elem_sz = (pm->iWords+4)*2;
     if(elem_sz > leftBytes) break; // pm valid but incomplete data

     *iBytesUsed += elem_sz;
     *iElements += 1;
     pLmdControl->iElements++;
     pm = (sMbsHeader *)((char*)pm + elem_sz);
     leftBytes -= elem_sz;
  }
  //printf("Read %d bytes of %d, elements %d\n",*iBytesUsed,iBytes,*iElements);
  // fragment left? copy to internal buffer
  if(leftBytes>0){
    if(leftBytes > pLmdControl->iBufferWords*2){
      printf("fLmdGetBuffer: ERROR: internal buffer overflow. Needed:%d available:%d\n",
        leftBytes,pLmdControl->iBufferWords*2);
      return(LMD__FAILURE);
    } else {
      memcpy(pLmdControl->pBuffer,pm,leftBytes);
    }
  }
  pLmdControl->iLeftWords = leftBytes/2;
  if (pLmdControl->iLeftWords>0)
     pLmdControl->pMbsHeader = (sMbsHeader*)pLmdControl->pBuffer;
  else
     pLmdControl->pMbsHeader = 0;

  return(LMD__SUCCESS);
}
//===============================================================
uint32_t fLmdGetElement(sLmdControl *pLmdControl, uint32_t iEvent, sMbsHeader **event)
{
  sMbsHeader *pM;
  uint32_t i, evsz;
  int32_t iReturn;
  *event=NULL;

  if(iEvent == LMD__NO_INDEX) {
     if(pLmdControl->pBuffer==NULL) return(GETLMD__NOBUFFER); // internal buffer needed
     if(pLmdControl->pMbsFileHeader->iElements == 0) return(GETLMD__NOMORE);

     // check if we need to read extra data
     if ((pLmdControl->iLeftWords < 4) ||
         (pLmdControl->pMbsHeader == 0) ||
         (pLmdControl->pMbsHeader->iWords+4 > pLmdControl->iLeftWords)) {
             // first copy old data, if it exists
             if (pLmdControl->iLeftWords > 0) {
                memmove(pLmdControl->pBuffer, pLmdControl->pMbsHeader, pLmdControl->iLeftWords*2);
//                printf("copy to the begin rest %u bytes", pLmdControl->iLeftWords*2);
             }

             // second, try to read more bytes

             iReturn = fLmdReadBuffer(pLmdControl,
                    (char *)(pLmdControl->pBuffer+pLmdControl->iLeftWords),
                    (pLmdControl->iBufferWords-pLmdControl->iLeftWords)*2);

             if(iReturn <= 0) { printf("fLmdGetElement: EOF\n"); return(GETLMD__EOFILE); }

             if(pLmdControl->iSwap) fLmdSwap4((uint32_t *)(pLmdControl->pBuffer+pLmdControl->iLeftWords),iReturn/4);

             pLmdControl->iBytes += iReturn;
             pLmdControl->pMbsHeader=(sMbsHeader *)pLmdControl->pBuffer;
             pLmdControl->iLeftWords += iReturn/2;
         }

     // check if read buffer enough for event

     evsz = (pLmdControl->pMbsHeader->iWords + 4) * 2;

     if (evsz > pLmdControl->iLeftWords*2) {
        printf ("fLmdGetElement: Error, full element %u does not fit in buffer %u",
                  evsz, pLmdControl->iLeftWords*2);
        return (GETLMD__TOOBIG);
     }

     pLmdControl->pMbsFileHeader->iElements--;
     pM = pLmdControl->pMbsHeader;
     pLmdControl->pMbsHeader = (sMbsHeader *) ((char*) pM + evsz);
     pLmdControl->iLeftWords -= evsz/2;
     pLmdControl->iElements++;
     *event=pM;
     return(LMD__SUCCESS);
  }
  // get indexed event
  if(pLmdControl->iOffsetEntries){
    if(iEvent >= pLmdControl->iOffsetEntries)return(GETLMD__OUTOF_RANGE);
    fseeko64(pLmdControl->fFile,fLmdOffsetGet(pLmdControl,iEvent-1)*4,SEEK_SET);
    i=(fLmdOffsetGet(pLmdControl,iEvent)-fLmdOffsetGet(pLmdControl,iEvent-1));
    iReturn = fLmdReadBuffer(pLmdControl,(char *)pLmdControl->pBuffer,i*4);
    if(iReturn <= 0) {
       printf("fLmdGetElement: EOF\n");
       return(GETLMD__EOFILE);
    }
    if((uint32_t) iReturn != (i*4)) {
       printf("fLmdGetBuffer: LMD read error: unexpected EOF: %s\n",pLmdControl->cFile);
       return(GETLMD__EOFILE);
    }
    if(pLmdControl->iSwap)fLmdSwap4((uint32_t *)pLmdControl->pBuffer,iReturn/4);
    pLmdControl->pMbsHeader=(sMbsHeader *)pLmdControl->pBuffer;
    if((pLmdControl->pMbsHeader->iWords+4) != i*2){
      printf("fLmdGetElement: Error Event %d: size from table is %d, header %d\n",
       iEvent,i/2,pLmdControl->pMbsHeader->iWords+4);
      return(GETLMD__SIZE_ERROR);
    }
    pLmdControl->iBytes+=iReturn;
    *event=pLmdControl->pMbsHeader;
    return(LMD__SUCCESS);
  }
  else return(GETLMD__NOMORE);
  // return zero if no more events
}
//===============================================================
uint32_t fLmdGetClose(sLmdControl *pLmdControl)
{
   fLmdCleanup(pLmdControl); // cleanup except fFile
   if(fclose(pLmdControl->fFile) != 0) {
      pLmdControl->fFile=NULL;
      return(LMD__CLOSE_ERR);
   }
   pLmdControl->fFile=NULL;
   return(LMD__SUCCESS);
}
//===============================================================
int32_t fLmdReadBuffer(sLmdControl *pLmdControl, char *buffer, uint32_t bytes){
  int32_t IObytes;
  IObytes=(int32_t)fread(buffer,1,bytes,pLmdControl->fFile);
  //if(IObytes < bytes) printf("Read %s: request %d bytes, got %d\n",pLmdControl->cFile,bytes,IObytes);
  return(IObytes);
}
//===============================================================
int32_t fLmdWriteBuffer(sLmdControl *pLmdControl, char *buffer, uint32_t bytes){
  int32_t IObytes;
  IObytes=(int32_t)fwrite(buffer,1,bytes,pLmdControl->fFile);
  //if(IObytes < bytes) printf("Write %s: request %d bytes, put %d\n",
  // pLmdControl->cFile,bytes,IObytes);
  return(IObytes);
}
//===============================================================
uint64_t fLmdGetBytesWritten(sLmdControl *pLmdControl)
{
   uint64_t bytes;
   bytes=pLmdControl->iBytes;
   // add pending data size in current buffer
    if(pLmdControl->iBufferWords > pLmdControl->iLeftWords)
    bytes += (pLmdControl->iBufferWords - pLmdControl->iLeftWords)*2;
    // add table size which will be written at close
   if ((pLmdControl->pOffset4!=NULL)||(pLmdControl->pOffset8!=NULL))
     bytes += (pLmdControl->iElements+1)*pLmdControl->iOffsetSize;
   return bytes;
}
//===============================================================
uint32_t fLmdCleanup(sLmdControl *pLmdControl)
{
   // do not clean fFile
   if(pLmdControl->pTCP     != NULL)free(pLmdControl->pTCP);
   if(pLmdControl->cHeader  != NULL)free(pLmdControl->cHeader);
   if(pLmdControl->pOffset4 != NULL)free(pLmdControl->pOffset4);
   if(pLmdControl->pOffset8 != NULL)free(pLmdControl->pOffset8);
   if((pLmdControl->pBuffer  != NULL) && (pLmdControl->iInternBuffer>0))
      free(pLmdControl->pBuffer);
   if((pLmdControl->pMbsFileHeader != NULL) && (pLmdControl->iInternHeader>0))
      free(pLmdControl->pMbsFileHeader);
   pLmdControl->pTCP=NULL;
   pLmdControl->cHeader=NULL;
   pLmdControl->pBuffer=NULL;
   pLmdControl->iInternBuffer=0;
   pLmdControl->pOffset4=NULL;
   pLmdControl->pOffset8=NULL;
   pLmdControl->pMbsFileHeader=NULL;
   pLmdControl->iInternHeader=0;
   pLmdControl->pMbsHeader=NULL;
   return (LMD__SUCCESS);
}
//===============================================================
// can be called after GetOpen or ConnectMbs
uint32_t fLmdGetSwap(sLmdControl *pLmdControl)
{
   if(pLmdControl != NULL)
      return pLmdControl->iSwap;
   return (uint32_t) -1;
}
//===============================================================
// can be called after PutOpen or before PutClose
void fLmdSetWrittenEndian(sLmdControl *pLmdControl,uint32_t iE)
{
   if(pLmdControl->pMbsFileHeader != NULL)
      pLmdControl->pMbsFileHeader->iWrittenEndian=iE;
   else
      printf("fLmdSetWrittenEndian: No file header allocated!\n");
}
//===============================================================
// can be called after GetOpen or GetMbsEvent
uint32_t   fLmdGetWrittenEndian(sLmdControl *pLmdControl)
{
   if(pLmdControl->pMbsFileHeader != NULL)
      return(pLmdControl->pMbsFileHeader->iWrittenEndian);

   printf("fLmdGetWrittenEndian: No file header allocated!\n");
   return(LMD__ENDIAN_UNKNOWN);
}
//===============================================================
sLmdControl * fLmdAllocateControl(void)
{
  sLmdControl *x;
  x = (sLmdControl *)malloc(sizeof(sLmdControl));
  if (x) memset(x,0,sizeof(sLmdControl));
  return x;
}
//===============================================================
void fLmdOffsetElements(sLmdControl *pLmdControl, uint32_t bytes, uint32_t *elements, uint32_t *used)
{
  lmdoff_t *off1,*off2;
  uint32_t elem=0,i,*iff1,*iff2;

  if(pLmdControl->iOffsetSize == 4){
    iff1=pLmdControl->pOffset4+pLmdControl->iElements;
    iff2=iff1;
    for(i=pLmdControl->iElements;i<pLmdControl->iOffsetEntries-1;i++){
      if((*(iff1+1)-*iff2)>bytes/4) break;
      iff1++;
      elem++;
      pLmdControl->iElements++;
    }
    *used=(*iff1-*iff2)*4;
    *elements=elem;
  }
  else if(pLmdControl->iOffsetSize == 8){
    off1=pLmdControl->pOffset8+pLmdControl->iElements;
    off2=off1;
    for(i=pLmdControl->iElements;i<pLmdControl->iOffsetEntries-1;i++){
      if((*(off1+1)-*off2)>bytes/4) break;
      off1++;
      elem++;
      pLmdControl->iElements++;
    }
    *used=(*off1-*off2)*4;
    *elements=elem;
  }
}
//===============================================================
uint32_t fLmdOffsetRead(sLmdControl *pLmdControl)
{
  int32_t iReturn;
  sMbsHeader *pTableHead;

  pTableHead=(sMbsHeader *)malloc(16); // header with 8 bytes data for future use.
  fseeko64(pLmdControl->fFile,(lmdoff_t)pLmdControl->pMbsFileHeader->iTableOffset*4,SEEK_SET);
  iReturn = fLmdReadBuffer(pLmdControl, (char *)pTableHead,16);
    if(iReturn!=16) {
      printf("fLmdGetBuffer: LMD read error: unexpected EOF: %s\n",pLmdControl->cFile);
      free(pTableHead);
      return(GETLMD__NOLMDFILE);
    }
    if(pLmdControl->iSwap)fLmdSwap4((uint32_t *)pTableHead,4);
  if(pTableHead->iType != LMD__TYPE_FILE_INDEX_101_2){
    printf("fLmdOffsetTable: LMD format error: no index table: %s, type %0x\n",
      pLmdControl->cFile,pTableHead->iType);
    free(pTableHead);
    return(GETLMD__NOLMDFILE);
  }
  //printf("Table: words:%d type:%08x\n",pTableHead->iWords,pTableHead->iType);
  free(pTableHead);
  pLmdControl->iOffsetEntries=pLmdControl->pMbsFileHeader->iElements+1;
  pLmdControl->pOffset8=(lmdoff_t *)malloc(pLmdControl->iOffsetEntries*pLmdControl->iOffsetSize);
  iReturn = fLmdReadBuffer(pLmdControl,
                           (char *)pLmdControl->pOffset8,
                           pLmdControl->iOffsetEntries*pLmdControl->iOffsetSize);
  if((uint32_t) iReturn != pLmdControl->iOffsetEntries*pLmdControl->iOffsetSize) {
    printf("fLmdOffsetTable: LMD format error: no index table: %s\n",pLmdControl->cFile);
    pLmdControl->iOffsetEntries = 0;
    return(GETLMD__NOLMDFILE);
  }
  if(pLmdControl->iSwap){
    fLmdSwap4((uint32_t *)pLmdControl->pOffset8,iReturn/4);
    if(pLmdControl->iOffsetSize == 8)
    fLmdSwap8((uint64_t *)pLmdControl->pOffset8,iReturn/8);
  }
  // go back behing header
  fseeko64(pLmdControl->fFile,(lmdoff_t)sizeof(sMbsFileHeader),SEEK_SET);
  // use small table
  if(pLmdControl->iOffsetSize == 4){
    pLmdControl->pOffset4= (uint32_t *)pLmdControl->pOffset8;
    pLmdControl->pOffset8=NULL;
  }
  return(LMD__SUCCESS);
}
//===============================================================
uint32_t fLmdOffsetWrite(sLmdControl *pLmdControl)
{
  int32_t iReturn;
  char *pbuf;
  lmdoff_t current;
  sMbsHeader *pTableHead;
  pTableHead = (sMbsHeader *)malloc(16); // header with 8 bytes data for future use.
  if (!pTableHead) {
     printf("fLmdOffsetWrite: memory allocation error\n");
     return (LMD__FAILURE);
  }
  memset(pTableHead,0,16);
  pTableHead->iWords=(pLmdControl->iElements+1)*pLmdControl->iOffsetSize/2+4;
  pTableHead->iType=LMD__TYPE_FILE_INDEX_101_2;
/*   printf("Table: words:%d type:%08x offbytes:%d\n", */
/*     pTableHead->iWords,pTableHead->iType,pLmdControl->iOffsetSize); */
  iReturn = fgetpos64(pLmdControl->fFile,(fpos64_t *) &current);
  iReturn = fLmdWriteBuffer(pLmdControl, (char *)pTableHead,16);
  free(pTableHead);
  pbuf=(char *)pLmdControl->pOffset4; // try short table
  if(pbuf == NULL) pbuf=(char *)pLmdControl->pOffset8;
  iReturn = fLmdWriteBuffer(pLmdControl, pbuf,
        (pLmdControl->iElements+1)*pLmdControl->iOffsetSize);
  if(pLmdControl->pOffset8)
    pLmdControl->pMbsFileHeader->iTableOffset = *(pLmdControl->pOffset8+pLmdControl->iElements);
  if(pLmdControl->pOffset4)
     pLmdControl->pMbsFileHeader->iTableOffset = *(pLmdControl->pOffset4 + pLmdControl->iElements);
  if (current / 4 != pLmdControl->pMbsFileHeader->iTableOffset) {
     printf("Table offset mismatch: current:%llu calculated:%llu, cur-cal %llu\n", (long long unsigned)current / 4,
            (long long unsigned)pLmdControl->pMbsFileHeader->iTableOffset,
            (long long unsigned)(current / 4 - pLmdControl->pMbsFileHeader->iTableOffset));
     return (LMD__FAILURE);
  }
  if((uint32_t) iReturn != (pLmdControl->iElements+1)*pLmdControl->iOffsetSize){
    printf("Table write error \n");
    return(LMD__FAILURE);
  }
  return(LMD__SUCCESS);
}
//===============================================================
uint32_t fLmdOffsetSet(sLmdControl *pLmdControl, uint32_t lwords)
{
  if(pLmdControl->iElements >= pLmdControl->iOffsetEntries)fLmdOffsetResize(pLmdControl,0);
  if(pLmdControl->pOffset8){
    *(pLmdControl->pOffset8+pLmdControl->iElements)=
    *(pLmdControl->pOffset8+pLmdControl->iElements-1)+(lmdoff_t)lwords;
  }
  if(pLmdControl->pOffset4){
    *(pLmdControl->pOffset4+pLmdControl->iElements)=
    *(pLmdControl->pOffset4+pLmdControl->iElements-1)+lwords;
  }
  return(LMD__SUCCESS);
}
//===============================================================
lmdoff_t fLmdOffsetGet(sLmdControl *pLmdControl, uint32_t index)
{
   if (pLmdControl->pOffset8)
      return (*(pLmdControl->pOffset8 + index));
   if (pLmdControl->pOffset4)
      return ((lmdoff_t) * (pLmdControl->pOffset4 + index));
   return 0;
}
//===============================================================
void fLmdOffsetResize(sLmdControl *pLmdControl, uint32_t firstValue){
  lmdoff_t *new;
  uint32_t oldEntries,newEntries;

  oldEntries=pLmdControl->iOffsetEntries;
  newEntries=oldEntries+OFFSET__ENTRIES;
  new=(lmdoff_t *)malloc(newEntries*pLmdControl->iOffsetSize);
  memset(new,0,newEntries*pLmdControl->iOffsetSize);
  if(oldEntries > 0){ //table was expanded
    //printf("Resize table %d to %d entries\n",oldEntries,newEntries);
    if(pLmdControl->pOffset8){
      memcpy(new,pLmdControl->pOffset8,oldEntries*pLmdControl->iOffsetSize);
      free(pLmdControl->pOffset8);
      pLmdControl->pOffset8=new;
    }
    if(pLmdControl->pOffset4){
      memcpy(new,pLmdControl->pOffset4,oldEntries*pLmdControl->iOffsetSize);
      free(pLmdControl->pOffset4);
      pLmdControl->pOffset4=(uint32_t *)new;
    }
  }
  else { // table was new
    //printf("Create table %d entries, first offset %d\n",newEntries,firstValue);
    if(pLmdControl->iOffsetSize==8){
      pLmdControl->pOffset8=new;
      *pLmdControl->pOffset8=(lmdoff_t)firstValue;
    }
    if(pLmdControl->iOffsetSize==4){
      pLmdControl->pOffset4=(uint32_t *)new;
      *pLmdControl->pOffset4=firstValue;
    }
  }
  pLmdControl->iOffsetEntries=newEntries;
}
//===============================================================
void fLmdPrintBufferHeader(uint32_t iVerbose, sMbsBufferHeader *pMbsBufferHeader)
{
   if (iVerbose) {
      if (pMbsBufferHeader) {
         printf("BfHd: # %d, DataWords:%d Type:%08x Elements:%d sec:%d.%d MaxWords:%d\n", pMbsBufferHeader->iBuffer,
                pMbsBufferHeader->iUsedWords, pMbsBufferHeader->iType, pMbsBufferHeader->iElements,
                pMbsBufferHeader->iTimeSpecSec, pMbsBufferHeader->iTimeSpecNanoSec / 1000, pMbsBufferHeader->iMaxWords);
      }
   }
}
//===============================================================
void fLmdPrintFileHeader(uint32_t iVerbose, sMbsFileHeader *pMbsFileHeader)
{
   if (iVerbose) {
      if (pMbsFileHeader) {
         printf("FiHd: DataWords:%d Type:%d.%d Elements:%d sec:%d.%d MaxWords:%d Index: %llx[%d]\n",
                pMbsFileHeader->iUsedWords, pMbsFileHeader->iType & 0xffff, pMbsFileHeader->iType >> 16,
                pMbsFileHeader->iElements, pMbsFileHeader->iTimeSpecSec, pMbsFileHeader->iTimeSpecNanoSec / 1000,
                pMbsFileHeader->iMaxWords, (long long unsigned) pMbsFileHeader->iTableOffset, pMbsFileHeader->iOffsetSize);
      }
   }
}
//===============================================================
void fLmdPrintHeader(uint32_t iVerbose, sMbsHeader *pMbsHeader)
{
   if (iVerbose) {
      if (pMbsHeader) {
         printf("ElHd: words:%d type:%08x\n", pMbsHeader->iWords, pMbsHeader->iType);
      }
   }
}
//===============================================================
void fLmdPrintEvent(uint32_t iVerbose, sMbsEventHeader *pMbsEventHeader)
{
   if (iVerbose) {
      if (pMbsEventHeader) {
         printf("EvHd: words:%6d type:%08x trigger:%2d #:%4d\n", pMbsEventHeader->iWords, pMbsEventHeader->iType,
                pMbsEventHeader->iTrigger >> 16, pMbsEventHeader->iEventNumber);
      }
   }
}
//===============================================================
void fLmdPrintControl(uint32_t iVerbose, sLmdControl *pLmdControl)
{
   if (iVerbose) {
      printf("Ctrl: file:%s words:%d left:%d bytes read:%llu elements:%d\n", pLmdControl->cFile,
             pLmdControl->iBufferWords, pLmdControl->iLeftWords, (long long unsigned) pLmdControl->iBytes, pLmdControl->iElements);
      fLmdPrintFileHeader(iVerbose, pLmdControl->pMbsFileHeader);
      fLmdPrintEvent(iVerbose, (sMbsEventHeader *)pLmdControl->pMbsHeader);
   }
}
//===============================================================
void fLmdSwap4(uint32_t *array, uint32_t items)
{
   uint32_t i, *pp;
   pp = array;
   for (i = 0; i < items; i++) {
      *pp = (*pp >> 24) + ((*pp >> 8) & 0x0000ff00) + ((*pp << 8) & 0x00ff0000) + (*pp << 24);
      pp++;
   }
}
//===============================================================
void fLmdSwap8(uint64_t *array, uint32_t items)
{
   uint64_t *pp;
   uint32_t i;
   pp = array;
   for (i = 0; i < items; i++) {
      *pp = (*pp << 32) + (*pp >> 32);
      pp++;
   }
}
