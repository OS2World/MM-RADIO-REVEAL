#define  INCL_DOSFILEMGR
#define  INCL_DOSMISC
#define  INCL_DOSQUEUES
#define  INCL_DOSSESMGR
#define  INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "reveal.h"

USHORT IOAddr = 0x30f;
USHORT IOAddrAlt = 0x20f; /* Try this alt IOAddr if the other one doesn't work. */

int CheckStereo(void)
{
   HFILE  filehandle;
   ULONG action;
   APIRET rc;
   DataPacket dp;
   ParmPacket pp;
   ULONG pio;
   ULONG dio;

   rc=DosOpen("TESTCFG$",&filehandle, &action, 0L, 0, 1,
            OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0L);
   if (rc == 0)
   {

      pio = sizeof(pp);
      pp.portaddress = IOAddr;
      pp.width = 1;
      dio = sizeof(dp);

      rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEINIOINSTR,
                       &pp, pio, &pio, &dp, dio, &dio);

      DosClose(filehandle);
   }

   return(!((dp.data >> 1) &0x01));
}

int WriteCard(USHORT usFrequency, USHORT usMult, USHORT usSwitchOnOff)
{
   USHORT tShift = 0;
   HFILE  filehandle;
   ULONG action;
   APIRET rc;
   DataPacket dp;
   ParmPacket pp;
   ULONG pio;
   ULONG dio;
   ULONG ulBaseTime;
   ULONG ulTime;


   rc=DosOpen("TESTCFG$",&filehandle, &action, 0L, 0, 1,
            OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0L);
   if (rc == 0)
   {

      pio = sizeof(pp);
      pp.portaddress = IOAddr;
      pp.width = 1;
      dio = sizeof(dp);

      while (tShift < 16)
      {
         dp.data = 1;
         if (((usFrequency >> tShift) & 1) == 1)
            dp.data |= 4;
         else
            dp.data &= 0xfb;

         dp.data &= 0xfd;

         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         dp.data |= 2;

         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);

         tShift++;

      }

      tShift = 0;
      while (tShift < 8)
      {
         if (((usMult << tShift) & 0x80) == 0x80)
            dp.data |= 4;
         else
            dp.data &= 0xfb;
         dp.data &= 0xfd;

         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         dp.data |= 0x2;

         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);

         tShift++;
      }


      if (usSwitchOnOff == 0)
         dp.data &= 0xf8;
      else
         dp.data = 0xd8;

      rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                       &pp, pio, &pio, &dp, dio, &dio);

      if (usSwitchOnOff == 0)
         dp.data |= 0xc0;
      else
         dp.data |= 0xc8;

      rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEOUTIOINSTR,
                       &pp, pio, &pio, &dp, dio, &dio);


      rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                     (PVOID)&ulBaseTime,
                     sizeof(ulBaseTime));

      ulTime = ulBaseTime;
      rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEINIOINSTR,
                       &pp, pio, &pio, &dp, dio, &dio);
      /* Give the tuner upto 200ms to lock in Stereo */
      while ((ulTime < ulBaseTime + 200) && ((dp.data>> 1) &0x01))
      {
         rc = DosDevIOCtl(filehandle, IOCTL_TESTCFG_SYS, TESTCFG_SYS_ISSUEINIOINSTR,
                          &pp, pio, &pio, &dp, dio, &dio);
         rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                        (PVOID)&ulTime,
                        sizeof(ulTime));
      }

      DosClose(filehandle);
   }

   return((dp.data >> 1) &0x01);
}

int main(int argc, char *argv[])
{
   double dStation;
   USHORT sStation;
   USHORT x;
   USHORT usStereo;

   printf("\r\n\r\nNote: This is for the Atlanta Georgia, USA Radio station area.\r\n");
   printf("If you do not hear music try: Reveal -alt\r\n\t for alternate IO Address\r\n\r\n\r\n");
   fflush(NULL);

   if (argc > 1)
   {

      if (strnicmp(argv[1],"/alt", 4) == 0)
         IOAddr = IOAddrAlt;
      if (strnicmp(argv[1],"-alt", 4) == 0)
         IOAddr = IOAddrAlt;
   }
   printf("90.1 Now Playing ");
   fflush(NULL);
   dStation = (90.1 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("DAVEFM (Old Z93) Now Playing ");
   fflush(NULL);
   dStation = (92.9 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("96Rock Now Playing ");
   fflush(NULL);
   dStation = (96.1 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("Peach 94.9 Now Playing ");
   fflush(NULL);
   dStation = (94.9 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("B98.5 Now Playing ");
   fflush(NULL);
   dStation = (98.5 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("99x Now Playing ");
   fflush(NULL);
   dStation = (99.7 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);

   printf("104.7 The Fish Now Playing ");
   fflush(NULL);
   dStation = (104.7 - 88.0) * 40;
   sStation = 0xF6C + (USHORT)dStation;
   usStereo = !WriteCard(sStation, 0x05, 1);
   if (usStereo)
      printf("in Stereo\r\n");
   else
      printf("\r\n");
   DosSleep(5000);


/*   printf("Scanning Dial - down\r\n");

   for (x = 0xF6C;x > 0; x--)
   {
      sStation = 0xF6C - x;
      dStation = 88.0 - (sStation / 40.0);
      printf("%3.3f  \r", dStation);
      fflush(NULL);
      WriteCard(sStation, 0x05, 0);
      DosSleep(500);
   }
   printf("\r\n");  */


   printf("Scanning Dial - up\r\n");

   for (x = 0;x < 800; x++)
   {
      dStation = (x / 40.0) + 88.0;
      printf("%3.3f  ", dStation);
      fflush(NULL);
      sStation = 0xF6C + x;
      usStereo = !WriteCard(sStation, 0x05, 1);
      if (usStereo)
      {
         SHORT sWait = 0;
         while (sWait++ < 50)
         {
            printf("\r%3.3f  ", dStation);
            if (CheckStereo())
               printf("Stereo\r");
            else
               printf("        \r");
            fflush(NULL);
            DosSleep(100);
         }
      }
      else
      {
         printf("        \r");
         fflush(NULL);
      }
   }
   printf("\r\n");

   printf("Turning Off\r\n");
   WriteCard(0x0, 0x0, 0);

   return(0);
}

