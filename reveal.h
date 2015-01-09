typedef struct _ParmPacket
{
  USHORT portaddress;
  USHORT width;  /* 1=byte,2=word,3=dword */
} ParmPacket;

typedef struct _DataPacket
{
  ULONG data;   /* data read  */
} DataPacket;

