/* 7zDec.c -- Decoding from 7z folder
2019-02-02 : Igor Pavlov : Public domain */

#include "Precomp.h"

#include <string.h>

/* #define _7ZIP_PPMD_SUPPPORT */

#include "7z.h"
#include "7zCrc.h"

#include "Bcj2.h"
#include "7zFile.h"

#include "CpuArch.h"
#include "LzmaDec.h"
#include "Lzma2Dec.h"
#ifdef _7ZIP_PPMD_SUPPPORT
#include "Ppmd7.h"
#endif

#define k_Copy 0
#define k_Delta 3
#define k_LZMA2 0x21
#define k_LZMA  0x30101
#define k_BCJ   0x3030103
#define k_BCJ2  0x303011B
#define k_PPC   0x3030205
#define k_IA64  0x3030401
#define k_ARM   0x3030501
#define k_ARMT  0x3030701
#define k_SPARC 0x3030805

#define OUT_BUF_SIZE (1 << 20)

#ifdef _7ZIP_PPMD_SUPPPORT

#define k_PPMD 0x30401

typedef struct
{
  IByteIn vt;
  const Byte *cur;
  const Byte *end;
  const Byte *begin;
  UInt64 processed;
  BoolInt extra;
  SRes res;
  const ILookInStream *inStream;
} CByteInToLook;

static Byte ReadByte(const IByteIn *pp)
{
  CByteInToLook *p = CONTAINER_FROM_VTBL(pp, CByteInToLook, vt);
  if (p->cur != p->end)
    return *p->cur++;
  if (p->res == SZ_OK)
  {
    size_t size = p->cur - p->begin;
    p->processed += size;
    p->res = ILookInStream_Skip(p->inStream, size);
    size = (1 << 25);
    p->res = ILookInStream_Look(p->inStream, (const void **)&p->begin, &size);
    p->cur = p->begin;
    p->end = p->begin + size;
    if (size != 0)
      return *p->cur++;;
  }
  p->extra = True;
  return 0;
}

static SRes SzDecodePpmd(const Byte *props, unsigned propsSize, UInt64 inSize, const ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAllocPtr allocMain)
{
  CPpmd7 ppmd;
  CByteInToLook s;
  SRes res = SZ_OK;

  s.vt.Read = ReadByte;
  s.inStream = inStream;
  s.begin = s.end = s.cur = NULL;
  s.extra = False;
  s.res = SZ_OK;
  s.processed = 0;

  if (propsSize != 5)
    return SZ_ERROR_UNSUPPORTED;

  {
    unsigned order = props[0];
    UInt32 memSize = GetUi32(props + 1);
    if (order < PPMD7_MIN_ORDER ||
        order > PPMD7_MAX_ORDER ||
        memSize < PPMD7_MIN_MEM_SIZE ||
        memSize > PPMD7_MAX_MEM_SIZE)
      return SZ_ERROR_UNSUPPORTED;
    Ppmd7_Construct(&ppmd);
    if (!Ppmd7_Alloc(&ppmd, memSize, allocMain))
      return SZ_ERROR_MEM;
    Ppmd7_Init(&ppmd, order);
  }
  {
    CPpmd7z_RangeDec rc;
    Ppmd7z_RangeDec_CreateVTable(&rc);
    rc.Stream = &s.vt;
    if (!Ppmd7z_RangeDec_Init(&rc))
      res = SZ_ERROR_DATA;
    else if (s.extra)
      res = (s.res != SZ_OK ? s.res : SZ_ERROR_DATA);
    else
    {
      SizeT i;
      for (i = 0; i < outSize; i++)
      {
        int sym = Ppmd7_DecodeSymbol(&ppmd, &rc.vt);
        if (s.extra || sym < 0)
          break;
        outBuffer[i] = (Byte)sym;
      }
      if (i != outSize)
        res = (s.res != SZ_OK ? s.res : SZ_ERROR_DATA);
      else if (s.processed + (s.cur - s.begin) != inSize || !Ppmd7z_RangeDec_IsFinishedOK(&rc))
        res = SZ_ERROR_DATA;
    }
  }
  Ppmd7_Free(&ppmd, allocMain);
  return res;
}

#endif


static SRes SzDecodeLzma(const Byte *props, unsigned propsSize, UInt64 inSize, ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAllocPtr allocMain)
{
  CLzmaDec state;
  SRes res = SZ_OK;

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_AllocateProbs(&state, props, propsSize, allocMain));
  state.dic = outBuffer;
  state.dicBufSize = outSize;
  LzmaDec_Init(&state);

  for (;;)
  {
    const void *inBuf = NULL;
    size_t lookahead = (1 << 18);
    if (lookahead > inSize)
      lookahead = (size_t)inSize;
    res = ILookInStream_Look(inStream, &inBuf, &lookahead);
    if (res != SZ_OK)
      break;

    {
      SizeT inProcessed = (SizeT)lookahead, dicPos = state.dicPos;
      ELzmaStatus status;
      res = LzmaDec_DecodeToDic(&state, outSize, (const Byte *)inBuf, &inProcessed, LZMA_FINISH_END, &status);
      lookahead -= inProcessed;
      inSize -= inProcessed;
      if (res != SZ_OK)
        break;

      if (status == LZMA_STATUS_FINISHED_WITH_MARK)
      {
        if (outSize != state.dicPos || inSize != 0)
          res = SZ_ERROR_DATA;
        break;
      }

      if (outSize == state.dicPos && inSize == 0 && status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK)
        break;

      if (inProcessed == 0 && dicPos == state.dicPos)
      {
        res = SZ_ERROR_DATA;
        break;
      }

      res = ILookInStream_Skip(inStream, inProcessed);
      if (res != SZ_OK)
        break;
    }
  }

  LzmaDec_FreeProbs(&state, allocMain);
  return res;
}

static SRes SzDecodeLzmaStream(const Byte *props, unsigned propsSize, UInt64 packedSize, ILookInStream *inStream,
                               SizeT unpackSize, SzArEx_StreamOutEntry *streamOutInfo, UInt32 folderIndex,
                               ISzAllocPtr allocMain, UnpackProgressCallback callbackProgress, UnpackCurFileCallback curFile)
{
    CLzmaDec state;
    UInt32 fileIndex = 0;
    UInt32 CRC = CRC_INIT_VAL;

    LzmaDec_Construct(&state);
    RINOK(LzmaDec_Allocate(&state, props, propsSize, allocMain));
    LzmaDec_Init(&state);

    Byte *outBuf = (Byte *)ISzAlloc_Alloc(allocMain, OUT_BUF_SIZE);

    if (curFile)
        curFile(streamOutInfo);

    const void *inBuf = NULL;
    SRes res = SZ_OK;
    size_t inPos = 0, inSize = 0;
    size_t offsetStreamOut = 0;
    for (;;)
    {
        if (inPos == inSize)
        {
            size_t lookahead = (1 << 18);
            if (lookahead > packedSize)
                lookahead = (size_t)packedSize;
            res = ILookInStream_Look(inStream, &inBuf, &lookahead);
            if (res != SZ_OK)
                break;
            inSize = lookahead;
            inPos = 0;
        }
        SizeT outBufferSize = OUT_BUF_SIZE;
        if ((offsetStreamOut + outBufferSize) > streamOutInfo[fileIndex].UnpackSize)
        {
            outBufferSize = streamOutInfo[fileIndex].UnpackSize - offsetStreamOut;
        }
        SizeT inProcessed = inSize - inPos;
        SizeT outProcessed = outBufferSize;
        ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
        ELzmaStatus status;

        if (outProcessed > unpackSize)
        {
            outProcessed = (SizeT)unpackSize;
            finishMode = LZMA_FINISH_END;
        }

        res = LzmaDec_DecodeToBuf(&state, outBuf, &outProcessed,
                                  inBuf + inPos, &inProcessed, finishMode, &status);
        if (res != SZ_OK || (unpackSize == 0))
            break;

        if (inProcessed == 0 && outProcessed == 0)
        {
            if (status != LZMA_STATUS_FINISHED_WITH_MARK)
                res = SZ_ERROR_DATA;
            break;
        }

#ifdef USE_WINDOWS_FILE
        if (streamOutInfo[fileIndex].outStream.file.handle != INVALID_HANDLE_VALUE)
#else
        if (streamOutInfo[fileIndex].outStream.file.file != NULL)
#endif
        {
            if (streamOutInfo[fileIndex].outStream.vt.Write(&streamOutInfo[fileIndex].outStream.vt, outBuf, outProcessed) != outProcessed)
            {
                res = SZ_ERROR_WRITE;
                break;
            }
        }

        inPos += inProcessed;
        packedSize -= inProcessed;
        unpackSize -= outProcessed;
        offsetStreamOut += outProcessed;

        if (offsetStreamOut > streamOutInfo[fileIndex].UnpackSize)
        {
            res = SZ_ERROR_DATA;
            break;
        }

        if (callbackProgress)
            callbackProgress(outProcessed);

        CRC = CrcUpdate(CRC, outBuf, outProcessed);
        // check for end of output stream
        if (offsetStreamOut == streamOutInfo[fileIndex].UnpackSize)
        {
            CRC = CRC_GET_DIGEST(CRC);
            if (CRC != streamOutInfo[fileIndex].CRC)
            {
                res = SZ_ERROR_CRC;
                break;
            }
            CRC = CRC_INIT_VAL;
            File_Close(&streamOutInfo[fileIndex].outStream.file);
            fileIndex++;
            // check for end of input stream
            if (streamOutInfo[fileIndex].folderIndex != folderIndex)
            {
                if (unpackSize != 0)
                    res = SZ_ERROR_DATA;
                break;
            }
            offsetStreamOut = 0;
            if (curFile)
                curFile(&streamOutInfo[fileIndex]);
        }

        res = ILookInStream_Skip(inStream, inProcessed);
        if (res != SZ_OK)
            break;
    }

    ISzAlloc_Free(allocMain, outBuf);
    LzmaDec_Free(&state, allocMain);

    return res;
}

#ifndef _7Z_NO_METHOD_LZMA2

static SRes SzDecodeLzma2(const Byte *props, unsigned propsSize, UInt64 inSize, ILookInStream *inStream,
    Byte *outBuffer, SizeT outSize, ISzAllocPtr allocMain)
{
  CLzma2Dec state;
  SRes res = SZ_OK;

  Lzma2Dec_Construct(&state);
  if (propsSize != 1)
    return SZ_ERROR_DATA;
  RINOK(Lzma2Dec_AllocateProbs(&state, props[0], allocMain));
  state.decoder.dic = outBuffer;
  state.decoder.dicBufSize = outSize;
  Lzma2Dec_Init(&state);

  for (;;)
  {
    const void *inBuf = NULL;
    size_t lookahead = (1 << 18);
    if (lookahead > inSize)
      lookahead = (size_t)inSize;
    res = ILookInStream_Look(inStream, &inBuf, &lookahead);
    if (res != SZ_OK)
      break;

    {
      SizeT inProcessed = (SizeT)lookahead, dicPos = state.decoder.dicPos;
      ELzmaStatus status;
      res = Lzma2Dec_DecodeToDic(&state, outSize, (const Byte *)inBuf, &inProcessed, LZMA_FINISH_END, &status);
      lookahead -= inProcessed;
      inSize -= inProcessed;
      if (res != SZ_OK)
        break;

      if (status == LZMA_STATUS_FINISHED_WITH_MARK)
      {
        if (outSize != state.decoder.dicPos || inSize != 0)
          res = SZ_ERROR_DATA;
        break;
      }

      if (inProcessed == 0 && dicPos == state.decoder.dicPos)
      {
        res = SZ_ERROR_DATA;
        break;
      }

      res = ILookInStream_Skip(inStream, inProcessed);
      if (res != SZ_OK)
        break;
    }
  }

  Lzma2Dec_FreeProbs(&state, allocMain);
  return res;
}

static SRes SzDecodeLzma2Stream(const Byte *props, unsigned propsSize, UInt64 packedSize, ILookInStream *inStream,
                                SizeT unpackSize, SzArEx_StreamOutEntry *streamOutInfo, UInt32 folderIndex,
                                ISzAllocPtr allocMain, UnpackProgressCallback callbackProgress, UnpackCurFileCallback curFile)
{
    CLzma2Dec state;
    UInt32 fileIndex = 0;
    UInt32 CRC = CRC_INIT_VAL;

    Lzma2Dec_Construct(&state);
    if (propsSize != 1)
        return SZ_ERROR_DATA;
    RINOK(Lzma2Dec_Allocate(&state, props[0], allocMain));
    Lzma2Dec_Init(&state);

    Byte *outBuf = (Byte *)ISzAlloc_Alloc(allocMain, OUT_BUF_SIZE);

    if (curFile)
        curFile(streamOutInfo);

    const void *inBuf = NULL;
    SRes res = SZ_OK;
    size_t inPos = 0, inSize = 0;
    size_t offsetStreamOut = 0;
    for (;;)
    {
        if (inPos == inSize)
        {
            size_t lookahead = (1 << 18);
            if (lookahead > packedSize)
                lookahead = (size_t)packedSize;
            res = ILookInStream_Look(inStream, &inBuf, &lookahead);
            if (res != SZ_OK)
                break;
            inSize = lookahead;
            inPos = 0;
        }
        SizeT outBufferSize = OUT_BUF_SIZE;
        if ((offsetStreamOut + outBufferSize) > streamOutInfo[fileIndex].UnpackSize)
        {
            outBufferSize = streamOutInfo[fileIndex].UnpackSize - offsetStreamOut;
        }
        SizeT inProcessed = inSize - inPos;
        SizeT outProcessed = outBufferSize;
        ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
        ELzmaStatus status;

        if (outProcessed > unpackSize)
        {
            outProcessed = (SizeT)unpackSize;
            finishMode = LZMA_FINISH_END;
        }

        res = Lzma2Dec_DecodeToBuf(&state, outBuf, &outProcessed,
                                   inBuf + inPos, &inProcessed, finishMode, &status);
        if (res != SZ_OK || (unpackSize == 0))
            break;

        if (inProcessed == 0 && outProcessed == 0)
        {
            if (status != LZMA_STATUS_FINISHED_WITH_MARK)
                res = SZ_ERROR_DATA;
            break;
        }

#ifdef USE_WINDOWS_FILE
        if (streamOutInfo[fileIndex].outStream.file.handle != INVALID_HANDLE_VALUE)
#else
        if (streamOutInfo[fileIndex].outStream.file.file != NULL)
#endif
        {
            if (streamOutInfo[fileIndex].outStream.vt.Write(&streamOutInfo[fileIndex].outStream.vt, outBuf, outProcessed) != outProcessed)
            {
                res = SZ_ERROR_WRITE;
                break;
            }
        }

        inPos += inProcessed;
        packedSize -= inProcessed;
        unpackSize -= outProcessed;
        offsetStreamOut += outProcessed;

        if (offsetStreamOut > streamOutInfo[fileIndex].UnpackSize)
        {
            res = SZ_ERROR_DATA;
            break;
        }

        if (callbackProgress)
            callbackProgress(outProcessed);

        CRC = CrcUpdate(CRC, outBuf, outProcessed);
        // check for end of output stream
        if (offsetStreamOut == streamOutInfo[fileIndex].UnpackSize)
        {
            CRC = CRC_GET_DIGEST(CRC);
            if (CRC != streamOutInfo[fileIndex].CRC)
            {
                res = SZ_ERROR_CRC;
                break;
            }
            CRC = CRC_INIT_VAL;
            File_Close(&streamOutInfo[fileIndex].outStream.file);
            fileIndex++;
            // check for end of input stream
            if (streamOutInfo[fileIndex].folderIndex != folderIndex)
            {
                if (unpackSize != 0)
                    res = SZ_ERROR_DATA;
                break;
            }
            offsetStreamOut = 0;
            if (curFile)
                curFile(&streamOutInfo[fileIndex]);
        }

        res = ILookInStream_Skip(inStream, inProcessed);
        if (res != SZ_OK)
            break;
    }

    ISzAlloc_Free(allocMain, outBuf);
    Lzma2Dec_Free(&state, allocMain);

    return res;
}

#endif


static SRes SzDecodeCopy(UInt64 inSize, ILookInStream *inStream, Byte *outBuffer)
{
  while (inSize > 0)
  {
    const void *inBuf;
    size_t curSize = (1 << 18);
    if (curSize > inSize)
      curSize = (size_t)inSize;
    RINOK(ILookInStream_Look(inStream, &inBuf, &curSize));
    if (curSize == 0)
      return SZ_ERROR_INPUT_EOF;
    memcpy(outBuffer, inBuf, curSize);
    outBuffer += curSize;
    inSize -= curSize;
    RINOK(ILookInStream_Skip(inStream, curSize));
  }
  return SZ_OK;
}

static SRes SzDecodeCopyStream(UInt64 inSize, ILookInStream *inStream, SzArEx_StreamOutEntry *streamOutInfo,
                               UInt32 folderIndex, UnpackProgressCallback callbackProgress, UnpackCurFileCallback curFile)
{
    size_t offsetStreamOut = 0;
    UInt32 fileIndex = 0;
    UInt32 CRC = CRC_INIT_VAL;

    if (curFile)
        curFile(streamOutInfo);

    while (inSize > 0)
    {
        const void *inBuf;
        size_t curSize = (1 << 18);
        if (curSize > inSize)
            curSize = (size_t)inSize;
        RINOK(ILookInStream_Look(inStream, &inBuf, &curSize));
        if (curSize == 0)
            return SZ_ERROR_INPUT_EOF;
        if ((offsetStreamOut + curSize) > streamOutInfo[fileIndex].UnpackSize)
        {
            curSize = streamOutInfo[fileIndex].UnpackSize - offsetStreamOut;
        }
#ifdef USE_WINDOWS_FILE
        if (streamOutInfo[fileIndex].outStream.file.handle != INVALID_HANDLE_VALUE)
#else
        if (streamOutInfo[fileIndex].outStream.file.file != NULL)
#endif
        {
            if (streamOutInfo[fileIndex].outStream.vt.Write(&streamOutInfo[fileIndex].outStream.vt, inBuf, curSize) != curSize)
            {
                return SZ_ERROR_WRITE;
            }
        }
        inSize -= curSize;
        offsetStreamOut += curSize;

        if (offsetStreamOut > streamOutInfo[fileIndex].UnpackSize)
        {
            return SZ_ERROR_DATA;
        }

        if (callbackProgress)
            callbackProgress(curSize);

        CRC = CrcUpdate(CRC, inBuf, curSize);
        // check for end of output stream
        if (offsetStreamOut == streamOutInfo[fileIndex].UnpackSize)
        {
            CRC = CRC_GET_DIGEST(CRC);
            if (CRC != streamOutInfo[fileIndex].CRC)
            {
                return SZ_ERROR_CRC;
            }
            CRC = CRC_INIT_VAL;
            File_Close(&streamOutInfo[fileIndex].outStream.file);
            fileIndex++;
            // check for end of input stream
            if (streamOutInfo[fileIndex].folderIndex != folderIndex)
            {
                if (inSize != 0)
                    return SZ_ERROR_DATA;
                break;
            }
            offsetStreamOut = 0;
            if (curFile)
                curFile(&streamOutInfo[fileIndex]);
        }

        RINOK(ILookInStream_Skip(inStream, curSize));
    }
    return SZ_OK;
}

static BoolInt IS_MAIN_METHOD(UInt32 m)
{
  switch (m)
  {
    case k_Copy:
    case k_LZMA:
    #ifndef _7Z_NO_METHOD_LZMA2
    case k_LZMA2:
    #endif
    #ifdef _7ZIP_PPMD_SUPPPORT
    case k_PPMD:
    #endif
      return True;
  }
  return False;
}

static BoolInt IS_SUPPORTED_CODER(const CSzCoderInfo *c)
{
  return
      c->NumStreams == 1
      /* && c->MethodID <= (UInt32)0xFFFFFFFF */
      && IS_MAIN_METHOD((UInt32)c->MethodID);
}

#define IS_BCJ2(c) ((c)->MethodID == k_BCJ2 && (c)->NumStreams == 4)

static SRes CheckSupportedFolder(const CSzFolder *f)
{
  if (f->NumCoders < 1 || f->NumCoders > 4)
    return SZ_ERROR_UNSUPPORTED;
  if (!IS_SUPPORTED_CODER(&f->Coders[0]))
    return SZ_ERROR_UNSUPPORTED;
  if (f->NumCoders == 1)
  {
    if (f->NumPackStreams != 1 || f->PackStreams[0] != 0 || f->NumBonds != 0)
      return SZ_ERROR_UNSUPPORTED;
    return SZ_OK;
  }
  
  
  #ifndef _7Z_NO_METHODS_FILTERS

  if (f->NumCoders == 2)
  {
    const CSzCoderInfo *c = &f->Coders[1];
    if (
        /* c->MethodID > (UInt32)0xFFFFFFFF || */
        c->NumStreams != 1
        || f->NumPackStreams != 1
        || f->PackStreams[0] != 0
        || f->NumBonds != 1
        || f->Bonds[0].InIndex != 1
        || f->Bonds[0].OutIndex != 0)
      return SZ_ERROR_UNSUPPORTED;
    switch ((UInt32)c->MethodID)
    {
      case k_Delta:
      case k_BCJ:
      case k_PPC:
      case k_IA64:
      case k_SPARC:
      case k_ARM:
      case k_ARMT:
        break;
      default:
        return SZ_ERROR_UNSUPPORTED;
    }
    return SZ_OK;
  }

  #endif

  
  if (f->NumCoders == 4)
  {
    if (!IS_SUPPORTED_CODER(&f->Coders[1])
        || !IS_SUPPORTED_CODER(&f->Coders[2])
        || !IS_BCJ2(&f->Coders[3]))
      return SZ_ERROR_UNSUPPORTED;
    if (f->NumPackStreams != 4
        || f->PackStreams[0] != 2
        || f->PackStreams[1] != 6
        || f->PackStreams[2] != 1
        || f->PackStreams[3] != 0
        || f->NumBonds != 3
        || f->Bonds[0].InIndex != 5 || f->Bonds[0].OutIndex != 0
        || f->Bonds[1].InIndex != 4 || f->Bonds[1].OutIndex != 1
        || f->Bonds[2].InIndex != 3 || f->Bonds[2].OutIndex != 2)
      return SZ_ERROR_UNSUPPORTED;
    return SZ_OK;
  }
  
  return SZ_ERROR_UNSUPPORTED;
}

#define CASE_BRA_CONV(isa) case k_ ## isa: isa ## _Convert(outBuffer, outSize, 0, 0); break;

static SRes SzFolder_Decode2(const CSzFolder *folder,
    const Byte *propsData,
    const UInt64 *unpackSizes,
    const UInt64 *packPositions,
    ILookInStream *inStream, UInt64 startPos,
    Byte *outBuffer, SizeT outSize, ISzAllocPtr allocMain,
    Byte *tempBuf[])
{
  UInt32 ci;
  SizeT tempSizes[3] = { 0, 0, 0};
  SizeT tempSize3 = 0;
  Byte *tempBuf3 = 0;

  RINOK(CheckSupportedFolder(folder));

  for (ci = 0; ci < folder->NumCoders; ci++)
  {
    const CSzCoderInfo *coder = &folder->Coders[ci];

    if (IS_MAIN_METHOD((UInt32)coder->MethodID))
    {
      UInt32 si = 0;
      UInt64 offset;
      UInt64 inSize;
      Byte *outBufCur = outBuffer;
      SizeT outSizeCur = outSize;
      if (folder->NumCoders == 4)
      {
        UInt32 indices[] = { 3, 2, 0 };
        UInt64 unpackSize = unpackSizes[ci];
        si = indices[ci];
        if (ci < 2)
        {
          Byte *temp;
          outSizeCur = (SizeT)unpackSize;
          if (outSizeCur != unpackSize)
            return SZ_ERROR_MEM;
          temp = (Byte *)ISzAlloc_Alloc(allocMain, outSizeCur);
          if (!temp && outSizeCur != 0)
            return SZ_ERROR_MEM;
          outBufCur = tempBuf[1 - ci] = temp;
          tempSizes[1 - ci] = outSizeCur;
        }
        else if (ci == 2)
        {
          if (unpackSize > outSize) /* check it */
            return SZ_ERROR_PARAM;
          tempBuf3 = outBufCur = outBuffer + (outSize - (size_t)unpackSize);
          tempSize3 = outSizeCur = (SizeT)unpackSize;
        }
        else
          return SZ_ERROR_UNSUPPORTED;
      }
      offset = packPositions[si];
      inSize = packPositions[(size_t)si + 1] - offset;
      RINOK(LookInStream_SeekTo(inStream, startPos + offset));

      if (coder->MethodID == k_Copy)
      {
        if (inSize != outSizeCur) /* check it */
          return SZ_ERROR_DATA;
        RINOK(SzDecodeCopy(inSize, inStream, outBufCur));
      }
      else if (coder->MethodID == k_LZMA)
      {
        RINOK(SzDecodeLzma(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #ifndef _7Z_NO_METHOD_LZMA2
      else if (coder->MethodID == k_LZMA2)
      {
        RINOK(SzDecodeLzma2(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #endif
      #ifdef _7ZIP_PPMD_SUPPPORT
      else if (coder->MethodID == k_PPMD)
      {
        RINOK(SzDecodePpmd(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream, outBufCur, outSizeCur, allocMain));
      }
      #endif
      else
        return SZ_ERROR_UNSUPPORTED;
    }
    else if (coder->MethodID == k_BCJ2)
    {
      UInt64 offset = packPositions[1];
      UInt64 s3Size = packPositions[2] - offset;
      
      if (ci != 3)
        return SZ_ERROR_UNSUPPORTED;
      
      tempSizes[2] = (SizeT)s3Size;
      if (tempSizes[2] != s3Size)
        return SZ_ERROR_MEM;
      tempBuf[2] = (Byte *)ISzAlloc_Alloc(allocMain, tempSizes[2]);
      if (!tempBuf[2] && tempSizes[2] != 0)
        return SZ_ERROR_MEM;
      
      RINOK(LookInStream_SeekTo(inStream, startPos + offset));
      RINOK(SzDecodeCopy(s3Size, inStream, tempBuf[2]));

      if ((tempSizes[0] & 3) != 0 ||
          (tempSizes[1] & 3) != 0 ||
          tempSize3 + tempSizes[0] + tempSizes[1] != outSize)
        return SZ_ERROR_DATA;

      {
        CBcj2Dec p;
        
        p.bufs[0] = tempBuf3;   p.lims[0] = tempBuf3 + tempSize3;
        p.bufs[1] = tempBuf[0]; p.lims[1] = tempBuf[0] + tempSizes[0];
        p.bufs[2] = tempBuf[1]; p.lims[2] = tempBuf[1] + tempSizes[1];
        p.bufs[3] = tempBuf[2]; p.lims[3] = tempBuf[2] + tempSizes[2];
        
        p.dest = outBuffer;
        p.destLim = outBuffer + outSize;
        
        Bcj2Dec_Init(&p);
        RINOK(Bcj2Dec_Decode(&p));

        {
          unsigned i;
          for (i = 0; i < 4; i++)
            if (p.bufs[i] != p.lims[i])
              return SZ_ERROR_DATA;
          
          if (!Bcj2Dec_IsFinished(&p))
            return SZ_ERROR_DATA;

          if (p.dest != p.destLim
             || p.state != BCJ2_STREAM_MAIN)
            return SZ_ERROR_DATA;
        }
      }
    }
    #ifndef _7Z_NO_METHODS_FILTERS
    else if (ci == 1)
    {
      if (coder->MethodID == k_Delta)
      {
        if (coder->PropsSize != 1)
          return SZ_ERROR_UNSUPPORTED;
        {
          Byte state[DELTA_STATE_SIZE];
          Delta_Init(state);
          Delta_Decode(state, (unsigned)(propsData[coder->PropsOffset]) + 1, outBuffer, outSize);
        }
      }
      else
      {
        if (coder->PropsSize != 0)
          return SZ_ERROR_UNSUPPORTED;
        switch (coder->MethodID)
        {
          case k_BCJ:
          {
            UInt32 state;
            x86_Convert_Init(state);
            x86_Convert(outBuffer, outSize, 0, &state, 0);
            break;
          }
          CASE_BRA_CONV(PPC)
          CASE_BRA_CONV(IA64)
          CASE_BRA_CONV(SPARC)
          CASE_BRA_CONV(ARM)
          CASE_BRA_CONV(ARMT)
          default:
            return SZ_ERROR_UNSUPPORTED;
        }
      }
    }
    #endif
    else
      return SZ_ERROR_UNSUPPORTED;
  }

  return SZ_OK;
}

SRes SzFolder_Decode3(const CSzFolder *folder,
    const Byte *propsData,
    const UInt64 *packPositions,
    ILookInStream *inStream, UInt64 startPos,
    SizeT outSize,
    SzArEx_StreamOutEntry *streamOutInfo,
    UInt32 folderIndex,
    ISzAllocPtr allocMain,
    UnpackProgressCallback callbackProgress,
    UnpackCurFileCallback curFile)
{
    UInt32 ci;

    for (ci = 0; ci < folder->NumCoders; ci++)
    {
        const CSzCoderInfo *coder = &folder->Coders[ci];

        if (IS_MAIN_METHOD((UInt32)coder->MethodID))
        {
            UInt64 offset;
            UInt64 inSize;
            offset = packPositions[0];
            inSize = packPositions[1] - offset;

            RINOK(LookInStream_SeekTo(inStream, startPos + offset));

            if (coder->MethodID == k_Copy)
            {
                if (inSize != outSize) /* check it */
                  return SZ_ERROR_DATA;
                RINOK(SzDecodeCopyStream(inSize, inStream, streamOutInfo, folderIndex, callbackProgress, curFile));
            }
            else if (coder->MethodID == k_LZMA)
            {
                RINOK(SzDecodeLzmaStream(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream,
                                         outSize, streamOutInfo, folderIndex, allocMain, callbackProgress, curFile));
            }
            #ifndef _7Z_NO_METHOD_LZMA2
            else if (coder->MethodID == k_LZMA2)
            {
                RINOK(SzDecodeLzma2Stream(propsData + coder->PropsOffset, coder->PropsSize, inSize, inStream,
                                          outSize, streamOutInfo, folderIndex, allocMain, callbackProgress, curFile));
            }
            #endif
            else
                return SZ_ERROR_UNSUPPORTED;
        }
        else
            return SZ_ERROR_UNSUPPORTED;
    }

    return SZ_OK;
}

SRes SzAr_DecodeFolder(const CSzAr *p, UInt32 folderIndex,
    ILookInStream *inStream, UInt64 startPos,
    Byte *outBuffer, size_t outSize,
    ISzAllocPtr allocMain)
{
  SRes res;
  CSzFolder folder;
  CSzData sd;
  
  const Byte *data = p->CodersData + p->FoCodersOffsets[folderIndex];
  sd.Data = data;
  sd.Size = p->FoCodersOffsets[(size_t)folderIndex + 1] - p->FoCodersOffsets[folderIndex];
  
  res = SzGetNextFolderItem(&folder, &sd);
  
  if (res != SZ_OK)
    return res;

  if (sd.Size != 0
      || folder.UnpackStream != p->FoToMainUnpackSizeIndex[folderIndex]
      || outSize != SzAr_GetFolderUnpackSize(p, folderIndex))
    return SZ_ERROR_FAIL;
  {
    unsigned i;
    Byte *tempBuf[3] = { 0, 0, 0};

    res = SzFolder_Decode2(&folder, data,
        &p->CoderUnpackSizes[p->FoToCoderUnpackSizes[folderIndex]],
        p->PackPositions + p->FoStartPackStreamIndex[folderIndex],
        inStream, startPos,
        outBuffer, (SizeT)outSize, allocMain, tempBuf);
    
    for (i = 0; i < 3; i++)
      ISzAlloc_Free(allocMain, tempBuf[i]);

    if (res == SZ_OK)
      if (SzBitWithVals_Check(&p->FolderCRCs, folderIndex))
        if (CrcCalc(outBuffer, outSize) != p->FolderCRCs.Vals[folderIndex])
          res = SZ_ERROR_CRC;

    return res;
  }
}
